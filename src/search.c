#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <threads.h>
#include <unistd.h>

#include "position.h"
#include "search.h"
#include "util.h"

static atomic_bool running = false, thrd_joined = true;
static pthread_t thrd;

struct arg {
	struct position *pos;
	struct search_limits limits;
};

static void *search(void *arg)
{
	struct arg *starg = arg;
	struct position *pos = starg->pos;
	struct search_limits *limits = &starg->limits;

	(void)pos;
	(void)limits;

	/* TODO: find best move */

	printf("bestmove e2e4\n");

	free(arg);
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
