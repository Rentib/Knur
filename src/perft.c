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

#include <stdint.h>
#include <stdio.h>

#include "knur.h"
#include "movegen.h"
#include "perft.h"
#include "position.h"

static uint64_t perft_help(Position *pos, int depth);

void
perft(Position *pos, int depth /* assuming depth >= 1 */)
{
  Move *m, *last, move_list[256];
  uint64_t nodes_searched = 0, nodes;
  char movestr[6];

  last = generate_moves(GT_ALL, move_list, pos);

  for (m = move_list; m != last; m++) {
    if (!is_legal(pos, *m))
      continue;
    do_move(pos, *m);
    nodes = perft_help(pos, depth - 1);
    undo_move(pos, *m);
    sprintf(movestr, "%c%c%c%c%c", 
        (FROM_SQ(*m) & 7) + 'a', '8' - (FROM_SQ(*m) >> 3),
        (  TO_SQ(*m) & 7) + 'a', '8' - (  TO_SQ(*m) >> 3),
         TYPE_OF(*m) == PROMOTION ? 'p' : '\0');
         
    printf("%s: %lu\n", movestr, nodes);
    nodes_searched += nodes;
  }
  printf("\nNodes searched: %lu\n\n", nodes_searched);
}

static uint64_t
perft_help(Position *pos, int depth)
{
  Move *m, *last, move_list[256];
  uint64_t nodes = 0ULL;

  if (!depth)
    return 1ULL;

  last = generate_moves(GT_ALL, move_list, pos);

  if (depth == 1) {
    for (m = move_list; m != last; m++)
      nodes += (is_legal(pos, *m) ? 1ULL : 0ULL);
  } else {
    for (m = move_list; m != last; m++) {
      if (!is_legal(pos, *m))
        continue;
      do_move(pos, *m);
      nodes += perft_help(pos, depth - 1);
      undo_move(pos, *m);
    }
  }
  return nodes;
}
