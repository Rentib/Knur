#ifndef KNUR_TRANSPOSITION_H_
#define KNUR_TRANSPOSITION_H_

#include <stddef.h>

#include "knur.h"

constexpr size_t TT_DEFAULT_SIZE = 16;
constexpr size_t TT_MIN_SIZE = 1;
constexpr size_t TT_MAX_SIZE = 1024;

constexpr size_t MEBIBYTE = 1ULL << 20;

enum tt_type {
	TT_NONE,
	TT_PV,
	TT_ALPHA,
	TT_BETA,
};

void tt_init(size_t mb);
void tt_free(void);
void tt_clear(void);
bool tt_probe(u64 key, int depth, int alpha, int beta, int *score,
	      enum move *move);
void tt_store(u64 key, int depth, enum tt_type type, int score, enum move move);

void pht_init(size_t mb);
void pht_free(void);
void pht_clear(void);
bool pht_probe(u64 key, u64 wpawns, u64 bpawns, int *score);
void pht_store(u64 key, u64 wpawns, u64 bpawns, int score);

#endif /* KNUR_TRANSPOSITION_H_ */
