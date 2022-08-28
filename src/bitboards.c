/*
  Knur, a chess engine.
  Copyright (C) 2022 Stanisław Bitner <sbitner420@tutanota.com>

  Knur is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Knur is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdint.h>
#include <stdio.h>

#include "bitboards.h"
#include "knur.h"
#include "util.h"

#define MAGIC_ATTACK(magic, occupancy)    \
  (magic.attacks[magic_hash(&magic, (occupancy))])

/** \typedef Magic
 * Defines structure Magic.
 */
typedef struct Magic Magic;

/** \struct Magic
 * A structure for storing fancy magics.
 */
struct Magic {
  /**{*/
  U64      relevant; /**< Mask of relevant bits; */
  U64      magic;    /**< Magic number; */
  U64     *attacks;  /**< Dynamic array of attacks; */
  unsigned shift;    /**< Number of relevant bits. */
  /**}*/
};

static U64 get_bishop_attacks(Square sq, U64 occ);
static U64 get_rook_attacks(Square sq, U64 occ);
static void look_for_magic(PieceType pt, Square sq);
static inline unsigned magic_hash(Magic *m, U64 occ);
static void mask_king_attacks(Square sq);
static void mask_knight_attacks(Square sq);
static void mask_pawn_attacks(Square sq);
static void mask_relevant_bishop_occupancy(Square sq);
static void mask_relevant_rook_occupancy(Square sq);
static U64 mask_state(U64 mask, unsigned idx);
static U64 slide(Direction dir, Square sq, U64 occ);

const U64 Rank8BB = 0xFFULL;
const U64 Rank7BB = Rank8BB << 8;
const U64 Rank6BB = Rank7BB << 8;
const U64 Rank5BB = Rank6BB << 8;
const U64 Rank4BB = Rank5BB << 8;
const U64 Rank3BB = Rank4BB << 8;
const U64 Rank2BB = Rank3BB << 8;
const U64 Rank1BB = Rank2BB << 8;

const U64 FileABB = 0x0101010101010101ULL;
const U64 FileBBB = FileABB << 1;
const U64 FileCBB = FileBBB << 1;
const U64 FileDBB = FileCBB << 1;
const U64 FileEBB = FileDBB << 1;
const U64 FileFBB = FileEBB << 1;
const U64 FileGBB = FileFBB << 1;
const U64 FileHBB = FileGBB << 1;

/* lookup tables */
static U64 between[64][64];
static U64 pawn_attacks[2][64]; /* [Color][Square] */
static U64 knight_attacks[64];  /* [Square] */
static U64 king_attacks[64];    /* [Square] */
static Magic BishopMagics[64];  /* [Square] */
static Magic RookMagics[64];    /* [Square] */

U64
attacks_bb(PieceType pt, Square sq, U64 occ)
{
  switch (pt) {
  case KNIGHT: return knight_attacks[sq];
  case BISHOP: return MAGIC_ATTACK(BishopMagics[sq], occ);
  case   ROOK: return MAGIC_ATTACK(RookMagics[sq], occ);
  case  QUEEN: return MAGIC_ATTACK(BishopMagics[sq], occ)
                    | MAGIC_ATTACK(RookMagics[sq], occ);
  case   KING: return king_attacks[sq];
  default: return 0ULL;
  }
}

U64
between_bb(Square sq1, Square sq2)
{
  return between[sq1][sq2];
}

void
bitboards_free(void)
{
  Square sq;
  for (sq = SQ_A8; sq <= SQ_H1; sq++) {
    free(RookMagics[sq].attacks);
    free(BishopMagics[sq].attacks);
  }
}

static U64
get_bishop_attacks(Square sq, U64 occ)
{
  return slide(NORTH_EAST, sq, occ)
       | slide(SOUTH_EAST, sq, occ)
       | slide(SOUTH_WEST, sq, occ)
       | slide(NORTH_WEST, sq, occ);
}

static U64
get_rook_attacks(Square sq, U64 occ)
{
  return slide(NORTH, sq, occ)
       | slide(EAST,  sq, occ)
       | slide(SOUTH, sq, occ)
       | slide(WEST,  sq, occ);
}

void
bitboards_init(void)
{
  Square sq, sq1, sq2;
  for (sq = SQ_A8; sq <= SQ_H1; sq++) {
    mask_king_attacks(sq);
    mask_knight_attacks(sq);
    mask_pawn_attacks(sq);
    mask_relevant_bishop_occupancy(sq);
    mask_relevant_rook_occupancy(sq);
    look_for_magic(BISHOP, sq);
    look_for_magic(  ROOK, sq);
  }

  for (sq1 = SQ_A8; sq1 <= SQ_H1; sq1++) {
    for (sq2 = SQ_A8; sq2 <= SQ_H1; sq2++) {
      if (GET_BIT(attacks_bb(BISHOP, sq1, 0ULL), sq2))
        between[sq1][sq2] = attacks_bb(BISHOP, sq1, GET_BITBOARD(sq2))
                          & attacks_bb(BISHOP, sq2, GET_BITBOARD(sq1));
      if (GET_BIT(attacks_bb(ROOK, sq1, 0ULL), sq2))
        between[sq1][sq2] = attacks_bb(ROOK, sq1, GET_BITBOARD(sq2))
                          & attacks_bb(ROOK, sq2, GET_BITBOARD(sq1));
      between[sq1][sq2] |= GET_BITBOARD(sq1) | GET_BITBOARD(sq2);
    }
  }
}

