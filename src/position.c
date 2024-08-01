#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bitboards.h"
#include "knur.h"
#include "position.h"
#include "util.h"

#define START_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

INLINE void add_enpas(struct position *position, enum square square);
INLINE void add_piece(struct position *position, enum piece piece,
		      enum square square);
INLINE void del_enpas(struct position *position);
INLINE void del_piece(struct position *position, enum piece piece,
		      enum square square);
INLINE void flip_stm(struct position *position);
INLINE void update_castle(struct position *pos, enum square from,
			  enum square to);

void add_enpas(struct position *pos, enum square sq) { pos->st->enpas = sq; }

void add_piece(struct position *pos, enum piece pc, enum square sq)
{
	BB_SET(pos->color[PIECE_COLOR(pc)], sq);
	BB_SET(pos->piece[PIECE_TYPE(pc)], sq);
	BB_SET(pos->piece[ALL_PIECES], sq);
	pos->board[sq] = pc;
}

void del_enpas(struct position *pos)
{
	if (pos->st->enpas != SQ_NONE) {
		pos->st->enpas = SQ_NONE;
	}
}

void del_piece(struct position *pos, enum piece pc, enum square sq)
{
	BB_XOR(pos->color[PIECE_COLOR(pc)], sq);
	BB_XOR(pos->piece[PIECE_TYPE(pc)], sq);
	BB_XOR(pos->piece[ALL_PIECES], sq);
	pos->board[sq] = NO_PIECE;
}

void flip_stm(struct position *pos) { pos->stm ^= 1; }

void update_castle(struct position *pos, enum square from, enum square to)
{
	static const int castling_table[64] = {
	    13, 15, 15, 15, 5,  15, 15, 7,  15, 15, 15, 15, 15, 15, 15, 15,
	    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
	    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
	    15, 15, 15, 15, 15, 15, 15, 15, 14, 15, 15, 15, 10, 15, 15, 11,
	};

	pos->st->castle &= castling_table[from] & castling_table[to];
}

void pos_init(struct position *pos)
{
	pos->st = ecalloc(1, sizeof(struct position_state));
	pos_set_fen(pos, nullptr);
}

void pos_destroy(struct position *pos)
{
	struct position_state *st;
	while (pos->st) {
		st = pos->st;
		pos->st = st->prev;
		free(st);
	}
}

void pos_set_fen(struct position *pos, const char *fen)
{
	char c, *str, *saveptr = nullptr, *token;
	enum square sq;
	enum color color;
	struct position_state *st;

	while (pos->st->prev) {
		st = pos->st;
		pos->st = st->prev;
		free(st);
	}

	fen = fen == nullptr ? START_FEN : fen;

	pos->stm = WHITE;
	ARRAY_FILL(pos->color, 0);
	ARRAY_FILL(pos->piece, 0);
	ARRAY_FILL(pos->board, NO_PIECE);
	pos->game_ply = 0;

	pos->st->enpas = SQ_NONE;
	pos->st->castle = 0;
	pos->st->fifty_rule = 0;
	pos->st->captured = NO_PIECE;
	pos->st->checkers = 0;
	pos->st->prev = nullptr;

	str = strdup(fen);
	token = strtok_r(str, " ", &saveptr);

	/* piece placement */
	for (sq = 0; sq < SQUARE_NB && *token;) {
		c = *token++;
		if (c == '/')
			continue;
		if (isdigit(c)) {
			sq += c - '0';
			continue;
		}
		color = isupper(c) ? WHITE : BLACK;
		switch (tolower(c)) {
		case 'p': add_piece(pos, PIECE_MAKE(PAWN, color), sq); break;
		case 'n': add_piece(pos, PIECE_MAKE(KNIGHT, color), sq); break;
		case 'b': add_piece(pos, PIECE_MAKE(BISHOP, color), sq); break;
		case 'r': add_piece(pos, PIECE_MAKE(ROOK, color), sq); break;
		case 'q': add_piece(pos, PIECE_MAKE(QUEEN, color), sq); break;
		case 'k': add_piece(pos, PIECE_MAKE(KING, color), sq); break;
		default:  die("wrong fen");
		}
		sq++;
	}

	/* side to move */
	token = strtok_r(nullptr, " ", &saveptr);
	if (*token == 'b')
		flip_stm(pos);

	/* castling rights */
	token = strtok_r(nullptr, " ", &saveptr);
	while ((c = *token++)) {
		switch (c) {
		case 'K': pos->st->castle |= 4; break;
		case 'Q': pos->st->castle |= 1; break;
		case 'k': pos->st->castle |= 8; break;
		case 'q': pos->st->castle |= 2; break;
		case '-': break;
		default:  die("wrong fen");
		}
	}

	/* en passant square */
	token = strtok_r(nullptr, " ", &saveptr);
	if (*token != '-') {
		c = *token++;
		sq = (c - 'a') + 8 * (8 - (*token - '0'));
		add_enpas(pos, sq);
	}

	/* half move clock */
	pos->st->fifty_rule = atoi(strtok_r(nullptr, " ", &saveptr));

	/* full move counter */
	pos->game_ply = atoi(strtok_r(nullptr, " ", &saveptr));

	/* additional information */
	pos->st->checkers =
	    pos_attackers(
		pos, BB_TO_SQUARE(pos->piece[KING] & pos->color[pos->stm])) &
	    pos->color[!pos->stm];

	free(str);
}

