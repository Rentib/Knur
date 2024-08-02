#ifndef KNUR_SEARCH_H_
#define KNUR_SEARCH_H_

#include <pthread.h>
#include <stdatomic.h>

#include "position.h"

struct search_limits {
	int time;
	int inc;
	int movestogo;
	int depth;
	int movetime;
	bool infinite;
};

bool search_running(void);
void search_start(struct position *position, struct search_limits *limits);
void search_stop(void);

#endif /* KNUR_SEARCH_H_ */
