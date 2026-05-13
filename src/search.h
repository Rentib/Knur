/*
  Knur, a UCI chess engine.
  Copyright (C) 2024-2026 Stanisław Bitner

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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
