#ifndef KNUR_SEARCH_H_
#define KNUR_SEARCH_H_

#include <pthread.h>
#include <stdatomic.h>

#include "knur.h"
#include "position.h"

struct search_limits {
	int time;
	int inc;
	int movestogo;
	int depth;
	int movetime;
	bool infinite;

	u64 start;
};

struct search_stack {
	int ply;             /* halfmove counter */
	enum move move;      /* current move */
	enum move *pv;       /* principal variation */
	enum move killer[2]; /* killer moves */
};

bool search_running(void);
void search_start(struct position *position, struct search_limits *limits);
void search_stop(void);

#endif /* KNUR_SEARCH_H_ */
