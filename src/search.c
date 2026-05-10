#include <math.h>
#include <pthread.h>
#include <setjmp.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>

#include "bitboards.h"
#include "evaluate.h"
#include "history.h"
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

static int quiescence(struct position *position, struct search_stack *search_stack, int alpha, int beta);
static int negamax(struct position *position, struct search_stack *search_stack, int alpha, int beta, int depth, bool cutnode);
static void *search(void *arg);

struct search_params search_params = {
    .window_depth = 4,
    .window_size = 25,
    .rfp_depth = 3,
    .rfp_margin = 47,
    .nmp_depth = 3,
    .lmr_base = 0.7844,
    .lmr_scale = 2.4696,
};
static struct search_params *sp = &search_params;

static u64 nodes, max_nodes;
static atomic_bool running = false, thrd_joined = true;
static u64 stop_time;
static pthread_t thrd;
static jmp_buf jbuffer;
static int lmr_reduction[MAX_PLY][64];

INLINE bool abort_search(void)
{
	if ((stop_time && nodes % 4096 == 0 && gettime() >= stop_time) ||
	    (nodes >= max_nodes))
		running = false;
	return !running;
}

static int quiescence(struct position *pos, struct search_stack *ss, int alpha,
		      int beta)
{
	int value = evaluate(pos);
	enum move move;
	struct move_picker mp;

	if (abort_search())
		longjmp(jbuffer, 1);
	nodes++;

	if (value >= beta)
		return beta;
	if (value > alpha)
		alpha = value;

	mp_init(&mp, pos, MOVE_NONE, ss);
	while ((move = mp_next(&mp, pos, true)) != MOVE_NONE) {
		if (!pos_is_legal(pos, move))
			continue;

		ss->move = move;
		pos_do_move(pos, move);
		tt_prefetch(pos->key);
		value = -quiescence(pos, ss, -beta, -alpha);
		pos_undo_move(pos, move);

		if (value >= beta)
			return beta;
		if (value > alpha)
			alpha = value;
	}

	return alpha;
}

