#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bitboards.h"
#include "knur.h"
#include "movegen.h"
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

static struct {
	u64 piece_square[PIECE_NB][SQUARE_NB]; /* [piece][square] */
	u64 side;                              /* side to move */
	u64 castle[16];                        /* [castle mask] */
	u64 enpassant[8];                      /* [file] */
} zobrist;

void add_enpas(struct position *pos, enum square sq)
{
	pos->key ^= zobrist.enpassant[SQ_FILE(sq)];
	pos->st->enpas = sq;
}

void add_piece(struct position *pos, enum piece pc, enum square sq)
{
	pos->key ^= zobrist.piece_square[pc][sq];
	BB_SET(pos->color[PIECE_COLOR(pc)], sq);
	BB_SET(pos->piece[PIECE_TYPE(pc)], sq);
	BB_SET(pos->piece[ALL_PIECES], sq);
	pos->board[sq] = pc;
}

void del_enpas(struct position *pos)
{
	if (pos->st->enpas != SQ_NONE) {
		pos->key ^= zobrist.enpassant[SQ_FILE(pos->st->enpas)];
		pos->st->enpas = SQ_NONE;
	}
}

void del_piece(struct position *pos, enum piece pc, enum square sq)
{
	pos->key ^= zobrist.piece_square[pc][sq];
	BB_XOR(pos->color[PIECE_COLOR(pc)], sq);
	BB_XOR(pos->piece[PIECE_TYPE(pc)], sq);
	BB_XOR(pos->piece[ALL_PIECES], sq);
	pos->board[sq] = NO_PIECE;
}

void flip_stm(struct position *pos)
{
	pos->key ^= zobrist.side;
	pos->stm ^= 1;
}

void update_castle(struct position *pos, enum square from, enum square to)
{
	static const int castling_table[64] = {
	    13, 15, 15, 15, 5,  15, 15, 7,  15, 15, 15, 15, 15, 15, 15, 15,
	    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
	    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
	    15, 15, 15, 15, 15, 15, 15, 15, 14, 15, 15, 15, 10, 15, 15, 11,
	};

	pos->key ^= zobrist.castle[pos->st->castle];
	pos->st->castle &= castling_table[from] & castling_table[to];
	pos->key ^= zobrist.castle[pos->st->castle];
}

void pos_init(void)
{
	unsigned i, j;
	for (i = 0; i < PIECE_NB; i++)
		for (j = 0; j < SQUARE_NB; j++)
			zobrist.piece_square[i][j] = rand_u64();
	zobrist.side = rand_u64();
	for (i = 0; i < 16; i++)
		zobrist.castle[i] = rand_u64();
	for (i = 0; i < 8; i++)
		zobrist.enpassant[i] = rand_u64();
}

void pos_set_fen(struct position *pos, const char *fen)
{
	char c, *str, *saveptr = nullptr, *token;
	enum square sq;
	enum color color;

	memset(pos, 0, sizeof(struct position));

	fen = fen == nullptr ? START_FEN : fen;

	pos->stm = WHITE;
	ARRAY_FILL(pos->color, 0);
	ARRAY_FILL(pos->piece, 0);
	ARRAY_FILL(pos->board, NO_PIECE);
	ARRAY_FILL(pos->reps, 0);
	pos->game_ply = 0;

	pos->st = pos->state_stack;
	pos->st->enpas = SQ_NONE;
	pos->st->castle = 0;
	pos->st->fifty_rule = 0;
	pos->st->captured = NO_PIECE;
	pos->st->checkers = 0;
	pos->st = pos->state_stack;

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
	pos->key ^= zobrist.castle[pos->st->castle];

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
	pos->reps[pos->game_ply - 1] = pos->key;

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
	printf("    Key:            %016lX\n", pos->key);
}

