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

/** \typedef Magic
 * Defines structure Magic.
 * \struct Magic
 * A structure for storing fancy magics.
 */
typedef struct Magic {
  /**{*/
  U64      relevant; /**< Mask of relevant bits. */
  U64      magic;    /**< Magic number. */
  U64     *attacks;  /**< Dynamic array of attacks. */
  unsigned shift;    /**< Number of relevant bits. */
  /**}*/
} Magic;

static void mask_king_attacks(Square sq);
static void mask_knight_attacks(Square sq);
static void mask_pawn_attacks(Square sq);
static void mask_relevant_bishop_occupancy(Square sq);
static void mask_relevant_rook_occupancy(Square sq);
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
static U64 pawn_attacks[2][64]; /* [Color][Square] */
static U64 knight_attacks[64];  /* [Square] */
static U64 king_attacks[64];    /* [Square] */
static Magic BishopMagics[64];  /* [Square] */
static Magic RookMagics[64];    /* [Square] */

void
init_bitboards(void)
{
  Square sq;
  for (sq = SQ_A8; sq <= SQ_H1; sq++) {
    mask_king_attacks(sq);
    mask_knight_attacks(sq);
    mask_pawn_attacks(sq);
    mask_relevant_bishop_occupancy(sq);
    mask_relevant_rook_occupancy(sq);
    print_mask(knight_attacks[sq]);
    getchar();
  }
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
  while ((mask = shift(dir, mask) & ~occ))
    res |= mask;
  return res;
}
