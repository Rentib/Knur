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

#ifndef KNUR_MOVEPICKER_H_
#define KNUR_MOVEPICKER_H_

#include "knur.h"
#include "position.h"
#include "search.h"

enum mp_stage {
	MP_STAGE_HASH,
	MP_STAGE_GENERATE_CAPTURES,
	MP_STAGE_GOOD_CAPTURES,
	MP_STAGE_KILLER1,
	MP_STAGE_KILLER2,
	MP_STAGE_COUNTER,
	MP_STAGE_GENERATE_QUIET,
	MP_STAGE_QUIET,
	MP_STAGE_BAD_CAPTURES,
	MP_STAGE_DONE,
};

struct move_picker {
	enum mp_stage stage;
	int scores[256]; /* move scores */
	enum move moves[256];
	enum move *captures, *quiets;
	enum move hashmove;
	enum move killer[2];
	enum move counter;
};

void mp_init(struct move_picker *mp, struct position *position, enum move hashmove, struct search_stack *search_stack);
enum move mp_next(struct move_picker *mp, struct position *position, bool skip_quiet);

#endif /* KNUR_MOVEPICKER_H_ */
