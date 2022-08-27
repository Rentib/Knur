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

#include <stdio.h>

#include "evaluate.h"
#include "knur.h"
#include "movegen.h"
#include "position.h"
#include "search.h"

#define INFINITY         69000
#define CHECKMATE        32000
#define STALEMATE        0

static int negamax(Position *pos, int alpha, int beta, int depth);

SearchInfo info;

static int
negamax(Position *pos, int alpha, int beta, int depth)
{
  int value;
  Move *m, *last, move_list[256];
  U64 checkers = attackers_to(pos, pos->ksq[pos->turn], ~pos->empty)
               & pos->color[!pos->turn];
  int legalmove = 0;

  if (!depth) {
    if (!checkers)
      return evaluate(pos);
    depth = 1;
  }

  last = generate_moves(GT_ALL, move_list, pos);

  for (m = move_list; m != last; m++) {
    if (!is_legal(pos, *m))
      continue;
    legalmove++;
    do_move(pos, *m);
    value = -negamax(pos, -beta, -alpha, depth - 1);
    undo_move(pos, *m);

    if (value >= beta)
      return value;

    if (value > alpha)
      alpha = value;
  }

  if (!legalmove)
    return checkers ? pos->ply - CHECKMATE : STALEMATE;

  return alpha;
}

static inline const char *
mtstr(Move m)
{
  const char ptc[6] = { 'p', 'n', 'b', 'r', 'q', 'k' };
  static char res[6];
  sprintf(res, "%c%c%c%c%c", 
      'a' + (FROM_SQ(m) & 7), '8' - (FROM_SQ(m) >> 3),
      'a' + (  TO_SQ(m) & 7), '8' - (  TO_SQ(m) >> 3),
      TYPE_OF(m) == PROMOTION ? ptc[PROMOTION_TYPE(m)] : '\0');
  return res;
}

void
search(Position *pos)
{
  int bestvalue = -CHECKMATE, value;
  int alpha = -INFINITY, beta = INFINITY;
  Move bestmove = MOVE_NONE;
  Move *m, *last, move_list[256];

  pos->ply = 0;
  last = generate_moves(GT_ALL, move_list, pos);

  for (m = move_list; m != last; m++) {
    if (!is_legal(pos, *m))
      continue;
    do_move(pos, *m);
    value = -negamax(pos, -beta, -alpha, info.depth - 1);
    undo_move(pos, *m);

    if (value > alpha)
      alpha = value;

    if (value > bestvalue) {
      bestvalue = value;
      bestmove = *m;
    }
  }

  printf("bestmove %s\n", mtstr(bestmove));
}
