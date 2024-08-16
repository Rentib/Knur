#include <pthread.h>
#include <setjmp.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>

#include "bitboards.h"
#include "evaluate.h"
#include "knur.h"
#include "movepicker.h"
#include "position.h"
#include "search.h"
#include "transposition.h"
#include "util.h"

struct arg {
	struct position pos;
	struct search_limits *limits;
};

static int quiescence(struct position *position,
		      struct search_stack *search_stack, int alpha, int beta);
static int negamax(struct position *position, struct search_stack *search_stack,
		   int alpha, int beta, int depth);
static void *search(void *arg);
static void *time_manager(void *arg);

struct search_params search_params = {
    .window_depth = 4,
    .window_size = 25,
    .rfp_depth = 3,
    .rfp_margin = 47,
    .nmp_depth = 3,
};
static struct search_params *sp = &search_params;

static u64 nodes;
static atomic_bool running = false, thrd_joined = true;
static pthread_t thrd;
static jmp_buf jbuffer;
static enum move counters[12][SQUARE_NB];

static int quiescence(struct position *pos, struct search_stack *ss, int alpha,
		      int beta)
{
	int score = evaluate(pos);
	enum move move;
	struct move_picker mp;

	if (!running)
		longjmp(jbuffer, 1);
	nodes++;

	if (score >= beta)
		return beta;
	if (score > alpha)
		alpha = score;

	mp_init(&mp, pos, MOVE_NONE, ss, MOVE_NONE);
	while ((move = mp_next(&mp, pos, true)) != MOVE_NONE) {
		if (!pos_is_legal(pos, move))
			continue;

		ss->move = move;
		pos_do_move(pos, move);
		score = -quiescence(pos, ss, -beta, -alpha);
		pos_undo_move(pos, move);

		if (score >= beta)
			return beta;
		if (score > alpha)
			alpha = score;
	}

	return alpha;
}