int negamax(struct position *pos, struct search_stack *ss, int alpha, int beta, int depth, bool cutnode)
{
	bool isroot = !ss->ply;
	bool pvnode = beta - alpha != 1;
	bool in_check = !!pos->st->checkers;
	bool tt_hit, improving, is_quiet, full_search;
	int value = -CHECKMATE;
	int best_value = -CHECKMATE;
	int eval, R, movecount = 0;
	int orig_alpha = alpha, prob_beta;
	int tt_depth, tt_value = UNKNOWN, tt_eval = UNKNOWN;
	enum tt_bound tt_bound = TT_NONE;
	enum move move, bestmove = MOVE_NONE;
	enum move hashmove = MOVE_NONE;
	struct move_picker mp;

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
	if (abort_search())
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
	tt_hit = tt_probe(pos->key, &tt_depth, &tt_bound, &tt_value, &tt_eval, &hashmove);
	if (tt_hit && !pvnode && depth <= tt_depth && tt_value != UNKNOWN &&
	    hashmove != MOVE_NONE &&
	    ((tt_bound == TT_EXACT) ||
	     (tt_bound == TT_UPPER && tt_value <= alpha) ||
	     (tt_bound == TT_LOWER && tt_value >= beta)) &&
	    pos_is_pseudo_legal(pos, hashmove) && pos_is_legal(pos, hashmove)) {
		return IS_MATE(tt_value) ? tt_value < 0 ? tt_value + ss->ply
							: tt_value - ss->ply
					 : tt_value;
	}

	/* Internal Iterative deepening (~13 elo).
	 * If we haven't found a hashmove for the position, it usually is a good
	 * idea to search the current position with shallower depth and get its
	 * best move. At the same time we can get a better eval of the position.
	 */
	if (!tt_hit && depth >= 6 && (pvnode || cutnode)) {
		(void)negamax(pos, ss, alpha, beta, depth * 2 / 3, cutnode);
		tt_hit = tt_probe(pos->key, &tt_depth, &tt_bound, &tt_value, &tt_eval, &hashmove);
	}

	/* Static Evaluation
	 * As evaluate() function is expensive, try to get it from tt first.
	 */
	eval = ss->eval = in_check                     ? UNKNOWN
			: tt_hit && tt_eval != UNKNOWN ? tt_eval
						       : evaluate(pos);

	if (!tt_hit && eval != UNKNOWN)
		tt_store(pos->key, 0, TT_NONE, UNKNOWN, eval, MOVE_NONE);

	improving = in_check && eval > (ss - 2)->eval;

	if (pvnode || in_check)
		goto move_loop;

	/* Reverse Futility Pruning (~107 elo).
	 * Eval is so high that we assume that it won't get below beta.
	 */
	if (depth <= sp->rfp_depth &&
	    eval - sp->rfp_margin * MAX(0, depth - improving) >= beta)
		return eval;

	/* Null Move Pruning (~133 elo).
	 * Position is so good that we can give the enemy a free move and still
	 * be winning.
	 */
	if ((ss - 1)->move != MOVE_NULL && depth >= sp->nmp_depth &&
	    pos_non_pawn(pos, pos->stm)) {
		R = 3 + (depth >= 8 && BB_SEVERAL(pos_non_pawn(pos, pos->stm)));
		ss->move = MOVE_NULL;
		pos_do_null_move(pos);
		tt_prefetch(pos->key);
		value = -negamax(pos, ss + 1, -beta, -alpha, depth - R - 1, !cutnode);
		pos_undo_null_move(pos);

		if (value >= beta)
			return value;
	}

	/* ProbCut (~9 elo).
	 * After calculating a and b parameters as well as the standard
	 * deviation of error e, we can get a high probability of the actual
	 * value being greater than beta.
	 * For confidence we use Phi^(-1)(99%) = 2.3263.
	 *
	 * v >= beta
	 * (v' * a + c - beta) / sigma >= -e/sigma
	 * v' >= (Phi^(-1)(p) * sigma + beta - c) / a
	 *
	 * Data gathered from 10^6 positions from selfplay games.
	 * ==================================================
	 * REGRESSION: v = v'*a + c + e
	 * a:           1.096462
	 * c:          -4.981127
	 * Sigma:       127.8935
	 * R2:          0.964498
	 * ==================================================
	 */
	prob_beta = (2.3263 * 127.8935 + beta - -4.981127) / 1.096462;
	if (depth >= 6 && !IS_MATE(beta) &&
	    !(tt_hit && tt_depth >= depth - 3 && tt_value < prob_beta)) {
		mp_init(&mp, pos, hashmove, ss);
		while ((move = mp_next(&mp, pos, true)) && mp.stage <= MP_STAGE_GOOD_CAPTURES) {
			if (!pos_is_legal(pos, move))
				continue;

			pos_do_move(pos, move);
			tt_prefetch(pos->key);

			/* TODO: it might be beneficial to validate with
			 * quiescence only in case of deep searches */
			value = -quiescence(pos, ss + 1, -prob_beta, -prob_beta + 1);
			if (value >= prob_beta)
				value = -negamax(pos, ss + 1, -prob_beta, -prob_beta + 1, depth - 4, !cutnode);

			pos_undo_move(pos, move);

			if (value >= prob_beta) {
				if (!tt_hit || tt_depth < depth - 3)
					tt_store(pos->key, depth - 3, TT_LOWER, value, eval, move);
				return value;
			}
		}
	}

move_loop:
	mp_init(&mp, pos, hashmove, ss);
	while ((move = mp_next(&mp, pos, false)) != MOVE_NONE) {
		if (!pos_is_legal(pos, move))
			continue;

		movecount++;

		is_quiet = pos_is_quiet(pos, move);

		/* Late Move Pruning (~11 elo).
		 * If we have already found a move which raises alpha and
		 * checked quite a few moves, then — assuming that the move
		 * ordering is alright — we can prune other moves.
		 */
		if (!pvnode && mp.stage >= MP_STAGE_GENERATE_QUIET &&
		    !in_check && pos_non_pawn(pos, pos->stm) &&
		    orig_alpha < alpha &&
		    movecount >= (3 + depth * depth) / (2 - improving))
			break;

		ss->move = move;
		pos_do_move(pos, move);
		tt_prefetch(pos->key);

		/* Late Move Reductions.
		 * Reduce the depth of search for moves other than the first
		 * one. This assumes the move ordering is so good that the first
		 * move is the best one.
		 */
		if (depth >= 2 && movecount > 1) {
			/* Quiet Late Move Reductions (~32 elo). */
			if (is_quiet) {
				R = lmr_reduction[MIN(depth, MAX_PLY)]
						 [MIN(movecount, 64)];

				R += !pvnode + !improving;
				R += in_check &&
				     PIECE_TYPE(pos->board[MOVE_FROM(move)]) ==
					 KING;
				R -= mp.stage < MP_STAGE_QUIET;

				/* TODO: adjust based on history scores */
			}

			R = MAX(1, MIN(depth - 1, R));

			value = -negamax(pos, ss + 1, -(alpha + 1), -alpha, depth - R, true);

			/* TODO: adjust research depth based on results */

			full_search = value > alpha && R > 1;
		} else {
			full_search = !pvnode || movecount > 1;
		}

		if (full_search)
			value = -negamax(pos, ss + 1, -(alpha + 1), -alpha, depth - 1, !cutnode);

		if (pvnode && (movecount == 1 || value > alpha))
			value = -negamax(pos, ss + 1, -beta, -alpha, depth - 1, false);

		pos_undo_move(pos, move);

		if (value <= best_value)
			continue;
		best_value = value;
		bestmove = move;

		if (value >= beta) {
			if (is_quiet) {
				/* killer heuristic */
				if (move != ss->killer[0]) {
					ss->killer[1] = ss->killer[0];
					ss->killer[0] = move;
				}

				history_update(pos, ss, move, depth);
			}
			break;
		}

		if (value > alpha) {
			alpha = value;

			*ss->pv = bestmove;
			memcpy(ss->pv + 1, (ss + 1)->pv,
			       sizeof(enum move) * depth);
		}
	}

	if (!movecount)
		best_value = in_check ? MATED_IN(ss->ply) : 0;

	/* Store results in the Transposition Table.
	 * Store the hashmove and the value of the position at the current
	 * depth.
	 */
	tt_store(pos->key, depth,
		 best_value <= orig_alpha ? TT_UPPER
		 : best_value >= beta     ? TT_LOWER
					  : TT_EXACT,
		 best_value, eval, bestmove);

	return best_value;
}

