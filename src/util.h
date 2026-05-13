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

#ifndef KNUR_UTIL_H_
#define KNUR_UTIL_H_

#include <stddef.h>
#include <stdint.h>

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define ARRAY_FILL(arr, val)                                                   \
	do {                                                                   \
		for (unsigned __i = 0; __i < ARRAY_SIZE(arr); __i++)           \
			arr[__i] = val;                                        \
	} while (0)

[[noreturn]] void die(const char *fmt, ...);
void *ecalloc(size_t nmemb, size_t size);
uint64_t rand_sparse_u64(void);
uint64_t rand_u64(void);
uint64_t gettime(void);

#endif /* KNUR_UTIL_H_ */
