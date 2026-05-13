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
