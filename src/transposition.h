#ifndef KNUR_TRANSPOSITION_H_
#define KNUR_TRANSPOSITION_H_

#include <stddef.h>

#include "knur.h"

#define TT_DEFAULT_SIZE (16)
#define TT_MIN_SIZE     (1)
#define TT_MAX_SIZE     (1024)

#define MEBIBYTE (1ULL << 20)

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
