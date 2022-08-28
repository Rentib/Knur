/*
  Knur, a chess engine.
  Copyright (C) 2022 Stanisław Bitner <sbitner420@tutanota.com>

  Knur is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Knur is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "knur.h"
#include "movesort.h"
#include "position.h"

static inline void
set_score(Move *m, int val)
{
  *m = (Move)((val << 16) + *m);
}

Move *
process_moves(Move *begin, Move *end, Move hashmove, const Position *pos)
{
  Move *m;
  for (m = begin; m != end; m++) {
    if (is_legal(pos, *m)) {
      /* move scoring */
      if (*m == hashmove) {
        set_score(m, 16969);
      } else {
        set_score(m, 420);
      }
      *begin++ = *m;
    }
  }
  return begin;
}

void
sort_moves(Move *begin, Move *end)
{
  (void)begin, (void)end;
}
