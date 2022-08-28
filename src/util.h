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

#ifndef KNUR_UTIL_H_
#define KNUR_UTIL_H_

#include <stdint.h>
#include <stdlib.h>

#define MAX(a, b)        ((a) > (b) ? (a) : (b))
#define MIN(a, b)        ((a) < (b) ? (a) : (b))
#define LENGTH(X)        (sizeof X / sizeof X[0])

void die(const char *fmt, ...);
void *ecalloc(size_t nitems, size_t size);
void *emalloc(size_t size);
int gettime(void);
uint64_t rand_sparse_u64(void);
uint64_t rand_u64(void);
void readline(char *input);


#endif /* KNUR_UTIL_H_ */