void pos_do_move(struct position *pos, enum move m)
{
	const enum color us = pos->stm, them = !us;
	const enum direction up = us == WHITE ? NORTH : SOUTH;
	const enum square from = MOVE_FROM(m), to = MOVE_TO(m);
	const enum piece pc = pos->board[from], captured = pos->board[to];
	struct position_state *st = pos->st + 1;

	*st = *(pos->st);
	st->fifty_rule++;
	if (pc == PAWN || captured != NO_PIECE)
		st->fifty_rule = 0;
	st->captured = captured;
	pos->st = st;

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
			if (to < from) { /* queenside (long) */
				del_piece(pos, PIECE_MAKE(ROOK, us),
					  to + 2 * WEST);
				add_piece(pos, PIECE_MAKE(ROOK, us), to + EAST);
			} else { /* kingside (short) */
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

	pos->reps[pos->game_ply++] = pos->key;
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
	struct position_state *st = pos->st - 1;

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
			if (to < from) { /* queenside (long) */
				del_piece(pos, PIECE_MAKE(ROOK, us), to + EAST);
				add_piece(pos, PIECE_MAKE(ROOK, us),
					  to + 2 * WEST);
			} else { /* kingside (short) */
				del_piece(pos, PIECE_MAKE(ROOK, us), to + WEST);
				add_piece(pos, PIECE_MAKE(ROOK, us), to + EAST);
			}
		}
	}

	del_piece(pos, pc, to);
	add_piece(pos, pc, from);

	if (captured != NO_PIECE)
		add_piece(pos, captured, to);
	pos->key ^= zobrist.castle[pos->st->castle];
	pos->key ^= zobrist.castle[st->castle];
	if (st->enpas != SQ_NONE)
		pos->key ^= zobrist.enpassant[SQ_FILE(st->enpas)];
	pos->st = st;

	pos->game_ply--;
}

bool pos_is_draw(const struct position *pos)
{
	int n = 1, i = pos->game_ply;

	/* fifty move rule */
	if (pos->st->fifty_rule >= 100)
		return true;

	/* 3-folr repetition */
	while (i >= pos->game_ply - (int)pos->st->fifty_rule && n < 3)
		n += pos->key == pos->reps[--i];
	return n >= 3;
}

bool pos_is_legal(const struct position *pos, enum move m)
{
	const enum color us = pos->stm, them = !us;
	const enum square from = MOVE_FROM(m), to = MOVE_TO(m),
			  ksq = BB_TO_SQUARE(pos->piece[KING] & pos->color[us]);
	const enum direction down = us == WHITE ? SOUTH : NORTH;
	u64 enemies = pos->color[them];
	u64 occ = pos->piece[ALL_PIECES];

	BB_RESET(enemies, to);
	BB_XOR(occ, from);
	BB_SET(occ, to);

	if (ksq == from)
		return !(pos_attackers_occ(pos, to, occ) & enemies);

	if (MOVE_TYPE(m) == MT_ENPASSANT) {
		BB_XOR(enemies, to + down);
		BB_XOR(occ, to + down);
	}

	/* check for discovered checks */
	return !(((bb_attacks(ROOK, ksq, occ) &
		   (pos->piece[ROOK] | pos->piece[QUEEN])) |
		  (bb_attacks(BISHOP, ksq, occ) &
		   (pos->piece[BISHOP] | pos->piece[QUEEN]))) &
		 enemies);
}

bool pos_is_pseudo_legal(const struct position *pos, enum move m)
{
	enum color us = pos->stm, them = !us;
	enum square from = MOVE_FROM(m), to = MOVE_TO(m),
		    ksq = BB_TO_SQUARE(pos->piece[KING] & pos->color[us]);
	enum piece pc = pos->board[from];
	enum direction up = us == WHITE ? NORTH : SOUTH;
	enum move move_list[256], *last;

	if (m == MOVE_NONE || pc == NO_PIECE || PIECE_COLOR(pc) != us ||
	    BB_TEST(pos->piece[ALL_PIECES] & pos->color[us], to))
		return false;

	if (MOVE_TYPE(m) != MT_NORMAL) {
		last = mg_generate(MGT_SPECIAL, move_list, pos);
		while (last-- != move_list)
			if (*last == m)
				return true;
		return false;
	}

	if (PIECE_TYPE(pc) == PAWN) {
		if (SQ_RANK(to) == 0 || SQ_RANK(to) == 7)
			return false;

		if (!BB_TEST(bb_pawn_attacks(us, from) & pos->color[them],
			     to) &&
		    !(from + up == to && pos->board[to] == NO_PIECE) &&
		    !(from + up * 2 == to &&
		      pos->board[from + up * 1] == NO_PIECE &&
		      pos->board[from + up * 2] == NO_PIECE &&
		      (SQ_RANK(from) == 1 || SQ_RANK(from) == 6))) {
			return false;
		}
	} else if (!BB_TEST(
		       bb_attacks(PIECE_TYPE(pc), from, pos->piece[ALL_PIECES]),
		       to))
		return false;

	if (pos->st->checkers) {
		if (PIECE_TYPE(pc) != KING) {
			if (BB_SEVERAL(pos->st->checkers))
				return false;
			if (!BB_TEST(bb_between(ksq, BB_LSB(pos->st->checkers)),
				     to))
				return false;
		} else if (pos_attackers_occ(pos, to,
					     pos->piece[ALL_PIECES] ^
						 BB_FROM_SQUARE(from)) &
			   pos->color[them])
			return false;
	}

	return true;
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
