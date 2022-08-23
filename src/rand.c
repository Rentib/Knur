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

/* ~122ms magic number generation */
static struct {
  uint64_t a;
  uint64_t b;
} state = { 0x7E5B49327CDD086DULL, 0x640BC27E82D1C729ULL };

uint64_t
rand_sparse_u64(void)
{
  return rand_u64() & rand_u64() & rand_u64();
}

/* xorshiro */
uint64_t
rand_u64(void)
{
    uint64_t a = state.a, b = state.b;
    uint64_t res = a + b;
    b ^= a;
    state.a = ((a << 55) | (a >> 9)) ^ b ^ (b << 14);
    state.b = (b << 36) | (b >> 28);
    return res;
}
