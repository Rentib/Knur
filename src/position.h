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

#ifndef KNUR_POSITION_H_
#define KNUR_POSITION_H_

#include <stdint.h>

#include "bitboards.h"
#include "knur.h"

typedef uint64_t Key;

/** \typedef State
 * Defines structure State.
 */
typedef struct State State;

/** \struct State
 * A structure for storing position data which cannot be reverse calculated.
 */
struct State {
  /**{*/
  Square    enpas;    /**< En passant square; */
  int       castle;   /**< Bitfield for storing castle rights (QqKk); */
  int       fifty;    /**< Fifty move rule; */
  PieceType captured; /**< Captured piece type; */
  State    *prev;     /**< Pointer to previous state; */
  /**}*/
};

/**< \typedef Position
 * Defines structure Position.
 */
typedef struct Position Position;

/**< \struct Position
 * A structure for storing position data.
 */
struct Position {
  Color     turn;        /**< Color of side to move; */
  U64       empty;       /**< Bitboard of empty squares; */
  U64       color[2];    /**< [Color] Bitboards of WHITE and BLACK pieces; */
  U64       piece[6];    /**< [PieceType] Bitboards of all pieces; */
  PieceType board[64];   /**< [Square] Piece standing on each square; */
  Square    ksq[2];      /**< [Color] Square of WHITE and BLACK king; */
  int       game_ply;    /**< PLY of game; */
  int       ply;         /**< PLY of search; */
  Key       key;
  State    *st;          /**< Position state. */
};

void pos_print(const Position *pos);
void pos_set(Position *pos, const char *fen);
void do_move(Position *pos, Move m);
void undo_move(Position *pos, Move m);
void zobrist_init(void);
int is_legal(const Position *pos, Move m);
U64 attackers_to(const Position *pos, Square sq, U64 occ);

#endif /* KNUR_POSITION_H_ */
