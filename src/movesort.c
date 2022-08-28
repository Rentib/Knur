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

static const int mvv[] = {
  [  PAWN] = 9200,
  [KNIGHT] = 9300,
  [BISHOP] = 9400,
  [  ROOK] = 9500,
  [ QUEEN] = 10000,
  [  KING] = -1,  /* it should be impossible to capture a king */
  [  NONE] = 100, /* move is not a capture */
};

/** \brief Sets score of a move.
 * \param[in,out] m - pointer to a move;
 * \param[in] val   - value that we want to assign to a move.
 */
static inline void
set_score(Move *m, int val)
{
  *m = (Move)((val << 16) + *m);
}

Move *
process_moves(Move *begin, Move *end, Move hashmove, const Position *pos)
{
  Square from, to;
  Move *m;
  for (m = begin; m != end; m++) {
    if (is_legal(pos, *m)) {
      if (*m == hashmove) {
        set_score(m, 16969);
      } else {
        switch (TYPE_OF(*m)) {
        case NORMAL:
          from = FROM_SQ(*m), to = TO_SQ(*m);
          if (pos->board[to] != NONE) {
            /* capture */
            set_score(m, mvv[pos->board[to]] - pos->board[from]);
          } else {
            /* quiet move */
            set_score(m, 0);
          }
          break;
        case PROMOTION:
          set_score(m, 10000);
          break;
        case CASTLE:
          set_score(m, 5000);
          break;
        case EN_PASSANT:
          set_score(m, 7000);
          break;
        }
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
