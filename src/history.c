#include "knur.h"
#include "history.h"

struct history history;

void history_clear(void)
{
	memset(&history, 0, sizeof(history));
}

void history_update(struct position *pos, struct search_stack *ss, enum move move, int depth)
{
	int delta;
	int16_t *score;
	enum square from = MOVE_FROM(move);
	enum square to = MOVE_TO(move);
	enum move prev = (ss - 1)->move;
	enum square prev_to = SQ_NONE;

	from = MOVE_FROM(move);
	to = MOVE_TO(move);

	/* countermove heuristic */
	if (prev != MOVE_NONE && prev != MOVE_NULL) {
		prev_to = MOVE_TO(prev);
		history.cmh[pos->board[prev_to]][prev_to] = move;
	}

	/* history heuristic */
	/* TODO: good/bad moves */
	score = &(history.hh[pos->stm][from][to]);
	/* NOTE: Formula taken from Ethereal */
	delta = (depth > 13 ? 32 : 16) * depth * depth + 128 * MAX(depth - 1, 0);
	*score += delta - *score * ABS(delta) / (1 << 16);
}