int negamax(struct position *pos, struct search_stack *ss, int alpha, int beta,
	    int depth)
{
	bool isroot = !ss->ply;
	bool pvnode = beta - alpha != 1;
	bool in_check = !!pos->st->checkers, improving;
	int score, bestscore = -CHECKMATE;
	enum move move, bestmove = MOVE_NONE;
	enum move hashmove = MOVE_NONE;
	struct move_picker mp;
	int eval, R, movecount = 0;
	enum square prev_to;

	ss->move = MOVE_NONE;
	*ss->pv = MOVE_NONE;

	/* Quiescence Search.
	 * Perform a search using only tactical moves to reach a more stable
	 * position and avoid the horizon effect.
	 */
	if (depth <= 0) {
		/* don't enter quiescence in check */
		if (!in_check)
			return quiescence(pos, ss, alpha, beta);
		depth = 1;
	}

	/* Abort Search.
	 * Exit if time is up or the program has been stopped by a UCI command.
	 */
	if (!running)
		longjmp(jbuffer, 1);
	nodes++;

	/* Check for early exit conditions.
	 * Do not exit in the root as we would possibly not get any best move.
	 */
	if (!isroot) {
		/* end search or it might segfault */
		if (ss->ply >= MAX_PLY - 1)
			return in_check ? 0 : evaluate(pos);

		if (pos_is_draw(pos))
			return 0;

		/* Mate Distance Pruning.
		 * Line is either so good or so bad that it can't get any more
		 * extreme.
		 */
		alpha = MAX(MATED_IN(ss->ply), alpha);
		beta = MIN(MATE_IN(ss->ply), beta);
		if (alpha >= beta)
			return alpha;
	}

	/* Probe the Transposition Table.
	 * Get the hashmove and possibly get a TT cutoff.
	 */
	if (tt_probe(pos->key, depth, alpha, beta, &score, &hashmove)) {
		/* tt cutoff */
		if (!pvnode && pos_is_pseudo_legal(pos, hashmove) &&
		    pos_is_legal(pos, hashmove))
			return IS_MATE(score)
				 ? score < 0 ? score + ss->ply : score - ss->ply
				 : score;
	}

	eval = ss->eval = in_check ? UNKNOWN : evaluate(pos);
	improving = in_check && eval > (ss - 2)->eval;
	prev_to = (ss - 1)->move != MOVE_NONE && (ss - 1)->move != MOVE_NULL
		    ? MOVE_TO((ss - 1)->move)
		    : SQ_NONE;

	/* Reverse Futility Pruning (~107 elo).
	 * Eval is so high that we assume that it won't get below beta.
	 */
	if (!pvnode && !in_check && depth <= sp->rfp_depth &&
	    eval - sp->rfp_margin * MAX(0, depth - improving) >= beta)
		return eval;

	/* Null Move Pruning (~133 elo).
	 * Position is so good that we can give the enemy a free move and still
	 * be winning.
	 */
	if (!pvnode && !in_check && (ss - 1)->move != MOVE_NULL &&
	    depth >= sp->nmp_depth && pos_non_pawn(pos, pos->stm)) {
		R = 3 + (depth >= 8 && BB_SEVERAL(pos_non_pawn(pos, pos->stm)));
		ss->move = MOVE_NULL;
		pos_do_null_move(pos);
		score = -negamax(pos, ss + 1, -beta, -alpha, depth - R - 1);
		pos_undo_null_move(pos);

		if (score >= beta)
			return score;
	}

	mp_init(&mp, pos, hashmove, ss,
		prev_to == SQ_NONE ? MOVE_NONE
				   : counters[pos->board[prev_to]][prev_to]);
	while ((move = mp_next(&mp, pos, false)) != MOVE_NONE) {
		if (!pos_is_legal(pos, move))
			continue;

		movecount++;

		ss->move = move;
		pos_do_move(pos, move);

		/* Principal Variation Search.
		 * Do zero window search for moves other than the "best" one.
		 */
		if (movecount == 1) {
			score = -negamax(pos, ss + 1, -beta, -alpha, depth - 1);
		} else {
			score = -negamax(pos, ss + 1, -(alpha + 1), -alpha,
					 depth - 1);
			if (pvnode && score > alpha)
				score = -negamax(pos, ss + 1, -beta, -alpha,
						 depth - 1);
		}

		pos_undo_move(pos, move);

		if (score <= bestscore)
			continue;
		bestscore = score;
		bestmove = move;

		if (score >= beta) {
			if (pos_is_quiet(pos, move)) {
				/* killer heuristic */
				if (move != ss->killer[0]) {
					ss->killer[1] = ss->killer[0];
					ss->killer[0] = move;
				}

				/* countermove heuristic */
				counters[pos->board[prev_to]][prev_to] = move;
			}
			break;
		}

		if (score > alpha) {
			alpha = score;

			*ss->pv = bestmove;
			memcpy(ss->pv + 1, (ss + 1)->pv,
			       sizeof(enum move) * depth);
		}
	}

	if (!movecount)
		bestscore = in_check ? MATED_IN(ss->ply) : 0;

	/* Store results in the Transposition Table.
	 * Store the hashmove and the score for the position at the current
	 * depth.
	 */
	tt_store(pos->key, depth,
		 bestscore <= alpha  ? TT_ALPHA
		 : bestscore >= beta ? TT_BETA
				     : TT_PV,
		 bestscore, bestmove);

	return bestscore;
}