void pos_print(const struct position *pos)
{
	enum square sq = 0;
	const char *ptc = "PNBRQK";
	int rank, file, c;
	const char *sep = "+---+---+---+---+---+---+---+---+";
	printf("  %s\n", sep);
	for (rank = 8; rank >= 1; rank--) {
		printf("%d |", rank);
		for (file = 0; file < 8; file++, sq++) {
			c = BB_TEST(pos->color[BLACK], sq) ? 32 : 0;
			c |= pos->board[sq] == NO_PIECE
			       ? ' '
			       : ptc[PIECE_TYPE(pos->board[sq])];
			printf(" %c |", c);
		}
		printf("\n  %s\n", sep);
	}
	printf("    a   b   c   d   e   f   g   h\n");
	printf("    Turn: %s     Enpassant: %s\n",
	       pos->stm == WHITE ? "WHITE" : "BLACK", SQ_STR(pos->st->enpas));
	printf("    Castling:       %c%c%c%c\n",
	       pos->st->castle & 4 ? 'K' : '-', pos->st->castle & 1 ? 'Q' : '-',
	       pos->st->castle & 8 ? 'k' : '-',
	       pos->st->castle & 2 ? 'q' : '-');
}

void pos_do_move(struct position *pos, enum move m)
{
	const enum color us = pos->stm, them = !us;
	const enum direction up = us == WHITE ? NORTH : SOUTH;
	const enum square from = MOVE_FROM(m), to = MOVE_TO(m);
	const enum piece pc = pos->board[from], captured = pos->board[to];
	struct position_state *st = ecalloc(1, sizeof(struct position_state));

	*st = *(pos->st);
	st->fifty_rule++;
	if (pc == PAWN || captured != NO_PIECE)
		st->fifty_rule = 0;
	st->captured = captured;
	st->prev = pos->st;
	pos->st = st;
	pos->game_ply++;

	if (captured != NO_PIECE)
		del_piece(pos, captured, to);
	del_piece(pos, pc, from);
	add_piece(pos, pc, to);

	del_enpas(pos);

	if (PIECE_TYPE(pc) == PAWN) {
		if (from + 2 * up == to) {
			add_enpas(pos, to + (us == WHITE ? SOUTH : NORTH));
		} else if (MOVE_TYPE(m) == MT_PROMOTION) {
			del_piece(pos, pc, to);
			add_piece(pos, PIECE_MAKE(MOVE_PROMOTION(m), us), to);
		} else if (MOVE_TYPE(m) == MT_ENPASSANT) {
			del_piece(pos, PIECE_MAKE(PAWN, them), to - up);
		}
	} else if (PIECE_TYPE(pc) == KING) {
		if (MOVE_TYPE(m) == MT_CASTLE) {
			if (to < from) { // queenside (long)
				del_piece(pos, PIECE_MAKE(ROOK, us),
					  to + 2 * WEST);
				add_piece(pos, PIECE_MAKE(ROOK, us), to + EAST);
			} else { // kingside (short)
				del_piece(pos, PIECE_MAKE(ROOK, us), to + EAST);
				add_piece(pos, PIECE_MAKE(ROOK, us), to + WEST);
			}
		}
	}

	/* TODO: optimize */
	pos->st->checkers = pos_attackers(pos, BB_TO_SQUARE(pos->piece[KING] &
							    pos->color[them])) &
			    pos->color[us];

	flip_stm(pos);
	update_castle(pos, from, to);
}

