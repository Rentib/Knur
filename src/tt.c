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
  Key key;
  Move best;
  int depth;
  int age;
};

struct TT {
  size_t size;
  Entry *entries;
};

TT *
tt_create(size_t size)
{
  TT *tt = emalloc(sizeof(TT));
  unsigned i;
  for (tt->size = 1; tt->size < size / sizeof(Entry); tt->size <<= 1);
  tt->entries = emalloc(tt->size * sizeof(Entry));
  for (i = 0; i < tt->size; i++)
    tt->entries[i] = (Entry){ .key = 0ULL,
                              .best = MOVE_NONE,
                              .depth = -1,
                              .age = -1 };
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
