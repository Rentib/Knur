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

struct hash_table {
	size_t size;
	struct tt_entry *entries;
	unsigned shift;
};

static struct hash_table tt = {0, nullptr, 0};

void tt_init(size_t mb)
{
	if (tt.entries == nullptr)
		free(tt.entries);

	for (tt.size = 1;
	     tt.size * 2 * sizeof(struct tt_entry) <= mb * MEBIBYTE;
	     tt.size *= 2) {}
	tt.entries = ecalloc(tt.size, sizeof(struct tt_entry));
	tt.shift = 64 - __builtin_ctzll(tt.size);
}

void tt_free(void)
{
	free(tt.entries);
	tt.entries = nullptr;
}

void tt_clear(void)
{
	memset(tt.entries, 0, tt.size * sizeof(struct tt_entry));
}

bool tt_probe(u64 key, int depth, int alpha, int beta, int *score,
	      enum move *move)
{
	struct tt_entry *et = &tt.entries[key >> tt.shift];

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
	struct tt_entry *et = &tt.entries[key >> tt.shift];

	if (et->key == key && et->depth > depth)
		return;

	et->key = key;
	et->depth = depth;
	et->type = type;
	et->score = score;
	et->move = move;
}