static void
look_for_magic(PieceType pt, Square sq)
{
  Magic *m = (pt == ROOK ? &RookMagics[sq] : &BishopMagics[sq]);
  unsigned i, size = 1 << m->shift, rndcnt, hash;
  unsigned checked[size];
  U64 occupancy[size];
  U64 attacks[size];

  m->attacks = ecalloc(size, sizeof(U64));

  for (i = 0; i < size; checked[i++] = 0) {
    occupancy[i] = mask_state(m->relevant, i);
    attacks[i] = (pt == ROOK ?   get_rook_attacks(sq, occupancy[i])
                             : get_bishop_attacks(sq, occupancy[i]));
  }

  for (i = 0, rndcnt = 1; i < size; rndcnt++) {
    for (m->magic = 0; POPCOUNT((m->magic * m->relevant) >> 56) < 6; )
      m->magic = rand_sparse_u64();
    for (i = 0; i < size; i++) {
      hash = magic_hash(m, occupancy[i]);
      if (checked[hash] < rndcnt) {
        checked[hash] = rndcnt;
        m->attacks[hash] = attacks[i];
      } else if (m->attacks[hash] != attacks[i])
        break;
    }
  }
}

static inline unsigned
magic_hash(Magic *m, U64 occ)
{
  occ &= m->relevant;
  occ *= m->magic;
  return occ >> (64 - m->shift);
}

static void
mask_pawn_attacks(Square sq)
{
  U64 mask = GET_BITBOARD(sq);
  U64 bb = shift(WEST, mask) | shift(EAST, mask);
  pawn_attacks[WHITE][sq] = shift(NORTH, bb);
  pawn_attacks[BLACK][sq] = shift(SOUTH, bb);
}

static void
mask_knight_attacks(Square sq)
{
  U64 mask = GET_BITBOARD(sq);
  U64 bb1, bb2;
  bb1 = shift(EAST, mask) | shift(WEST, mask);
  bb2 = shift(EAST, shift(EAST, mask)) | shift(WEST, shift(WEST, mask));
  knight_attacks[sq] = shift(NORTH_NORTH, bb1) | shift(NORTH, bb2)
                     | shift(SOUTH_SOUTH, bb1) | shift(SOUTH, bb2);
}

static void
mask_king_attacks(Square sq)
{
  U64 mask = GET_BITBOARD(sq);
  U64 bb = shift(WEST, mask) | mask | shift(EAST, mask);
  king_attacks[sq] = (shift(NORTH, bb) | bb | shift(SOUTH, bb)) & ~mask;
}

static void
mask_relevant_bishop_occupancy(Square sq)
{
  U64 mask = (slide(NORTH_EAST, sq, 0ULL) & ~(Rank8BB | FileHBB))
           | (slide(SOUTH_EAST, sq, 0ULL) & ~(Rank1BB | FileHBB))
           | (slide(SOUTH_WEST, sq, 0ULL) & ~(Rank1BB | FileABB))
           | (slide(NORTH_WEST, sq, 0ULL) & ~(Rank8BB | FileABB));
  BishopMagics[sq].relevant = mask;
  BishopMagics[sq].shift = POPCOUNT(mask);
}

static void
mask_relevant_rook_occupancy(Square sq)
{
  U64 mask = (slide(NORTH, sq, 0ULL) & ~Rank8BB)
           | (slide(EAST,  sq, 0ULL) & ~FileHBB)
           | (slide(SOUTH, sq, 0ULL) & ~Rank1BB)
           | (slide(WEST,  sq, 0ULL) & ~FileABB);
  RookMagics[sq].relevant = mask;
  RookMagics[sq].shift = POPCOUNT(mask);
}

static U64
mask_state(U64 mask, unsigned idx)
{
  U64 res = 0ULL;
  Square sq;
  int bit;
  for (bit = 0; mask; bit++) {
    sq = pop_lsb(&mask);
    if ((1 << bit) & idx)
      SET_BIT(res, sq);
  }
  return res;
}

U64
pawn_attacks_bb(Color c, Square sq)
{
  return pawn_attacks[c][sq];
}

void
print_mask(U64 bb)
{
  Square sq = SQ_A8;
  Rank r;
  File f;
  const char *sep = "  +---+---+---+---+---+---+---+---+";
  printf("%s\n", sep);
  for (r = Rank8; r >= Rank1; r--) {
    printf("%d ", r + 1);
    for (f = FileA; f <= FileH; f++)
      printf("| %c ", GET_BIT(bb, sq++) ? 'X' : ' ');
    printf("|\n%s\n", sep);
  }
  printf("    a   b   c   d   e   f   g   h\n\n");
  printf("    Bitboard: %lu\n", bb);
}

U64
shift(Direction dir, U64 mask)
{
  switch (dir) {
  case NORTH:       return mask >> 8;
  case NORTH_NORTH: return mask >> 16;
  case SOUTH:       return mask << 8;
  case SOUTH_SOUTH: return mask << 16;
  case WEST:        return (mask & ~FileABB) >> 1;
  case EAST:        return (mask & ~FileHBB) << 1;
  case NORTH_WEST:  return (mask & ~FileABB) >> 9;
  case NORTH_EAST:  return (mask & ~FileHBB) >> 7;
  case SOUTH_WEST:  return (mask & ~FileABB) << 7;
  case SOUTH_EAST:  return (mask & ~FileHBB) << 9;
  default: return 0ULL;
  }
}

static U64
slide(Direction dir, Square sq, U64 occ)
{
  U64 res = 0ULL, mask = GET_BITBOARD(sq);
  while ((mask = shift(dir, mask)) && !(mask & occ))
      res |= mask;
  return res | mask;
}
