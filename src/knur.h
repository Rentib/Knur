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

#ifndef KNUR_H_
#define KNUR_H_

/* search defines */
#define INFINITY         69000
#define CHECKMATE        32000
#define STALEMATE        0
#define MAX_PLY          64
#define ISCHECKMATE      (CHECKMATE - MAX_PLY)

typedef enum {
  WHITE, BLACK
} Color;

typedef enum {
  PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, NONE
} PieceType;

typedef enum {
  SQ_A8, SQ_B8, SQ_C8, SQ_D8, SQ_E8, SQ_F8, SQ_G8, SQ_H8,
  SQ_A7, SQ_B7, SQ_C7, SQ_D7, SQ_E7, SQ_F7, SQ_G7, SQ_H7,
  SQ_A6, SQ_B6, SQ_C6, SQ_D6, SQ_E6, SQ_F6, SQ_G6, SQ_H6,
  SQ_A5, SQ_B5, SQ_C5, SQ_D5, SQ_E5, SQ_F5, SQ_G5, SQ_H5,
  SQ_A4, SQ_B4, SQ_C4, SQ_D4, SQ_E4, SQ_F4, SQ_G4, SQ_H4,
  SQ_A3, SQ_B3, SQ_C3, SQ_D3, SQ_E3, SQ_F3, SQ_G3, SQ_H3,
  SQ_A2, SQ_B2, SQ_C2, SQ_D2, SQ_E2, SQ_F2, SQ_G2, SQ_H2,
  SQ_A1, SQ_B1, SQ_C1, SQ_D1, SQ_E1, SQ_F1, SQ_G1, SQ_H1,
  SQ_NONE
} Square;

enum { Rank1, Rank2, Rank3, Rank4, Rank5, Rank6, Rank7, Rank8 };
enum { FileA, FileB, FileC, FileD, FileE, FileF, FileG, FileH };
typedef int Rank;
typedef int File;

typedef enum {
  NORTH = -8,
  EAST  =  1,
  SOUTH =  8,
  WEST  = -1,
  NORTH_EAST = NORTH + EAST,
  SOUTH_EAST = SOUTH + EAST,
  SOUTH_WEST = SOUTH + WEST,
  NORTH_WEST = NORTH + WEST,
  NORTH_NORTH = NORTH + NORTH,
  SOUTH_SOUTH = SOUTH + SOUTH,
} Direction;


/** \enum Move
 * Enumerator of chess moves. A chess move is represented by a 32 bit integer.
 * 000000000000000|00|00|000000|000000
 *    move score  |pt|mt|fromsq|tosq
 * pt     - promotion piece type
 * mt     - move type
 * fromsq - from square
 * tosq   - to square
 */
typedef enum {
  MOVE_NONE = 0,
} Move;

typedef enum {
  NORMAL     = 0 << 12,
  PROMOTION  = 1 << 12,
  CASTLE     = 2 << 12,
  EN_PASSANT = 3 << 12,
} MoveType;

#define TO_SQ(move)             ((Square)((move) & 0x3F))
#define FROM_SQ(move)           ((Square)(((move) >> 6) & 0x3F))
#define TYPE_OF(move)           ((MoveType)((move) & (3 << 12)))
#define PROMOTION_TYPE(move)    ((PieceType)((((move) >> 14)) & 3) + KNIGHT)
#define MAKE_MOVE(from, to)     ((Move)(((from) << 6) + (to)))
#define MAKE_PROMOTION(from, to, pt) \
  ((Move)((((pt) - KNIGHT) << 14) + PROMOTION + ((from) << 6) + (to)))
#define MAKE_CASTLE(from, to)        \
  ((Move)(CASTLE + ((from) << 6) + (to)))
#define MAKE_EN_PASSANT(from, to)    \
  ((Move)(EN_PASSANT + ((from) << 6) + (to)))

#endif /* KNUR_H_ */
