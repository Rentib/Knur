#include "nnue.h"

#if USE_NNUE
#include <stdio.h>
#include <string.h>

#include "../knur.h"

#define INCBIN_PREFIX
#define INCBIN_STYLE INCBIN_STYLE_SNAKE
#include "../incbin/incbin.h"

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
