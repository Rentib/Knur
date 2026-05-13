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

#include "nnue.h"

#if USE_NNUE
#include <stdio.h>
#include <string.h>

#include "knur.h"

#define INCBIN_PREFIX
#define INCBIN_STYLE INCBIN_STYLE_SNAKE
#include "incbin/incbin.h"

INCBIN(embed, EVALFILE);

int16_t NN_HIDDEN_WEIGHTS[NN_INPUT_SIZE][NN_HIDDEN_SIZE];
int16_t NN_HIDDEN_BIAS[NN_HIDDEN_SIZE];
int16_t NN_OUTPUT_WEIGHTS[COLOR_NB * NN_HIDDEN_SIZE * NN_OUTPUT_BUCKETS];
int16_t NN_OUTPUT_BIAS[NN_OUTPUT_BUCKETS];

constexpr int SCALE = 400;
constexpr int QA = 255;
constexpr int QB = 64;

void nnue_init(void)
{
	const int16_t *data = (int16_t *)embed_data;
	size_t i, j;

	for (i = 0; i < NN_INPUT_SIZE; i++)
		for (j = 0; j < NN_HIDDEN_SIZE; j++)
			NN_HIDDEN_WEIGHTS[i][j] = *data++;

	for (i = 0; i < NN_HIDDEN_SIZE; i++)
		NN_HIDDEN_BIAS[i] = *data++;

#if 0
	/* NOTE: Alexandria nets have transposed format */
	for (i = 0; i < 2 * NN_HIDDEN_SIZE; i++)
		for (j = 0; j < NN_OUTPUT_BUCKETS; j++)
			NN_OUTPUT_WEIGHTS[j * COLOR_NB * NN_HIDDEN_SIZE + i] = *data++;
#else
	for (i = 0; i < COLOR_NB * NN_HIDDEN_SIZE * NN_OUTPUT_BUCKETS; i++)
		NN_OUTPUT_WEIGHTS[i] = *data++;
#endif

	for (i = 0; i < NN_OUTPUT_BUCKETS; i++)
		NN_OUTPUT_BIAS[i] = *data++;
}

INLINE int activate(int16_t x)
{
	int clamp = MIN(MAX(x, 0), QA);
	return clamp * clamp;
}

static int flatten(const int16_t *a, const int16_t *w)
{
	int r = 0;
	for (size_t i = 0; i < NN_HIDDEN_SIZE; i++)
		r += activate(a[i]) * w[i];
	return r;
}

int nnue_evaluate(enum color stm, const struct accumulator *acc, size_t bucket)
{
	int value = 0;
	size_t bucket_offset = COLOR_NB * NN_HIDDEN_SIZE * bucket;
	int16_t *weights = NN_OUTPUT_WEIGHTS + bucket_offset;

	value += flatten(acc->values[stm], weights);
	value += flatten(acc->values[!stm], weights + NN_HIDDEN_SIZE);

	return (value / QA + NN_OUTPUT_BIAS[bucket]) * SCALE / (QA * QB);
}

#else

void nnue_init(void) {}

#endif
