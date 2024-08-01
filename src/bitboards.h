#ifndef KNUR_BITBOARDS_H_
#define KNUR_BITBOARDS_H_

#include "knur.h"

#define BB_TEST(bitboard, square)  (((bitboard) >> (square)) & 1)
#define BB_SET(bitboard, square)   ((bitboard) |= (1ULL << (square)))
#define BB_RESET(bitboard, square) ((bitboard) &= ~(1ULL << (square)))
#define BB_XOR(bitboard, square)   ((bitboard) ^= (1ULL << (square)))
#define BB_SWAP(bitboard)          ((bitboard) = __builtin_bswap64(bitboard))
#define BB_POPCOUNT(bitboard)      (__builtin_popcountll(bitboard))
#define BB_LSB(bitboard)           (__builtin_ctzll(bitboard))
#define BB_MSB(bitboard)           (__builtin_clzll(bitboard) ^ 63)
#define BB_SINGLE(bitboard)        (!((bitboard) & ((bitboard) - 1)))
#define BB_SEVERAL(bitboard)       (!BB_SINGLE(bitboard))

#define BB_FROM_SQUARE(square) (1ULL << (square))
#define BB_TO_SQUARE(bitboard) (__builtin_ctzll(bitboard))

/* {{{ Common bitboards */
#define BB_RANK_1 (0xFF00000000000000ULL)
#define BB_RANK_2 (BB_RANK_1 >> 8)
#define BB_RANK_3 (BB_RANK_2 >> 8)
#define BB_RANK_4 (BB_RANK_3 >> 8)
#define BB_RANK_5 (BB_RANK_4 >> 8)
#define BB_RANK_6 (BB_RANK_5 >> 8)
#define BB_RANK_7 (BB_RANK_6 >> 8)
#define BB_RANK_8 (BB_RANK_7 >> 8)

#define BB_FILE_A (0x0101010101010101ULL)
#define BB_FILE_B (BB_FILE_A << 1)
#define BB_FILE_C (BB_FILE_B << 1)
#define BB_FILE_D (BB_FILE_C << 1)
#define BB_FILE_E (BB_FILE_D << 1)
#define BB_FILE_F (BB_FILE_E << 1)
#define BB_FILE_G (BB_FILE_F << 1)
#define BB_FILE_H (BB_FILE_G << 1)

#define BB_WHITE_SQUARES (0xAA55AA55AA55AA55ULL)
#define BB_BLACK_SQUARES (~BB_WHITE_SQUARES)
/* }}} */

void bb_init(void);
void bb_free(void);

void bb_print(u64 bitboard);

u64 bb_between(enum square square1, enum square square2);
u64 bb_pawn_attacks(enum color color, enum square square);
u64 bb_attacks(enum piece_type piece, enum square square, u64 occupancy);

INLINE u64 bb_shift(u64 bitboard, enum direction direction)
{
#define gen_shift(bb, dir) ((dir) < 0 ? ((bb) >> -(dir)) : (bb) << (dir))
	switch (direction) {
	case NORTH:       return gen_shift(bitboard, direction);
	case SOUTH:       return gen_shift(bitboard, direction);
	case EAST:        return gen_shift(bitboard & ~BB_FILE_H, direction);
	case WEST:        return gen_shift(bitboard & ~BB_FILE_A, direction);
	case NORTH_EAST:  return gen_shift(bitboard & ~BB_FILE_H, direction);
	case NORTH_WEST:  return gen_shift(bitboard & ~BB_FILE_A, direction);
	case SOUTH_EAST:  return gen_shift(bitboard & ~BB_FILE_H, direction);
	case SOUTH_WEST:  return gen_shift(bitboard & ~BB_FILE_A, direction);
	case NORTH_NORTH: return gen_shift(bitboard, direction);
	case SOUTH_SOUTH: return gen_shift(bitboard, direction);
	default:          __builtin_unreachable();
	}
}

INLINE enum square bb_poplsb(u64 *bitboard)
{
	enum square square = BB_LSB(*bitboard);
	BB_XOR(*bitboard, square);
	return square;
}

INLINE enum square bb_popmsb(u64 *bitboard)
{
	enum square square = BB_MSB(*bitboard);
	BB_XOR(*bitboard, square);
	return square;
}

#endif // KNUR_BITBOARDS_H_
