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