void *search(void *arg)
{
	struct arg *starg = arg;
	struct position *pos = &starg->pos;
	struct search_limits *limits = starg->limits;
	struct search_stack search_stack[MAX_PLY + 2] = {0};
	struct search_stack *ss = search_stack + 2;
	int i, depth, value;
	enum move bestmove;
	int alpha = -CHECKMATE, beta = CHECKMATE, window;

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

	/* clear move history */
	history_clear();

	/* clear transposition table */
	tt_update();

	stop_time = 0;
	if (limits->movetime != -1) {
		limits->movestogo = 1;
		limits->time = limits->movetime;
	}
	if (limits->time != -1) {
		limits->movetime = limits->time / limits->movestogo;
		limits->movetime += (limits->movestogo > 1) * limits->inc;
		limits->movetime -= 50;

		stop_time = limits->start + limits->movetime;
	}

	nodes = 0;
	max_nodes = limits->nodes ? limits->nodes : -1;

	/* iterative deepening */
	for (depth = 1; depth <= limits->depth; depth++) {
		if (setjmp(jbuffer))
			break;

		window = sp->window_size;
		if (depth >= sp->window_depth) {
			alpha = MAX(value + window, -CHECKMATE);
			beta  = MIN(value - window, +CHECKMATE);
		}

		while (true) {
			value = negamax(pos, ss, alpha, beta, depth, false);
			if (value <= alpha) {
				beta = (alpha + beta) / 2;
				alpha = MAX(alpha - window, -CHECKMATE);
			} else if (value >= beta) {
				beta = MIN(beta + window, +CHECKMATE);
			} else {
				break;
			}
			window = window + window / 2;
		}

		bestmove = *ss->pv;

		printf("info depth %d ", depth);
		if (IS_MATE(value))
			printf("score mate %d ",
			       value > 0 ? +(CHECKMATE - value + 1) / 2
					 : -(CHECKMATE + value + 1) / 2);
		else
			printf("score cp %d ", value);
		printf("time %zu ", gettime() - limits->start);
		printf("nodes %zu ", nodes);
		printf("hashfull %zu ", tt_hashfull());
		printf("pv");
		for (i = 0; i < depth && ss->pv[i] != MOVE_NONE; i++)
			printf(" %s", MOVE_STR(ss->pv[i]));
		printf("\n");
	}

	printf("bestmove %s\n", MOVE_STR(bestmove));

	for (i = 0; i < MAX_PLY; i++)
		free((ss + i)->pv);
	free(arg);
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
	if (!thrd_joined && pthread_join(thrd, nullptr))
		die("pthread_join:");
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

void search_init(void)
{
	for (int depth = 0; depth < MAX_PLY; depth++) {
		for (int movecount = 0; movecount < 64; movecount++)
			lmr_reduction[depth][movecount] =
			    sp->lmr_base +
			    log(depth) * log(movecount) / sp->lmr_scale;
	}
}

int search_eval(struct position *pos)
{
	struct search_stack search_stack[MAX_PLY + 2] = {0};
	struct search_stack *ss = search_stack + 2;
	int value, i;

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
	value = quiescence(pos, ss, -CHECKMATE, CHECKMATE);
	running = false;

	for (i = 0; i < MAX_PLY; i++)
		free((ss + i)->pv);

	return value;
}
