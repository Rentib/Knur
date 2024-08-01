#ifndef KNUR_MOVEGEN_H_
#define KNUR_MOVEGEN_H_

#include "knur.h"
#include "position.h"

enum mg_type {
	MGT_ALL,
	MGT_CAPTURES,
	MGT_QUIET,
};

enum move *mg_generate(enum mg_type mt, enum move *move_list,
		       const struct position *position);

#endif // KNUR_MOVEGEN_H_
