#include <pthread.h>
#include <setjmp.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>

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

struct search_stack {
	int ply;        /* halfmove counter */
	enum move move; /* current move */
	enum move *pv;  /* principal variation */
};

static int quiescence(struct position *position,
		      struct search_stack *search_stack, int alpha, int beta);
static int negamax(struct position *position, struct search_stack *search_stack,
		   int alpha, int beta, int depth);
static void *search(void *arg);
static void *time_manager(void *arg);

static u64 nodes;
static atomic_bool running = false, thrd_joined = true;
static pthread_t thrd;
static jmp_buf jbuffer;

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

	mp_init(&mp, pos, MOVE_NONE);
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
	int score, bestscore = -CHECKMATE;
	enum move move, bestmove = MOVE_NONE;
	enum move hashmove = MOVE_NONE;
	struct move_picker mp;
	int movecount = 0;

	ss->move = MOVE_NONE;
	*ss->pv = MOVE_NONE;

	if (!isroot) {
		if (!depth) {
			/* don't enter quiescence in check */
			if (!pos->st->checkers)
				return quiescence(pos, ss, alpha, beta);
			depth = 1;
		}

		/* end search or it might segfault */
		if (ss->ply >= MAX_PLY - 1)
			return pos->st->checkers ? 0 : evaluate(pos);

		if (pos_is_draw(pos))
			return 0;
	}

	if (!running)
		longjmp(jbuffer, 1);
	nodes++;

	if (tt_probe(pos->key, depth, alpha, beta, &score, &hashmove)) {
		/* tt cutoff */
		if (!pvnode && pos_is_legal(pos, hashmove))
			return IS_MATE(score)
				 ? score < 0 ? score + ss->ply : score - ss->ply
				 : score;
	}

	mp_init(&mp, pos, hashmove);
	while ((move = mp_next(&mp, pos, false)) != MOVE_NONE) {
		if (!pos_is_legal(pos, move))
			continue;

		movecount++;

		ss->move = move;
		pos_do_move(pos, move);

		/* principal variation search */
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

		if (score >= beta)
			break;

		if (score > alpha) {
			alpha = score;

			*ss->pv = bestmove;
			memcpy(ss->pv + 1, (ss + 1)->pv,
			       sizeof(enum move) * depth);
		}
	}

	if (!movecount)
		bestscore = pos->st->checkers ? MATED_IN(ss->ply) : 0;

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

	if (maxdepth <= 0 || MAX_PLY <= maxdepth)
		maxdepth = MAX_PLY - 1;
	nodes = 0;

	/* initialize search stack */
	for (i = 0; i < MAX_PLY; i++) {
		(ss + i)->ply = i;
		(ss + i)->pv = ecalloc(MAX_PLY - i, sizeof(enum move));
	}

	/* clear transposition table */
	tt_clear();

	if (pthread_create(&manager, nullptr, time_manager, arg))
		die("pthread_create:");
	if (pthread_detach(manager))
		die("pthread_detach:");

	for (depth = 1; depth <= maxdepth; depth++) {
		if (setjmp(jbuffer))
			break;

		score = negamax(pos, ss, -CHECKMATE, CHECKMATE, depth);
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
	movetime = time / movestogo + inc - 50;

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
