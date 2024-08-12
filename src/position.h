#ifndef KNUR_POSITION_H_
#define KNUR_POSITION_H_

#include "knur.h"

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

	struct position_state state_stack[MAX_MOVES]; /* state stack */
	struct position_state *st;                    /* state of position */
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

#endif /* KNUR_POSITION_H_ */
