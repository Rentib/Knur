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
	u64 nodes;
	int movetime;
	bool infinite;

	u64 start;
};

struct search_stack {
	int ply;             /* halfmove counter */
	int eval;            /* static evaluation */
	enum move move;      /* current move */
	enum move *pv;       /* principal variation */
	enum move killer[2]; /* killer moves */
	enum move skip;      /* singular move */
	int dextensions;     /* number of double extensions */
};

struct search_params {
	int window_depth;
	int window_size;
	int rfp_depth;
	int rfp_margin;
	int nmp_depth;
	float lmr_base;
	float lmr_scale;
};

bool search_running(void);
void search_start(struct position *position, struct search_limits *limits);
void search_stop(void);
void search_init(void);
int search_eval(struct position *position);

extern struct search_params search_params;

#endif /* KNUR_SEARCH_H_ */
