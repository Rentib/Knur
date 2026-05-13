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

#ifndef KNUR_TRANSPOSITION_H_
#define KNUR_TRANSPOSITION_H_

#include <stddef.h>

#include "knur.h"

constexpr size_t TT_DEFAULT_SIZE = 16;
constexpr size_t TT_MIN_SIZE = 1;
constexpr size_t TT_MAX_SIZE = 1024;

constexpr size_t MEBIBYTE = 1ULL << 20;

enum tt_bound {
	TT_NONE,
	TT_UPPER,
	TT_LOWER,
	TT_EXACT,
};

void tt_init(size_t mb);
void tt_free(void);
void tt_clear(void);
bool tt_probe(u64 key, int *depth, enum tt_bound *bound, int *value, int *eval, enum move *move);
void tt_store(u64 key, int depth, enum tt_bound bound, int value, int eval, enum move move);
void tt_update(void);
void tt_prefetch(u64 key);
size_t tt_hashfull(void);

void pht_init(size_t mb);
void pht_free(void);
void pht_clear(void);
bool pht_probe(u64 key, u64 wpawns, u64 bpawns, int *value);
void pht_store(u64 key, u64 wpawns, u64 bpawns, int value);

#endif /* KNUR_TRANSPOSITION_H_ */
