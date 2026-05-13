/*
  Knur, a UCI chess engine.
  Copyright (C) 2024-2026 Stanisław Bitner

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KNUR_POSITION_H_
#define KNUR_POSITION_H_

#include "knur.h"
#if USE_NNUE
#include "nnue.h"
#endif

struct position_state {
	enum square enpas;   /* enpassant square */
	int castle;          /* castling rights */
	int fifty_rule;      /* fifty move rule */
	enum piece captured; /* captured piece */
	u64 checkers;        /* bitboard of pieces giving a check */
};

struct position {
	enum color stm;              /* side to move */
	u64 color[COLOR_NB];         /* [color] colors' bitboards */
	u64 piece[PIECE_TYPE_NB];    /* [piece type] piece types' bitboards */
	enum piece board[SQUARE_NB]; /* [square] piece on each square */
	int game_ply;                /* game halfmove counter */
	u64 reps[MAX_MOVES];         /* [game ply] repetition array */
	u64 key;                     /* zobrist hash */
#if !USE_NNUE
	u64 pawn_key; /* zobrist hash for pawns */
#endif

	struct position_state state_stack[MAX_MOVES]; /* state stack */
	struct position_state *st;                    /* position's state */

#if USE_NNUE
	struct accumulator accumulator_stack[MAX_MOVES]; /* accumulator stack */
	struct accumulator *acc;                         /* position's accumulator */
#endif
};

void pos_init(void);

void pos_set_fen(struct position *position, const char *fen);
void pos_print(const struct position *position);

void pos_do_move(struct position *position, enum move move);
void pos_undo_move(struct position *position, enum move move);

void pos_do_null_move(struct position *position);
void pos_undo_null_move(struct position *position);

INLINE bool pos_is_quiet(const struct position *position, enum move move)
{
	return MOVE_TYPE(move) != MT_ENPASSANT &&
	       position->board[MOVE_TO(move)] == NO_PIECE;
}

bool pos_is_draw(const struct position *position);
bool pos_is_legal(const struct position *position, enum move move);
bool pos_is_pseudo_legal(const struct position *position, enum move move);

u64 pos_attackers(const struct position *position, enum square);
u64 pos_attackers_occ(const struct position *position, enum square square,
		      u64 occupancy);
INLINE u64 pos_non_pawn(const struct position *position, enum color side)
{
	return (position->piece[ALL_PIECES] ^ position->piece[PAWN] ^
		position->piece[KING]) &
	       position->color[side];
}

#endif /* KNUR_POSITION_H_ */
