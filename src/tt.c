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

#include <stddef.h>

#include "knur.h"
#include "tt.h"
#include "util.h"

/** \typedef Entry
 * Defines structure Entry
 */
typedef struct Entry Entry;

/** \struct Entry
 * A structure for storing TT entries.
 */
struct Entry {
  /**{*/
  Key key;        /**< Zobrist hash of a position; */
  int score;      /**< Score of a position; */
  Move move;      /**< Best move of a position; */
  int depth;      /**< Depth up to which the position has been checked; */
  EntryType type; /** Type of an entry; */
  int age;        /** Age of an entry. */
  /**}*/
};

struct TT {
  /**{*/
  size_t size;    /**< Number of entries inside the transposition table; */
  Entry *entries; /**< Array of entries. */
  /**}*/
};

TT *
tt_create(size_t size)
{
  TT *tt = emalloc(sizeof(TT));
  for (tt->size = 1; tt->size < size / sizeof(Entry); tt->size <<= 1);
  tt->entries = emalloc(tt->size * sizeof(Entry));
  tt_clear(tt);
  return tt;
}

void
tt_free(TT *tt)
{
  if (tt) {
    free(tt->entries);
    free(tt);
  }
}

void
tt_clear(TT *tt)
{
  unsigned i;
  for (i = 0; i < tt->size; i++)
    tt->entries[i] = (Entry){ .key = 0ULL,
                              .score = 0,
                              .move = MOVE_NONE,
                              .depth = 0,
                              .type = TT_NONE,
                              .age = -1 };
}

int
tt_probe(TT *tt, Key key, int *score, Move *move,
         int depth, int alpha, int beta, int ply)
{
  Entry *et = &tt->entries[key & (tt->size - 1)];
  if (key == et->key) {
    *move  = et->move;
    *score = et->score >  ISCHECKMATE ? et->score - ply
           : et->score < -ISCHECKMATE ? et->score + ply
           : et->score;
    if (depth <= et->depth && et->type != TT_NONE) {
      if (et->type == TT_PV
      || (et->type == TT_ALPHA && et->score <= alpha)
      || (et->type == TT_BETA  && et->score >= beta))
        return 1;
    }
  }
  *move = MOVE_NONE;
  *score = 0;
  return 0;
}

void
tt_store(TT *tt, Key key, int score, Move move, int depth, EntryType type)
{
  /* Always replace approach;
   * Will be improved in the future. */
  Entry *et = &tt->entries[key & (tt->size - 1)];
  et->key = key;
  et->score = score;
  et->move = move;
  et->depth = depth;
  et->type = type;
}
