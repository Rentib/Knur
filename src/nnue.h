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

#ifndef KNUR_NN_EVALUATE_H_
#define KNUR_NN_EVALUATE_H_

#if USE_NNUE
#include <string.h>

#include "knur.h"

struct nnue_index {
	size_t w;
	size_t b;
};

struct accumulator {
	int16_t values[COLOR_NB][NN_HIDDEN_SIZE] ALIGN;
};

extern int16_t NN_HIDDEN_WEIGHTS[NN_INPUT_SIZE][NN_HIDDEN_SIZE];
extern int16_t NN_HIDDEN_BIAS[NN_HIDDEN_SIZE];
extern int16_t NN_OUTPUT_WEIGHTS[COLOR_NB * NN_HIDDEN_SIZE * NN_OUTPUT_BUCKETS];
extern int16_t NN_OUTPUT_BIAS[NN_OUTPUT_BUCKETS];

INLINE struct nnue_index acc_index(enum piece pc, enum square sq)
{
	constexpr size_t COLOR_STRIDE = 64 * 6;
	constexpr size_t PIECE_STRIDE = 64;

	enum piece_type pt = PIECE_TYPE(pc);
	enum color c = PIECE_COLOR(pc);

	return (struct nnue_index){
	    .w = c * COLOR_STRIDE + pt * PIECE_STRIDE + SQ_FLIP(sq),
	    .b = (1 ^ c) * COLOR_STRIDE + pt * PIECE_STRIDE + sq,
	};
}

INLINE void acc_init(struct accumulator *acc)
{
	memcpy(acc->values[0], NN_HIDDEN_BIAS, sizeof(NN_HIDDEN_BIAS));
	memcpy(acc->values[1], NN_HIDDEN_BIAS, sizeof(NN_HIDDEN_BIAS));
}

INLINE void acc_add(struct accumulator *acc, enum piece pc, enum square sq)
{
	struct nnue_index idx = acc_index(pc, sq);
	for (size_t i = 0; i < NN_HIDDEN_SIZE; i++)
		acc->values[0][i] += NN_HIDDEN_WEIGHTS[idx.w][i];
	for (size_t i = 0; i < NN_HIDDEN_SIZE; i++)
		acc->values[1][i] += NN_HIDDEN_WEIGHTS[idx.b][i];
}

INLINE void acc_sub(struct accumulator *acc, enum piece pc, enum square sq)
{
	struct nnue_index idx = acc_index(pc, sq);
	for (size_t i = 0; i < NN_HIDDEN_SIZE; i++)
		acc->values[0][i] -= NN_HIDDEN_WEIGHTS[idx.w][i];
	for (size_t i = 0; i < NN_HIDDEN_SIZE; i++)
		acc->values[1][i] -= NN_HIDDEN_WEIGHTS[idx.b][i];
}

int nnue_evaluate(enum color stm, const struct accumulator *acc, size_t bucket);
#endif

void nnue_init(void);

#endif /* KNUR_NN_EVALUATE_H_ */
