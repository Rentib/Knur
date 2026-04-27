#ifndef KNUR_NN_EVALUATE_H_
#define KNUR_NN_EVALUATE_H_

#if USE_NNUE
#include <string.h>

#include "../knur.h"

struct nnue_index {
	size_t w;
	size_t b;
};

struct accumulator {
	int16_t values[COLOR_NB][NN_HIDDEN_SIZE] ALIGN;
};

extern int16_t NN_HIDDEN_WEIGHTS[NN_INPUT_SIZE][NN_HIDDEN_SIZE];
extern int16_t NN_HIDDEN_BIAS[NN_HIDDEN_SIZE];
extern int16_t NN_OUTPUT_WEIGHTS[COLOR_NB * NN_HIDDEN_SIZE];
extern int16_t NN_OUTPUT_BIAS;

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

int nnue_evaluate(enum color stm, const struct accumulator *acc);
#endif

void nnue_init(void);
void nnue_free(void);

#endif /* KNUR_NN_EVALUATE_H_ */
