#include <pthread.h>
#include <setjmp.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>

#include "evaluate.h"
#include "knur.h"
#include "movegen.h"
#include "position.h"
#include "search.h"
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

static int negamax(struct position *position, struct search_stack *search_stack,
		   int alpha, int beta, int depth);
static void *search(void *arg);
static void *time_manager(void *arg);

static atomic_bool running = false, thrd_joined = true;
static pthread_t thrd;
static jmp_buf jbuffer;

int negamax(struct position *pos, struct search_stack *ss, int alpha, int beta,
	    int depth)
{
	int isroot = !ss->ply;
	int score;
	int bestscore = -CHECKMATE;
	enum move bestmove;
	enum move move_list[256], *last, *m;

	if (!running)
		longjmp(jbuffer, 1);

	if (!isroot) {
		if (!depth) {
			/* TODO: quiescence */
			if (!pos->st->checkers)
				return evaluate(pos);
			return 0;
		}

		if (ss->ply >= MAX_PLY - 1)
			return pos->st->checkers ? 0 : evaluate(pos);
	}

	last = mg_generate(MGT_ALL, move_list, pos);
	for (m = move_list; m != last; m++) {
		if (!pos_is_legal(pos, *m))
			continue;

		ss->move = *m;
		pos_do_move(pos, *m);
		score = -negamax(pos, ss + 1, -beta, -alpha, depth - 1);
		pos_undo_move(pos, *m);

		if (score <= bestscore)
			continue;
		bestscore = score;
		bestmove = *m;

		if (score >= beta)
			break;

		if (score > alpha) {
			alpha = score;

			*ss->pv = bestmove;
			memcpy(ss->pv + 1, (ss + 1)->pv,
			       sizeof(enum move) * depth);
		}
	}

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
		maxdepth = MAX_PLY;

	/* initialize search stack */
	for (i = 0; i < MAX_PLY; i++) {
		(ss + i)->ply = i;
		(ss + i)->pv = ecalloc(MAX_PLY - i, sizeof(enum move));
	}

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
		printf("score cp %d ", score);
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