void pos_undo_move(struct position *pos, enum move m)
{
	flip_stm(pos);

	const enum color us = pos->stm, them = !us;
	const enum direction up = us == WHITE ? NORTH : SOUTH;
	const enum square from = MOVE_FROM(m), to = MOVE_TO(m);
	const enum piece pc = MOVE_TYPE(m) == MT_PROMOTION
				? PIECE_MAKE(PAWN, us)
				: pos->board[to];
	const enum piece captured = pos->st->captured;
	struct position_state *st = pos->st->prev;

	if (PIECE_TYPE(pc) == PAWN) {
		if (from + 2 * up == to) {
			del_enpas(pos);
		} else if (MOVE_TYPE(m) == MT_PROMOTION) {
			del_piece(pos, PIECE_MAKE(MOVE_PROMOTION(m), us), to);
			add_piece(pos, pc, to);
		} else if (MOVE_TYPE(m) == MT_ENPASSANT) {
			add_piece(pos, PIECE_MAKE(PAWN, them), to - up);
		}
	} else if (PIECE_TYPE(pc) == KING) {
		if (MOVE_TYPE(m) == MT_CASTLE) {
			if (to < from) { // queenside (long)
				del_piece(pos, PIECE_MAKE(ROOK, us), to + EAST);
				add_piece(pos, PIECE_MAKE(ROOK, us),
					  to + 2 * WEST);
			} else { // kingside (short)
				del_piece(pos, PIECE_MAKE(ROOK, us), to + WEST);
				add_piece(pos, PIECE_MAKE(ROOK, us), to + EAST);
			}
		}
	}

	del_piece(pos, pc, to);
	add_piece(pos, pc, from);

	if (captured != NO_PIECE)
		add_piece(pos, captured, to);

	free(pos->st);
	pos->st = st;

	pos->game_ply--;
}
u64 pos_attackers(const struct position *pos, enum square sq)
{
	return pos_attackers_occ(pos, sq, pos->piece[ALL_PIECES]);
}

/* clang-format off */
u64 pos_attackers_occ(const struct position *pos, enum square sq, u64 occ)
{
	return (((bb_pawn_attacks(WHITE, sq) & pos->color[BLACK])
	     |   (bb_pawn_attacks(BLACK, sq) & pos->color[WHITE])) & pos->piece[PAWN])
	     |   (bb_attacks(KNIGHT, sq, occ) &  pos->piece[KNIGHT])
	     |   (bb_attacks(BISHOP, sq, occ) & (pos->piece[BISHOP] | pos->piece[QUEEN]))
	     |   (bb_attacks(ROOK,   sq, occ) & (pos->piece[  ROOK] | pos->piece[QUEEN]))
	     |   (bb_attacks(KING,   sq, occ) &  pos->piece[  KING]);
}
/* clang-format on */
