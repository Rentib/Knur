#include "movegen.h"
#include "bitboards.h"
#include "knur.h"
#include "position.h"

INLINE enum move *add_promotions(enum direction dir, enum move *move_list,
				 enum square to)
{
	*move_list++ = MAKE_PROMOTION(to - dir, to, QUEEN);
	*move_list++ = MAKE_PROMOTION(to - dir, to, KNIGHT);
	*move_list++ = MAKE_PROMOTION(to - dir, to, ROOK);
	*move_list++ = MAKE_PROMOTION(to - dir, to, BISHOP);
	return move_list;
}

static enum move *castle_moves(enum move *move_list,
			       const struct position *position);
static enum move *pawn_moves(enum mg_type mt, enum move *move_list,
			     const struct position *position, u64 target);
static enum move *piece_moves(enum piece_type piece, enum move *move_list,
			      const struct position *position, u64 target);

enum move *castle_moves(enum move *move_list, const struct position *pos)
{
	(void)move_list;
	(void)pos;
	const enum color us = pos->stm, them = !us;
	const enum square ksq = BB_TO_SQUARE(pos->piece[KING] & pos->color[us]);
	const u64 occ = pos->piece[ALL_PIECES];
	// queenside
	if ((pos->st->castle & (1 << us)) &&
	    !(occ & bb_between(ksq - 3, ksq - 1)) &&
	    !(pos->color[them] & pos_attackers(pos, ksq + WEST)))
		*move_list++ = MAKE_CASTLE(ksq, ksq - 2);
	// kingside
	if ((pos->st->castle & (4 << us)) &&
	    !(occ & bb_between(ksq + 1, ksq + 2)) &&
	    !(pos->color[them] & pos_attackers(pos, ksq + EAST)))
		*move_list++ = MAKE_CASTLE(ksq, ksq + 2);
	return move_list;
}

enum move *pawn_moves(enum mg_type mt, enum move *move_list,
		      const struct position *pos, u64 target)
{
	const enum color us = pos->stm, them = !us;
	const enum direction up = us == WHITE ? NORTH : SOUTH;
	const enum direction upe = up + EAST, upw = up + WEST;
	const u64 rank4 = us == WHITE ? BB_RANK_4 : BB_RANK_5;
	const u64 rank7 = us == WHITE ? BB_RANK_7 : BB_RANK_2;
	const u64 empty = ~pos->piece[ALL_PIECES] & target;
	const u64 enemies = pos->color[them] & target;
	const u64 pawns = pos->piece[PAWN] & pos->color[us] & ~rank7;
	const u64 promo = pos->piece[PAWN] & pos->color[us] & rank7;
	u64 b1, b2;
	enum square to;

	// quiet moves
	if (mt != MGT_CAPTURES) {
		b1 = bb_shift(pawns, up) & ~pos->piece[ALL_PIECES];
		b2 = bb_shift(b1, up) & empty & rank4;
		b1 &= target;

		while (b1) {
			to = bb_poplsb(&b1);
			*move_list++ = MAKE_MOVE(to - up, to);
		}
		while (b2) {
			to = bb_poplsb(&b2);
			*move_list++ = MAKE_MOVE(to - up - up, to);
		}
	}

	// captures
	if (mt != MGT_QUIET) {
		b1 = bb_shift(pawns, upe) & enemies;
		b2 = bb_shift(pawns, upw) & enemies;

		while (b1) {
			to = bb_poplsb(&b1);
			*move_list++ = MAKE_MOVE(to - upe, to);
		}
		while (b2) {
			to = bb_poplsb(&b2);
			*move_list++ = MAKE_MOVE(to - upw, to);
		}

		if (pos->st->enpas != SQ_NONE) {
			for (b1 = pawns & bb_pawn_attacks(them, pos->st->enpas);
			     b1;)
				*move_list++ = MAKE_ENPASSANT(bb_poplsb(&b1),
							      pos->st->enpas);
		}
	}

	// promotions
	if (mt != MGT_CAPTURES) {
		b1 = bb_shift(promo, up) & empty;
		while (b1)
			move_list =
			    add_promotions(up, move_list, bb_poplsb(&b1));
	}
	if (mt != MGT_QUIET) {
		b1 = bb_shift(promo, upe) & enemies;
		b2 = bb_shift(promo, upw) & enemies;
		while (b1)
			move_list =
			    add_promotions(upe, move_list, bb_poplsb(&b1));
		while (b2)
			move_list =
			    add_promotions(upw, move_list, bb_poplsb(&b2));
	}

	return move_list;
}

enum move *piece_moves(enum piece_type pt, enum move *move_list,
		       const struct position *pos, u64 target)
{
	u64 pieces = pos->piece[pt] & pos->color[pos->stm], attacks;
	enum square from;
	while (pieces) {
		from = bb_poplsb(&pieces);
		attacks = bb_attacks(pt, from, pos->piece[ALL_PIECES]) & target;
		while (attacks)
			*move_list++ = MAKE_MOVE(from, bb_poplsb(&attacks));
	}
	return move_list;
}

enum move *mg_generate(enum mg_type mt, enum move *move_list,
		       const struct position *pos)
{
	const enum color us = pos->stm, them = !us;
	const enum square ksq = BB_TO_SQUARE(pos->piece[KING] & pos->color[us]);
	u64 target = mt == MGT_CAPTURES ? pos->color[them]
		   : mt == MGT_QUIET    ? ~pos->piece[ALL_PIECES]
					: ~pos->color[us];

	move_list = piece_moves(KING, move_list, pos, target);
	if (BB_SEVERAL(pos->st->checkers))
		return move_list;

	if (pos->st->checkers)
		target &= bb_between(ksq, BB_TO_SQUARE(pos->st->checkers));
	else if (mt != MGT_CAPTURES)
		move_list = castle_moves(move_list, pos);
	move_list = pawn_moves(mt, move_list, pos, target);
	move_list = piece_moves(KNIGHT, move_list, pos, target);
	move_list = piece_moves(BISHOP, move_list, pos, target);
	move_list = piece_moves(ROOK, move_list, pos, target);
	move_list = piece_moves(QUEEN, move_list, pos, target);

	return move_list;
}
