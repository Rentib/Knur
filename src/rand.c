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

#include "rand.h"

static uint64_t state = 0xfab15ae89128af81ULL;

uint64_t
rand_sparse_u64(void)
{
  return rand_u64() & rand_u64() & rand_u64();
}

uint32_t
rand_u32(void)
{
  uint32_t x = state;
  x ^= x >> 13;
  x ^= x << 17;
  x ^= x >> 5;
  return state = x;
}

uint64_t
rand_u64(void)
{
  return (uint64_t)(rand_u32() & 0xFFFF) << 0
       | (uint64_t)(rand_u32() & 0xFFFF) << 16
       | (uint64_t)(rand_u32() & 0xFFFF) << 32
       | (uint64_t)(rand_u32() & 0xFFFF) << 48;
}
