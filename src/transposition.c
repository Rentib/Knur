#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "knur.h"
#include "transposition.h"
#include "util.h"

constexpr size_t TT_BUCKETS = 3;

struct __attribute__((packed)) tt_entry {
	u64 key : 16;
	int depth : 8;
	int age : 6;
	enum tt_bound bound : 2;
	int16_t value;
	int16_t eval;
	enum move move;
};

struct tt_bucket {
	struct tt_entry entries[TT_BUCKETS];
	uint16_t padding;
};

static struct {
	struct tt_bucket *buckets;
	size_t mask;
	int age;
} tt = {nullptr, 0, 0};

void tt_init(size_t mb)
{
	size_t keysize = 16;

	if (tt.mask)
		free(tt.buckets);

	while ((1ull << keysize) * sizeof(struct tt_bucket) <=
	       mb * MEBIBYTE / 2)
		keysize++;

	tt.buckets = ecalloc(1, (1ull << keysize) * sizeof(struct tt_bucket));
	tt.mask = (1ull << keysize) - 1u;

	tt_clear();
}

void tt_free(void)
{
	if (tt.mask)
		free(tt.buckets);
	tt.mask = 0;
}

void tt_clear(void)
{
	memset(tt.buckets, 0, (tt.mask + 1) * sizeof(struct tt_bucket));
	tt.age = 0;
}

bool tt_probe(u64 key, int *depth, enum tt_bound *bound, int *value, int *eval, enum move *move)
{
	size_t i;
	struct tt_entry *et = tt.buckets[key & tt.mask].entries;
	key >>= 48;

	for (i = 0; i < TT_BUCKETS; i++, et++) {
		if (et->key != key)
			continue;

		et->age = tt.age;

		*depth = et->depth;
		*bound = et->bound;
		*value = et->value;
		*eval = et->eval;
		*move = et->move;

		return true;
	}

	return false;
}

void tt_store(u64 key, int depth, enum tt_bound bound, int value, int eval, enum move move)
{
	size_t i;
	struct tt_entry *et = tt.buckets[key & tt.mask].entries, *old = et;
	key >>= 48;

	for (i = 0; i < TT_BUCKETS && et->key != key; i++, et++) {
		if (old->depth - (tt.age - old->age) >=
		     et->depth - (tt.age -  et->age))
			old = et;
	}

	old = (i < TT_BUCKETS) ? et : old;

	if (bound != TT_EXACT && key == old->key && depth < old->depth - 2)
		return;

	if (move != MOVE_NONE || key != old->key)
		old->move = move;

	old->age = tt.age;

	old->depth = depth;
	old->bound = bound;
	old->value = value;
	old->eval = eval;
	old->key = key;
}

void tt_update(void)
{
	tt.age += 1;
}

void tt_prefetch(u64 key)
{
	__builtin_prefetch(&tt.buckets[key & tt.mask]);
}

size_t tt_hashfull(void)
{
	size_t used = 0, i, j;

	for (i = 0; i < 1000; i++)
		for (j = 0; j < TT_BUCKETS; j++)
			used += tt.buckets[i].entries[j].bound != TT_NONE &&
				tt.buckets[i].entries[j].age == tt.age;

	return used / TT_BUCKETS;
}

/* Pawn Hash Table {{{ */
struct __attribute__((packed)) pht_entry {
	u64 wpawns : 48;
	u64 bpawns : 48;
	int value : 32;
};


static struct {
	size_t size;
	struct pht_entry *entries;
	unsigned shift;
} pht = {0, nullptr, 0};

void pht_init(size_t mb)
{
	if (pht.entries != nullptr)
		free(pht.entries);

	for (pht.size = 1;
	     pht.size * 2 * sizeof(struct pht_entry) <= mb * MEBIBYTE;
	     pht.size *= 2) {}
	pht.entries = ecalloc(pht.size, sizeof(struct pht_entry));
	pht.shift = 64 - __builtin_ctzll(pht.size);
}

void pht_free(void)
{
	free(pht.entries);
	pht.entries = nullptr;
}

void pht_clear(void)
{
	memset(pht.entries, 0, pht.size * sizeof(struct pht_entry));
}

bool pht_probe(u64 key, u64 wpawns, u64 bpawns, int *value)
{
	struct pht_entry *et = &pht.entries[key >> pht.shift];

	if (wpawns == et->wpawns << 8 && bpawns == et->bpawns << 8) {
		*value = et->value;
		return true;
	}

	return false;
}

void pht_store(u64 key, u64 wpawns, u64 bpawns, int value)
{
	struct pht_entry *et = &pht.entries[key >> pht.shift];

	et->wpawns = wpawns >> 8;
	et->bpawns = bpawns >> 8;
	et->value = value;
}
/* }}} */
