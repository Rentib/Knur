#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <threads.h>

#include "knur.h"
#include "position.h"
#include "search.h"
#include "util.h"

struct arg {
	struct position *pos;
	struct search_limits limits;
};

static void *search(void *arg);
static void *time_manager(void *arg);

static atomic_bool running = false, thrd_joined = true;
static pthread_t thrd;

void *search(void *arg)
{
	struct arg *starg = arg;
	struct position *pos = starg->pos;
	struct search_limits *limits = &starg->limits;
	pthread_t manager;
	int maxdepth = limits->depth;

	if (maxdepth <= 0 || maxdepth > MAX_PLY)
		maxdepth = MAX_PLY;

	(void)pos;
	(void)limits;

	if (pthread_create(&manager, nullptr, time_manager, arg))
		die("pthread_create:");
	if (pthread_detach(manager))
		die("pthread_detach:");

	/* TODO: find best move */

	printf("bestmove e2e4\n");

	if (pthread_cancel(manager))
		die("pthread_cancel:");

	running = false;
	return nullptr;
}

static void *time_manager(void *arg)
{
	struct search_limits *limits = &((struct arg *)arg)->limits;
	int time = limits->time, inc = limits->inc,
	    movestogo = limits->movestogo, movetime = limits->movetime;

	free(arg);
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
	arg->pos = pos;
	arg->limits = *limits;
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
