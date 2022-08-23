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

#ifndef KNUR_BITBOARDS_H_
#define KNUR_BITBOARDS_H_

#include <stdint.h>

#include "knur.h"

#define GET_BIT(bitboard, square)    ((bitboard) & (1ULL << (square)))
#define SET_BIT(bitboard, square)    ((bitboard) |= (1ULL << (square)))
#define GET_BITBOARD(square)         (1ULL << (square))
#define GET_SQUARE(bitboard)         ((Square)(__builtin_ctzll(bitboard)))
#define POPCOUNT(bitboard)           (__builtin_popcountll(bitboard))
#define LSB(bitboard)                ((bitboard) & -(bitboard))

typedef uint64_t U64;

extern const U64 Rank1BB;
extern const U64 Rank2BB;
extern const U64 Rank3BB;
extern const U64 Rank4BB;
extern const U64 Rank5BB;
extern const U64 Rank6BB;
extern const U64 Rank7BB;
extern const U64 Rank8BB;

extern const U64 FileABB;
extern const U64 FileBBB;
extern const U64 FileCBB;
extern const U64 FileDBB;
extern const U64 FileEBB;
extern const U64 FileFBB;
extern const U64 FileGBB;
extern const U64 FileHBB;

U64 attacks_bb(PieceType pt, Square sq, U64 occ);
void free_bitboards(void);
void init_bitboards(void);
U64 pawn_attacks_bb(Color c, Square sq);
void print_mask(U64 mask);
U64 shift(Direction dir, U64 mask);

inline Square
pop_lsb(U64 *mask)
{
  U64 x = LSB(*mask);
  *mask ^= x;
  return GET_SQUARE(x);
}

#endif /* KNUR_BITBOARDS_H_ */
