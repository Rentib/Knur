#ifndef KNUR_POSITION_H_
#define KNUR_POSITION_H_

#include "knur.h"

struct position_state {
	enum square enpas;   /* enpassant square */
	int castle;          /* castling rights */
	int fifty_rule;      /* fifty move rule */
	enum piece captured; /* captured piece */
	u64 checkers;        /* bitboard of pieces giving a check */

	struct position_state *prev; /* pointer to previous position state */
};

struct position {
	enum color stm;           /* side to move */
	u64 color[COLOR_NB];      /* [color] bitboards for colors */
	u64 piece[PIECE_TYPE_NB]; /* [piece type] bitboards for piece types */
	enum piece board[SQUARE_NB]; /* [square] piece on each square */
	int game_ply;                /* game halfmove counter */

	struct position_state *st; /* state of position */
};

void pos_init(struct position *position);
void pos_destroy(struct position *position);
void pos_set_fen(struct position *position, const char *fen);
void pos_print(const struct position *position);

void pos_do_move(struct position *position, enum move move);
void pos_undo_move(struct position *position, enum move move);

bool pos_is_legal(const struct position *position, enum move move);

u64 pos_attackers(const struct position *position, enum square);
u64 pos_attackers_occ(const struct position *position, enum square square,
		      u64 occupancy);

#endif /* KNUR_POSITION_H_ */