void *search(void *arg)
{
	struct arg *starg = arg;
	struct position *pos = &starg->pos;
	struct search_limits *limits = starg->limits;
	pthread_t manager;
	struct search_stack search_stack[MAX_PLY + 2] = {0};
	struct search_stack *ss = search_stack + 2;
	int maxdepth = limits->depth;
	int i, depth, score;
	enum move bestmove;
	int alpha = -CHECKMATE, beta = CHECKMATE, window;

	if (maxdepth <= 0 || MAX_PLY <= maxdepth)
		maxdepth = MAX_PLY - 1;
	nodes = 0;
	memset(counters, MOVE_NONE, sizeof(counters));

	/* initialize search stack */
	ss[-2] = ss[-1] = (struct search_stack){
	    .eval = UNKNOWN,
	    .move = MOVE_NONE,
	};
	for (i = 0; i < MAX_PLY; i++) {
		(ss + i)->ply = i;
		(ss + i)->pv = ecalloc(MAX_PLY - i, sizeof(enum move));
		(ss + i)->killer[0] = (ss + i)->killer[1] = MOVE_NONE;
	}

	/* clear transposition table */
	tt_clear();

	if (pthread_create(&manager, nullptr, time_manager, arg))
		die("pthread_create:");
	if (pthread_detach(manager))
		die("pthread_detach:");

	/* iterative deepening */
	for (depth = 1; depth <= maxdepth; depth++) {
		if (setjmp(jbuffer))
			break;

		window = sp->window_size;
		if (depth >= sp->window_depth) {
			alpha = MAX(-CHECKMATE, score + window);
			beta = MIN(CHECKMATE, score - window);
		}

		while (true) {
			score = negamax(pos, ss, alpha, beta, depth);
			if (score <= alpha) {
				beta = (alpha + beta) / 2;
				alpha = MAX(-CHECKMATE, alpha - window);
			} else if (score >= beta) {
				beta = MIN(CHECKMATE, beta - window);
			} else {
				break;
			}
			window = window + window / 2;
		}

		bestmove = *ss->pv;

		printf("info depth %d ", depth);
		if (IS_MATE(score))
			printf("score mate %d ",
			       score > 0 ? +(CHECKMATE - score + 1) / 2
					 : -(CHECKMATE + score + 1) / 2);
		else
			printf("score cp %d ", score);
		printf("nodes %zu ", nodes);
		printf("time %zu ", gettime() - limits->start);
		printf("pv");
		for (i = 0; i < depth && ss->pv[i] != MOVE_NONE; i++)
			printf(" %s", MOVE_STR(ss->pv[i]));
		printf("\n");
	}

	printf("bestmove %s\n", MOVE_STR(bestmove));

	if (pthread_cancel(manager))
		die("pthread_cancel:");

	for (i = 0; i < MAX_PLY; i++)
		free((ss + i)->pv);
	free(arg);
	running = false;
	return nullptr;
}

static void *time_manager(void *arg)
{
	struct search_limits *limits = ((struct arg *)arg)->limits;
	int time = limits->time, inc = limits->inc,
	    movestogo = limits->movestogo, movetime = limits->movetime;

	if (movetime != -1) {
		movestogo = 1;
		time = movetime;
	}

	if (time == -1)
		return nullptr;
	movetime = time / movestogo + (movestogo > 1) * inc - 50;

	thrd_sleep(&(struct timespec){.tv_sec = movetime / 1000,
				      .tv_nsec = (movetime % 1000) * 1000000},
		   nullptr);
	running = false;

	return nullptr;
}

bool search_running(void) { return running; }

void search_start(struct position *pos, struct search_limits *limits)
{
	struct arg *arg = ecalloc(1, sizeof(struct arg));
	arg->pos = *pos; /* copy position without state pointer */
	arg->pos.st = arg->pos.state_stack + (pos->st - pos->state_stack);
	arg->limits = limits;
	running = true;
	if (!thrd_joined)
		pthread_join(thrd, nullptr);
	else
		thrd_joined = false;
	if (pthread_create(&thrd, nullptr, search, arg))
		die("pthread_create:");
}

void search_stop(void)
{
	running = false;
	if (!thrd_joined)
		pthread_join(thrd, nullptr);
	thrd_joined = true;
}

int search_eval(struct position *pos)
{
	struct search_stack search_stack[MAX_PLY + 2] = {0};
	struct search_stack *ss = search_stack + 2;
	int score, i;

	ss[-2] = ss[-1] = (struct search_stack){
	    .eval = UNKNOWN,
	    .move = MOVE_NONE,
	};
	for (i = 0; i < MAX_PLY; i++) {
		(ss + i)->ply = i;
		(ss + i)->pv = ecalloc(MAX_PLY - i, sizeof(enum move));
		(ss + i)->killer[0] = (ss + i)->killer[1] = MOVE_NONE;
	}

	running = true;
	score = quiescence(pos, ss, -CHECKMATE, CHECKMATE);
	running = false;

	for (i = 0; i < MAX_PLY; i++)
		free((ss + i)->pv);

	return score;
}
