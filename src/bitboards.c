#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef USE_PEXT
#include <immintrin.h>
#endif

#include "bitboards.h"
#include "knur.h"
#include "util.h"

#define MAGIC(magic, occupancy) (magic.attacks[magic_hash(&magic, occupancy)])

struct magic {
	u64 relevant; /* relevant occupancy bits */
#ifndef USE_PEXT
	u64 magic; /* magic number */
#endif
	u64 *attacks;   /* attacks for different occupancy masks */
	unsigned shift; /* right shift */
};

static void find_magic(enum piece_type piece, enum square square);
static void generate_king_attacks(enum square square);
static void generate_knight_attacks(enum square square);
static void generate_pawn_attacks(enum square square);
static void generate_relevant_bishop_occupancy(enum square square);
static void generate_relevant_rook_occupancy(enum square square);

static u64 get_bishop_attacks(enum square square, u64 occupancy);
static u64 get_rook_attacks(enum square square, u64 occupancy);
static u64 get_sliding_attacks(enum square square, u64 occupancy,
			       enum direction direction);
static u64 get_state(u64 bitboard, unsigned state);

INLINE unsigned magic_hash(struct magic *m, u64 occ)
{
#ifdef USE_PEXT
	return _pext_u64(occ, m->relevant);
#else
	occ &= m->relevant;
	occ *= m->magic;
	return occ >> (64 - m->shift);
#endif
}

static u64 between[SQUARE_NB][SQUARE_NB];     /* [square][square] */
static u64 pawn_attacks[COLOR_NB][SQUARE_NB]; /* [color][square] */
static u64 king_attacks[SQUARE_NB];           /* [square] */
static u64 knight_attacks[SQUARE_NB];         /* [square] */

static struct magic bishop_magics[SQUARE_NB]; /* [square] */
static struct magic rook_magics[SQUARE_NB];   /* [square] */

void bb_init(void)
{
	enum square sq, sq2;
	for (sq = 0; sq < SQUARE_NB; sq++) {
		generate_king_attacks(sq);
		generate_knight_attacks(sq);
		generate_pawn_attacks(sq);
		generate_relevant_bishop_occupancy(sq);
		generate_relevant_rook_occupancy(sq);
		find_magic(BISHOP, sq);
		find_magic(ROOK, sq);
	}

	for (sq = 0; sq < SQUARE_NB; sq++) {
		for (sq2 = 0; sq2 < SQUARE_NB; sq2++) {
			if (BB_TEST(bb_attacks(BISHOP, sq, 0ULL), sq2)) {
				between[sq][sq2] =
				    bb_attacks(BISHOP, sq,
					       BB_FROM_SQUARE(sq2)) &
				    bb_attacks(BISHOP, sq2, BB_FROM_SQUARE(sq));
			} else if (BB_TEST(bb_attacks(ROOK, sq, 0ULL), sq2)) {
				between[sq][sq2] =
				    bb_attacks(ROOK, sq, BB_FROM_SQUARE(sq2)) &
				    bb_attacks(ROOK, sq2, BB_FROM_SQUARE(sq));
			}
			BB_SET(between[sq][sq2], sq);
			BB_SET(between[sq][sq2], sq2);
		}
	}
}

void bb_free(void)
{
	enum square sq;
	for (sq = 0; sq < SQUARE_NB; sq++) {
		free(bishop_magics[sq].attacks);
		free(rook_magics[sq].attacks);
	}
}

void bb_print(u64 bb)
{
	int rank, file, c;
	enum square sq = SQ_A8;
	const char *sep = "+---+---+---+---+---+---+---+---+";
	printf("  %s\n", sep);
	for (rank = 8; rank >= 1; rank--) {
		printf("%d |", rank);
		for (file = 0; file < 8; file++) {
			c = BB_TEST(bb, sq++) ? 'X' : ' ';
			printf(" \033[31;1m%c\033[00;0m |", c);
		}
		printf("\n  %s\n", sep);
	}
	printf("    a   b   c   d   e   f   g   h\n");
	printf("  Bitnoard: %016" PRIx64 "\n", bb);
}

u64 bb_between(enum square square1, enum square square2)
{
	return between[square1][square2];
}

u64 bb_pawn_attacks(enum color c, enum square sq)
{
	return pawn_attacks[c][sq];
}

u64 bb_attacks(enum piece_type pt, enum square sq, u64 occ)
{
	(void)occ;
	switch (pt) {
	case KNIGHT: return knight_attacks[sq];
	case BISHOP: return MAGIC(bishop_magics[sq], occ);
	case ROOK:   return MAGIC(rook_magics[sq], occ);
	case QUEEN:
		return MAGIC(bishop_magics[sq], occ) |
		       MAGIC(rook_magics[sq], occ);
	case KING: return king_attacks[sq];
	default:   return 0;
	}
}

