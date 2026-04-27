#include "nnue.h"

#if USE_NNUE
#include <stdio.h>
#include <string.h>

#include "../knur.h"

#define INCBIN_PREFIX
#define INCBIN_STYLE INCBIN_STYLE_SNAKE
#include "../incbin/incbin.h"

INCBIN(embed, EVALFILE);

int16_t NN_HIDDEN_WEIGHTS[NN_INPUT_SIZE][NN_HIDDEN_SIZE] ALIGN; /* N x 768 */
int16_t NN_HIDDEN_BIAS[NN_HIDDEN_SIZE] ALIGN;                   /* N x   1 */
int16_t NN_OUTPUT_WEIGHTS[COLOR_NB * NN_HIDDEN_SIZE] ALIGN;     /* 1 x 2*N */
int16_t NN_OUTPUT_BIAS ALIGN;                                   /* 1 x   1 */

constexpr int SCALE = 400;
constexpr int QA = 255;
constexpr int QB = 64;

void nnue_init(void)
{
	const int16_t *data = (int16_t *)embed_data;

	for (unsigned i = 0; i < NN_INPUT_SIZE; i++)
		for (unsigned j = 0; j < NN_HIDDEN_SIZE; j++)
			NN_HIDDEN_WEIGHTS[i][j] = *data++;

	for (unsigned i = 0; i < NN_HIDDEN_SIZE; i++)
		NN_HIDDEN_BIAS[i] = *data++;

	for (unsigned i = 0; i < COLOR_NB * NN_HIDDEN_SIZE; i++)
		NN_OUTPUT_WEIGHTS[i] = *data++;

	NN_OUTPUT_BIAS = *data++;
}

void nnue_free(void) { /* TODO: implement */ }

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

int nnue_evaluate(enum color stm, const struct accumulator *acc)
{
	int value = 0;

	value += flatten(acc->values[stm], NN_OUTPUT_WEIGHTS);
	value += flatten(acc->values[!stm], NN_OUTPUT_WEIGHTS + NN_HIDDEN_SIZE);

	return (value / QA + NN_OUTPUT_BIAS) * SCALE / (QA * QB);
}

#else

void nnue_init(void) {}

#endif
