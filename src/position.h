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

/** \typedef Key
 * Defines Key as a 64 bit unsigned integer.
 */
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
  int       material[2]; /**< [Color] Material of each side in centipawns; */
  Key       key;         /**< Zobrist hash; */
  State    *st;          /**< Position state. */
};

/** \brief Prints board.
 * \param[in] pos - pointer to a structure containing a position.
 */
void pos_print(const Position *pos);

/** \brief Sets position to given FEN.
 * \param[in,out] pos - pointer to a structure containing a position;
 * \param[in] fen     - pointer to a FEN string.
 */
void pos_set(Position *pos, const char *fen);

/** \brief Makes a move.
 * \param[in,out] pos - pointer to a structure containing a position;
 * \param[in] m       - move to be made.
 */
void do_move(Position *pos, Move m);

/** \brief Undoes a move.
 * \param[in,out] pos - pointer to a structure containing a position;
 * \param[in] m       - move to be undone.
 */
void undo_move(Position *pos, Move m);

/** \brief Initialises numbers used for zobrist hashing.
 * https://www.chessprogramming.org/Zobrist_Hashing
 */
void zobrist_init(void);

/** \brief Checks if a move is legal.
 * \param[in] pos - pointer to a structure containing a position;
 * \param[in] m   - move to be checked.
 * \return 0 if move \p m is illegal, nonzero otherwise.
 */
int is_legal(const Position *pos, Move m);

/** \brief Returns a bitboard of attackers to a given square.
 * \param[in] pos - pointer to a structure containing a position;
 * \param[in] sq  - square to be checked;
 * \param[in] occ - bitboard of occupied squares.
 * \return A bitboard of pieces attacking square \p sq.
 */
U64 attackers_to(const Position *pos, Square sq, U64 occ);

#endif /* KNUR_POSITION_H_ */
