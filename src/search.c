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
#include <string.h>

#include "evaluate.h"
#include "knur.h"
#include "movegen.h"
#include "position.h"
#include "search.h"
#include "util.h"

#define INFINITY         69000
#define CHECKMATE        32000
#define STALEMATE        0
#define MAX_PLY          64

/** \typedef PV
 * Defines structure PV.
 */
typedef struct PV PV;

/** \struct PV
 * A structure for storing principle variation.
 * Dynamic memory management seems to be faster in this case.
 * When running search for 2'323'511'178 nodes
 * moves[64] -> 193427ms
 * moves*    -> 168569ms
 */
struct PV {
  /**{*/
  size_t len; /**< Number of pv moves stored; */
  Move *line; /**< Array for storing pv moves. */
  /**}*/
};

static const char *mtstr(Move m);
static int negamax(Position *pos, PV *pv, int alpha, int beta, int depth);
static PV *pv_create(size_t len);
static void pv_free(PV *pv);

SearchInfo info;

static const char *
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

static int
negamax(Position *pos, PV *pv, int alpha, int beta, int depth)
{
  int value;
  Move *m, *last, move_list[256];
  U64 checkers = attackers_to(pos, pos->ksq[pos->turn], ~pos->empty)
               & pos->color[!pos->turn];
  unsigned legalmove = 0;
  PV *new_pv;

  if (!depth) {
    if (!checkers)
      return evaluate(pos);
    depth = 1;
  }

  new_pv = pv_create(MAX_PLY - pos->ply);
  last = generate_moves(GT_ALL, move_list, pos);

  for (m = move_list; m != last; m++) {
    if (!is_legal(pos, *m))
      continue;
    legalmove++;
    do_move(pos, *m);
    value = -negamax(pos, new_pv, -beta, -alpha, depth - 1);
    undo_move(pos, *m);

    if (value >= beta) {
      pv_free(new_pv);
      return value;
    }

    if (value > alpha) {
      pv->line[0] = *m;
      pv->len = new_pv->len + 1;
      memcpy(pv->line + 1, new_pv->line, new_pv->len * sizeof(Move));
      alpha = value;
    }
  }

  pv_free(new_pv);

  if (!legalmove)
    return checkers ? pos->ply - CHECKMATE : STALEMATE;

  return alpha;
}

static PV *
pv_create(size_t len)
{
  PV *pv = emalloc(sizeof(PV));
  pv->len = 0;
  pv->line = emalloc(len * sizeof(Move));
  return pv;
}

static void
pv_free(PV *pv)
{
  if (pv) {
    free(pv->line);
    free(pv);
  }
}

void
search(Position *pos)
{
  int score, alpha = -INFINITY, beta = INFINITY;
  unsigned start = gettime(), i;
  Move bestmove = MOVE_NONE;
  PV *pv = pv_create(MAX_PLY);

  pos->ply = 0;

  score = negamax(pos, pv, alpha, beta, info.depth);

  bestmove = *pv->line;
  printf("info depth %d score cp %d nodes %lu time %d pv",
      info.depth, score, 0UL, gettime() - start);
  for (i = 0; i < pv->len; i++)
    printf(" %s", mtstr(pv->line[i]));
  printf("\n");

  printf("bestmove %s\n", mtstr(bestmove));

  pv_free(pv);
}
