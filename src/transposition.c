#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "knur.h"
#include "transposition.h"
#include "util.h"

struct __attribute__((packed)) tt_entry {
	u64 key;
	int depth : 8;
	enum tt_type type : 2;
	int16_t score;
	enum move move;
};

struct __attribute__((packed)) pht_entry {
	u64 wpawns : 48;
	u64 bpawns : 48;
	int score : 32;
};

struct hash_table {
	size_t size;
	union {
		struct tt_entry *tt_entries;
		struct pht_entry *pht_entries;
	};
	unsigned shift;
};

static struct hash_table tt = {0, {nullptr}, 0};
static struct hash_table pht = {0, {nullptr}, 0};

void tt_init(size_t mb)
{
	if (tt.tt_entries != nullptr)
		free(tt.tt_entries);

	for (tt.size = 1;
	     tt.size * 2 * sizeof(struct tt_entry) <= mb * MEBIBYTE;
	     tt.size *= 2) {}
	tt.tt_entries = ecalloc(tt.size, sizeof(struct tt_entry));
	tt.shift = 64 - __builtin_ctzll(tt.size);
}

void tt_free(void)
{
	free(tt.tt_entries);
	tt.tt_entries = nullptr;
}

void tt_clear(void)
{
	memset(tt.tt_entries, 0, tt.size * sizeof(struct tt_entry));
}

bool tt_probe(u64 key, int depth, int alpha, int beta, int *score,
	      enum move *move)
{
	struct tt_entry *et = &tt.tt_entries[key >> tt.shift];

	*score = UNKNOWN;
	*move = MOVE_NONE;

	if (key == et->key) {
		*move = et->move;
		if ((depth <= et->depth) &&
		    ((et->type == TT_PV) ||
		     (et->type == TT_ALPHA && et->score <= alpha) ||
		     (et->type == TT_BETA && et->score >= beta))) {
			*score = et->score;
			return true;
		}
	}

	return false;
}

void tt_store(u64 key, int depth, enum tt_type type, int score, enum move move)
{
	struct tt_entry *et = &tt.tt_entries[key >> tt.shift];

	if (et->key == key && et->depth > depth)
		return;

	et->key = key;
	et->depth = depth;
	et->type = type;
	et->score = score;
	et->move = move;
}

void pht_init(size_t mb)
{
	if (pht.pht_entries != nullptr)
		free(pht.pht_entries);

	for (pht.size = 1;
	     pht.size * 2 * sizeof(struct pht_entry) <= mb * MEBIBYTE;
	     pht.size *= 2) {}
	pht.tt_entries = ecalloc(pht.size, sizeof(struct pht_entry));
	pht.shift = 64 - __builtin_ctzll(pht.size);
}

void pht_free(void)
{
	free(pht.pht_entries);
	pht.pht_entries = nullptr;
}

void pht_clear(void)
{
	memset(pht.pht_entries, 0, pht.size * sizeof(struct pht_entry));
}

bool pht_probe(u64 key, u64 wpawns, u64 bpawns, int *score)
{
	struct pht_entry *et = &pht.pht_entries[key >> pht.shift];

	if (wpawns == et->wpawns << 8 && bpawns == et->bpawns << 8) {
		*score = et->score;
		return true;
	}

	return false;
}

void pht_store(u64 key, u64 wpawns, u64 bpawns, int score)
{
	struct pht_entry *et = &pht.pht_entries[key >> pht.shift];

	et->wpawns = wpawns >> 8;
	et->bpawns = bpawns >> 8;
	et->score = score;
}
