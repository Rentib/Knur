#ifndef KNUR_HISTORY_H_
#define KNUR_HISTORY_H_

#include "knur.h"
#include "position.h"
#include "search.h"

struct history {
	enum move cmh[12][SQUARE_NB];
	int16_t hh[COLOR_NB][SQUARE_NB][SQUARE_NB];
};

extern struct history history;

void history_clear(void);
void history_update(struct position *position, struct search_stack *search_stack, enum move move, int depth);

#endif /* KNUR_HISTORY_H_ */
