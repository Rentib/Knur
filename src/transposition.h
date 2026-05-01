#ifndef KNUR_TRANSPOSITION_H_
#define KNUR_TRANSPOSITION_H_

#include <stddef.h>

#include "knur.h"

constexpr size_t TT_DEFAULT_SIZE = 16;
constexpr size_t TT_MIN_SIZE = 1;
constexpr size_t TT_MAX_SIZE = 1024;

constexpr size_t MEBIBYTE = 1ULL << 20;

enum tt_bound {
	TT_NONE,
	TT_UPPER,
	TT_LOWER,
	TT_EXACT,
};

void tt_init(size_t mb);
void tt_free(void);
void tt_clear(void);
bool tt_probe(u64 key, int *depth, enum tt_bound *bound, int *value, enum move *move);
void tt_store(u64 key, int depth, enum tt_bound bound, int value, enum move move);

void pht_init(size_t mb);
void pht_free(void);
void pht_clear(void);
bool pht_probe(u64 key, u64 wpawns, u64 bpawns, int *value);
void pht_store(u64 key, u64 wpawns, u64 bpawns, int value);

#endif /* KNUR_TRANSPOSITION_H_ */
