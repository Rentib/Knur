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

#ifndef KNUR_TT_H_
#define KNUR_TT_H_

#include <stddef.h>
#include <stdint.h>

#include "knur.h"

#define MEGABYTE        0x100000ULL

/** \typedef EntryType
 * Defines types of entries inside a transposition table.
 * TT_PV    - pv-node;
 * TT_ALPHA - fail low node;
 * TT_BETA  - fail high node;
 * TT_NONE  - nothing.
 */
typedef enum { TT_PV, TT_ALPHA, TT_BETA, TT_NONE } EntryType;

/** \typedef Key
 * Defines Key as a 64 bit unsigned integer.
 */
typedef uint64_t Key;

/** \typedef TT
 * Defines structure TT.
 */
typedef struct TT TT;

/** \struct TT
 * Transposition table.
 */
struct TT;

TT *tt_create(size_t size);
void tt_free(TT *tt);

void tt_clear(TT *tt);
int tt_probe(TT *tt, Key key, int *score, Move *move, int depth, int alpha, int beta, int ply);
void tt_store(TT *tt, Key key, int score, Move move, int depth, EntryType type);
void tt_update(TT *tt);

#endif /* KNUR_TT_H_ */