void find_magic(enum piece_type pt, enum square sq)
{
	struct magic *m = (pt == ROOK ? rook_magics : bishop_magics) + sq;
	unsigned i, size = 1 << m->shift;
	u64 occupancy[size], attacks[size];
	unsigned checked[size];
	unsigned rndcnt, hash;

	m->attacks = ecalloc(size, sizeof(u64));

	for (i = 0; i < size; checked[i++] = 0) {
		occupancy[i] = get_state(m->relevant, i);
		attacks[i] = pt == ROOK ? get_rook_attacks(sq, occupancy[i])
					: get_bishop_attacks(sq, occupancy[i]);
	}

	for (i = 0, rndcnt = 1; i < size && rndcnt < 10000000; rndcnt++) {
#ifndef USE_PEXT
		m->magic = rand_sparse_u64();
		if (BB_POPCOUNT((m->relevant * m->magic) &
				0xFF00000000000000ULL) < 6)
			continue;
#endif
		for (i = 0; i < size; i++) {
			hash = magic_hash(m, occupancy[i]);
			if (checked[hash] < rndcnt) {
				checked[hash] = rndcnt;
				m->attacks[hash] = attacks[i];
			} else if (m->attacks[hash] != attacks[i]) {
				break;
			}
		}
	}

	if (i < size)
		die("failed to generate magics");
}

void generate_king_attacks(enum square sq)
{
	u64 bb = BB_FROM_SQUARE(sq);
	bb |= bb_shift(bb, NORTH) | bb_shift(bb, SOUTH);
	bb |= bb_shift(bb, EAST) | bb_shift(bb, WEST);
	king_attacks[sq] = BB_RESET(bb, sq);
}

void generate_knight_attacks(enum square sq)
{
	u64 bb = BB_FROM_SQUARE(sq), b1, b2;
	b1 = bb_shift(bb, NORTH) | bb_shift(bb, SOUTH);
	b2 = bb_shift(bb, NORTH_NORTH) | bb_shift(bb, SOUTH_SOUTH);
	knight_attacks[sq] = bb_shift(bb_shift(b1, EAST), EAST) |
			     bb_shift(bb_shift(b1, WEST), WEST) |
			     bb_shift(b2, EAST) | bb_shift(b2, WEST);
}

void generate_pawn_attacks(enum square sq)
{
	u64 bb = BB_FROM_SQUARE(sq);
	bb = bb_shift(bb, EAST) | bb_shift(bb, WEST);
	pawn_attacks[WHITE][sq] = bb_shift(bb, NORTH);
	pawn_attacks[BLACK][sq] = bb_shift(bb, SOUTH);
}

void generate_relevant_bishop_occupancy(enum square sq)
{
	u64 bb = get_bishop_attacks(sq, 0);
	bb &= ~(BB_RANK_1 | BB_RANK_8 | BB_FILE_A | BB_FILE_H);
	bishop_magics[sq].relevant = bb;
	bishop_magics[sq].shift = BB_POPCOUNT(bb);
}

void generate_relevant_rook_occupancy(enum square sq)
{
	u64 bb = (get_sliding_attacks(sq, 0, NORTH) & ~BB_RANK_8) |
		 (get_sliding_attacks(sq, 0, SOUTH) & ~BB_RANK_1) |
		 (get_sliding_attacks(sq, 0, EAST) & ~BB_FILE_H) |
		 (get_sliding_attacks(sq, 0, WEST) & ~BB_FILE_A);
	rook_magics[sq].relevant = bb;
	rook_magics[sq].shift = BB_POPCOUNT(bb);
}

u64 get_bishop_attacks(enum square sq, u64 occ)
{
	return get_sliding_attacks(sq, occ, NORTH_EAST) |
	       get_sliding_attacks(sq, occ, NORTH_WEST) |
	       get_sliding_attacks(sq, occ, SOUTH_EAST) |
	       get_sliding_attacks(sq, occ, SOUTH_WEST);
}

u64 get_rook_attacks(enum square sq, u64 occ)
{
	return get_sliding_attacks(sq, occ, NORTH) |
	       get_sliding_attacks(sq, occ, SOUTH) |
	       get_sliding_attacks(sq, occ, EAST) |
	       get_sliding_attacks(sq, occ, WEST);
}

u64 get_sliding_attacks(enum square sq, u64 occ, enum direction dir)
{
	u64 res = 0, bb = BB_FROM_SQUARE(sq);
	while (bb) {
		bb = bb_shift(bb, dir);
		res |= bb;
		bb &= ~occ;
	}
	return res;
}

u64 get_state(u64 bitboard, unsigned state)
{
	u64 res = 0, lsb;
	while (bitboard) {
		lsb = bitboard & -bitboard;
		if (state & 1)
			res |= lsb;
		bitboard ^= lsb;
		state >>= 1;
	}
	return res;
}
