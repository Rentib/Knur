#include "evaluate.h"
#include "bitboards.h"
#include "knur.h"
#include "position.h"
#include "transposition.h"

#define S(mg, eg)  ((int)((unsigned)(eg) << 16) + (mg))
#define SMG(score) ((int16_t)((uint16_t)((unsigned)((score) + 0x0000) >> 00)))
#define SEG(score) ((int16_t)((uint16_t)((unsigned)((score) + 0x8000) >> 16)))

struct eval_params eval_params = {
    .piece_value = {S(101, 101), S(299, 299), S(316, 316), S(497, 497),
		    S(901, 901), S(0, 0)},

    /* clang-format off */
    .pawn_pcsqt = {
	    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
	    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
	    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
	    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
	    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
	    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
	    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
	    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    },
    .knight_pcsqt = {
	    S(-15, -25), S(0, 0),  S(0, 0),  S(0, 0),  S(0, 0),  S(0, 0),  S(0, 0),  S(-15, -25),
	    S(-10, -20), S(0, 0),  S(0, 0),  S(0, 0),  S(0, 0),  S(0, 0),  S(0, 0),  S(-10, -20),
	    S(-10, -20), S(0, 0),  S(10, 0), S(10, 0), S(10, 0), S(10, 0), S(0, 0),  S(-10, -20),
	    S(-10, -20), S(5, 0),  S(10, 0), S(20, 0), S(20, 0), S(10, 0), S(5, 0),  S(-10, -20),
	    S(-10, -20), S(5, 0),  S(10, 0), S(20, 0), S(20, 0), S(10, 0), S(5, 0),  S(-10, -20),
	    S(-10, -20), S(0, 0),  S(10, 0), S(5, 0),  S(5, 0),  S(10, 0), S(0, 0),  S(-10, -20),
	    S(-10, -20), S(0, 0),  S(0, 0),  S(0, 0),  S(0, 0),  S(0, 0),  S(0, 0),  S(-10, -20),
	    S(-15, -25), S(-6, 0), S(0, 0),  S(0, 0),  S(0, 0),  S(0, 0),  S(-8, 0), S(-15, -25),
    },
    .bishop_pcsqt = {
	    S(0, 0), S(0, 0),  S(0, 0),  S(0, 0),  S(0, 0),  S(0, 0),  S(0, 0),  S(0, 0),
	    S(0, 0), S(0, 5),  S(0, 5),  S(0, 5),  S(0, 5),  S(0, 5),  S(0, 5),  S(0, 0),
	    S(0, 0), S(0, 5),  S(0, 9),  S(0, 9),  S(0, 9),  S(0, 9),  S(0, 5),  S(0, 0),
	    S(0, 0), S(18, 5), S(0, 9),  S(0, 9),  S(0, 9),  S(0, 9),  S(18, 5), S(0, 0),
	    S(0, 0), S(0, 5),  S(20, 9), S(20, 9), S(20, 9), S(20, 9), S(0, 5),  S(0, 0),
	    S(5, 0), S(0, 5),  S(7, 9),  S(10, 9), S(10, 9), S(7, 9),  S(0, 5),  S(5, 0),
	    S(0, 0), S(10, 5), S(0, 5),  S(7, 5),  S(7, 5),  S(0, 5),  S(10, 5), S(0, 0),
	    S(0, 0), S(0, 0),  S(-6, 0), S(0, 0),  S(0, 0),  S(-8, 0), S(0, 0),  S(0, 0),
    },
    .rook_pcsqt = {
	    S(7, 12), S(4, 7), S(7, 1), S(-16, -4), S(-1, 7), S(3, 9), S(-6, -2), S(7, 4),
	    S(5, -6), S(6, -16), S(8, -3), S(-8, 8), S(11, -16), S(5, 9), S(-3, -1), S(2, -4),
	    S(-6, 4), S(2, -2), S(-6, -1), S(2, 11), S(7, 4), S(1, 1), S(13, 6), S(-16, -9),
	    S(-3, -2), S(-9, 6), S(0, 3), S(-14, -4), S(1, 4), S(8, 5), S(2, -6), S(-3, -1),
	    S(-5, -5), S(-4, -2), S(-8, -10), S(-1, -11), S(7, -17), S(-6, 4), S(-7, 2), S(-8, -1),
	    S(-2,  5), S(-2, -3), S(-7, 2), S(4, 13), S(1, 0), S(1, 5), S(-1, -2), S(15, -8),
	    S(-10, -2), S(-5, 4), S(-2, -6), S(-10, 2), S(1, 1), S(4, 0), S(5, -5), S(12, -9),
	    S(7, -2), S(8, 1), S(-1, 4), S(1, 5), S(6, -1), S(4, -1), S(-10, 2), S(-4, -3),
    },
    .queen_pcsqt = {
	    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
	    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
	    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
	    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
	    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
	    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
	    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
	    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    },
    .king_pcsqt = {
	    S(-10, -50), S(-10, -20), S(-10, -20), S(-10, -20), S(-10, -20), S(-10, -20), S(-10, -20), S(-10, -50),
	    S(-10, -10), S(-10, 0),   S(-10, 0),   S(-10, 0),   S(-10, 0),   S(-10, 0),   S(-10, 0),   S(-10, -10),
	    S(-10, -10), S(-10, 0),   S(-10, 0),   S(-10, 0),   S(-10, 0),   S(-10, 0),   S(-10, 0),   S(-10, -10),
	    S(-10, -10), S(-10, 0),   S(-10, 0),   S(-10, 0),   S(-10, 0),   S(-10, 0),   S(-10, 0),   S(-10, -10),
	    S(-10, -10), S(-10, 0),   S(-10, 0),   S(-10, 0),   S(-10, 0),   S(-10, 0),   S(-10, 0),   S(-10, -10),
	    S(-10, -10), S(-10, 0),   S(-10, 0),   S(-10, 0),   S(-10, 0),   S(-10, 0),   S(-10, 0),   S(-10, -10),
	    S(-10, -10), S(-10, 0),   S(-10, 0),   S(-10, 0),   S(-10, 0),   S(-10, 0),   S(-10, 0),   S(-10, -10),
	    S(10, -50),  S(20, -20),  S(20, -20),  S(-10, -20), S(0, -20),   S(-10, -20), S(20, -20),  S(20, -50),
    },
    /* clang-format on */

    .pawn_backward = S(-7, -16),
    .pawn_blocked = {S(-19, -9), S(-8, 3)},
    .pawn_doubled = S(-10, -50),
    .pawn_connected = {S(0, 0), S(85, 86), S(54, 54), S(15, 14), S(5, 7),
                    S(8, 7), S(5, 2), S(0, 0)},
    .pawn_isolated = S(-1, -16),
    .pawn_passed = {S(0, 0), S(114, 93), S(53, 65), S(22, 32), S(6, 18),
		    S(5, 8), S(1, 5), S(0, 0)},
    .pawn_center = {S(-1, 1), S(13, -1), S(20, 0), S(27, -1), S(22, 2),
		    S(18, -2)},

    .knight_adj = {S(-9, -11), S(-8, -8), S(-5, -7), S(-4, -4), S(-1, -3),
		    S(2, 0), S(0, 1), S(3, 4), S(10, 6)},
    .knight_defended_by_pawn = S(1, 1),
    .knight_outpost = S(54, 31),
    .knight_mobility = {S(0, 0), S(1, 1), S(2, 2), S(3, 3), S(4, 4), S(5, 5),
                    S(6, 6), S(7, 7), S(8, 8)},

    .bishop_pair = S(20, 41),
    .bishop_rammed_pawns = S(-2, -1),
    .bishop_mobility = {S(0, 0), S(2, 1), S(4, 2), S(6, 3), S(8, 4), S(10, 5),
                    S(12, 6), S(14, 7), S(16, 8), S(18, 9), S(20, 10),
                    S(22, 11), S(24, 12), S(26, 13)},

    .rook_connected = S(8, 15),
    .rook_adj = {S(15, 14), S(13, 10), S(9, 9), S(8, 5), S(2, 4), S(0, -1),
		    S(-4, -4), S(-7, -6), S(-9, -9)},
    .rook_open_file = S(20, 23),
    .rook_semiopen_file = S(11, 24),
    .rook_7th = S(8, 17),
    .rook_mobility = {S(0, 0), S(1, 0), S(2, 0), S(3, 0), S(4, 0), S(5, 0),
                    S(6, 0), S(7, 0), S(8, 0), S(9, 0), S(10, 0), S(11, 0),
                    S(12, 0), S(13, 0), S(14, 0)},
};
static struct eval_params *ep = &eval_params;

#ifdef TUNE
struct eval_trace eval_trace;
#define TRACE_RESET()                                                          \
	eval_trace = (struct eval_trace) { 0 }
#define TRACE_INC(field, color)          eval_trace.field[color]++
#define TRACE_INC_VAL(field, color, val) eval_trace.field[color] += val
#define TRACE_SET(field, val)            eval_trace.field = val
#else
#define TRACE_RESET()
#define TRACE_INC(field, color)
#define TRACE_PHASE(val)
#endif

static int eval_pawns(const struct position *position, const enum color side);
static int eval_knights(const struct position *position, const enum color side);
static int eval_bishops(const struct position *position, const enum color side);
static int eval_rooks(const struct position *position, const enum color side);
static int eval_queens(const struct position *position, const enum color side);
static int eval_king(const struct position *position, const enum color side);

static u64 mask_rank[8];
static u64 mask_file[8];
static u64 mask_adj_file[8];
static u64 mask_passed[64];
static u64 mask_center_pawn;

static int pawn_cnt[2];

int eval_pawns(const struct position *pos, const enum color side)
{
	int eval = 0;
	enum square sq;
	u64 mask;
	u64 blocked, lever, lever_push, neighbours, phalanx, stoppers, support;
	u64 allied_pawns = pos->color[side] & pos->piece[PAWN];
	u64 enemy_pawns = pos->color[!side] & pos->piece[PAWN];

	if (side == BLACK) {
		allied_pawns = __builtin_bswap64(allied_pawns);
		enemy_pawns = __builtin_bswap64(enemy_pawns);
	}

	/* center */
	mask = allied_pawns & mask_center_pawn;
	eval += ep->pawn_center[BB_POPCOUNT(mask)];
	TRACE_INC(pawn_center[BB_POPCOUNT(mask)], side);

	for (mask = allied_pawns; mask;) {
		sq = bb_poplsb(&mask);
		TRACE_INC(piece_value[PAWN], side);

		blocked = enemy_pawns & BB_FROM_SQUARE(sq + NORTH);
		lever = enemy_pawns & bb_pawn_attacks(side, sq);
		lever_push = enemy_pawns & bb_pawn_attacks(side, sq + NORTH);
		neighbours = allied_pawns & mask_adj_file[SQ_FILE(sq)];
		phalanx = neighbours & mask_rank[SQ_RANK(sq)];
		stoppers = enemy_pawns & mask_passed[sq];
		support = neighbours & mask_rank[SQ_RANK(sq + SOUTH)];

		/* piece square table */
		eval += ep->pawn_pcsqt[sq];
		TRACE_INC(pawn_pcsqt[sq], side);

		/* backward */
		if (!(neighbours & (~BB_RANK_8 << 8 * SQ_RANK(sq)) &&
		      (blocked | lever_push))) {
			eval += ep->pawn_backward;
			TRACE_INC(pawn_backward, side);
		}

		/* blocked */
		if (blocked && sq <= SQ_H5) {
			eval += ep->pawn_blocked[SQ_RANK(sq) - 2];
			TRACE_INC(pawn_blocked[SQ_RANK(sq) - 2], side);
		}

		/* doubled */
		if (allied_pawns & BB_FROM_SQUARE(sq + SOUTH)) {
			eval += ep->pawn_doubled;
			TRACE_INC(pawn_doubled, side);
		}

		/* connected */
		if (phalanx | support) {
			eval += ep->pawn_connected[SQ_RANK(sq)];
			TRACE_INC(pawn_connected[SQ_RANK(sq)], side);
		}

		/* isolated */
		else if (!neighbours) {
			eval += ep->pawn_isolated;
			TRACE_INC(pawn_isolated, side);
		}

		/* passed */
		if (!(lever ^ stoppers) ||
		    (!(stoppers ^ lever_push) &&
		     BB_POPCOUNT(phalanx) >= BB_POPCOUNT(lever_push))) {
			eval += ep->pawn_passed[SQ_RANK(sq)];
			TRACE_INC(pawn_passed[SQ_RANK(sq)], side);
		}
	}

	return eval;
}

static int eval_knights(const struct position *pos, const enum color side)
{
	int eval = 0;
	enum square sq;
	u64 mask = pos->color[side] & pos->piece[KNIGHT];
	u64 allies = pos->color[side];
	u64 allied_pawns = pos->color[side] & pos->piece[PAWN];
	u64 enemy_pawns = pos->color[!side] & pos->piece[PAWN];

	if (side == BLACK) {
		mask = __builtin_bswap64(mask);
		allies = __builtin_bswap64(allies);
		allied_pawns = __builtin_bswap64(allied_pawns);
		enemy_pawns = __builtin_bswap64(enemy_pawns);
	}

	for (; mask;) {
		sq = bb_poplsb(&mask);
		TRACE_INC(piece_value[KNIGHT], side);

		/* piece square table */
		eval += ep->knight_pcsqt[sq];
		TRACE_INC(knight_pcsqt[sq], side);

		/* decrease value as allied pawns disappear */
		eval += ep->knight_adj[pawn_cnt[side]];
		TRACE_INC(knight_adj[pawn_cnt[side]], side);

		if (bb_pawn_attacks(BLACK, sq) & allied_pawns) {
			/* knight defended by a pawn */
			eval += ep->knight_defended_by_pawn;
			TRACE_INC(knight_defended_by_pawn, side);
			/* outposts */
			if (((SQ_A6 <= sq && sq <= SQ_H5) ||
			     (SQ_C4 <= sq && sq <= SQ_F4)) &&
			    !(enemy_pawns & mask_passed[sq] &
			      mask_adj_file[SQ_FILE(sq)])) {
				eval += ep->knight_outpost;
				TRACE_INC(knight_outpost, side);
			}
		}

		/* mobility */
		eval += ep->knight_mobility[BB_POPCOUNT(
		    bb_attacks(KNIGHT, sq, 0) &
		    ~(allies | bb_shift(enemy_pawns, SOUTH_EAST) |
		      bb_shift(enemy_pawns, SOUTH_WEST)))];
		TRACE_INC(knight_mobility[BB_POPCOUNT(
			      bb_attacks(KNIGHT, sq, 0) &
			      ~(allies | bb_shift(enemy_pawns, SOUTH_EAST) |
				bb_shift(enemy_pawns, SOUTH_WEST)))],
			  side);

		/* TODO: knight trapped on A8/H8/A7/H7 or A1/H1/A2/H2 */
		/* TODO: penalty for an undefended minor piece */
	}

	return eval;
}

static int eval_bishops(const struct position *pos, const enum color side)
{
	int eval = 0;

	enum square sq;
	(void)sq;
	u64 mask = pos->color[side] & pos->piece[BISHOP];
	u64 allies = pos->color[side];
	u64 allied_pawns = pos->color[side] & pos->piece[PAWN];
	u64 enemy_pawns = pos->color[!side] & pos->piece[PAWN];
	u64 occ = pos->piece[ALL_PIECES];

	if (side == BLACK) {
		mask = __builtin_bswap64(mask);
		allies = __builtin_bswap64(allies);
		allied_pawns = __builtin_bswap64(allied_pawns);
		enemy_pawns = __builtin_bswap64(enemy_pawns);
		occ = __builtin_bswap64(occ);
	}

	/* bishop pair */
	if (BB_SEVERAL(mask)) {
		eval += ep->bishop_pair;
		TRACE_INC(bishop_pair, side);
	}

	for (; mask;) {
		sq = bb_poplsb(&mask);
		TRACE_INC(piece_value[BISHOP], side);

		/* piece square table */
		eval += ep->bishop_pcsqt[sq];
		TRACE_INC(bishop_pcsqt[sq], side);

		/* bad bishop */
		if (sq % 2 == 0) {
			eval += ep->bishop_rammed_pawns *
				BB_POPCOUNT(allied_pawns & BB_WHITE_SQUARES);
			TRACE_INC_VAL(
			    bishop_rammed_pawns, side,
			    BB_POPCOUNT(allied_pawns & BB_WHITE_SQUARES));
		} else {
			eval += ep->bishop_rammed_pawns *
				BB_POPCOUNT(allied_pawns & BB_BLACK_SQUARES);
			TRACE_INC_VAL(
			    bishop_rammed_pawns, side,
			    BB_POPCOUNT(allied_pawns & BB_BLACK_SQUARES));
		}

		/* mobility */
		eval += ep->bishop_mobility[BB_POPCOUNT(
		    bb_attacks(BISHOP, sq, occ) & ~allies)];
		TRACE_INC(bishop_mobility[BB_POPCOUNT(
			      bb_attacks(BISHOP, sq, occ) & ~allies)],
			  side);

		/* TODO: bishop vs knight */
		/* TODO: color weakness */
		/* TODO: bishop trapped on A2/H2/A7/H7 or on A3/H3/A6/H6 */
		/* TODO: fianchetto */
		/* TODO: returning bishop */
		/* TODO: penalty for an undefended minor piece */
	}

	return eval;
}

static int eval_rooks(const struct position *pos, const enum color side)
{
	int eval = 0;
	enum square sq;
	u64 allies = pos->color[side];
	u64 mask = allies & pos->piece[ROOK];

	/* connected rooks */
	if (BB_SEVERAL(mask)) {
		sq = bb_poplsb(&mask);
		if (bb_attacks(ROOK, sq, pos->piece[ALL_PIECES]) & mask) {
			eval += ep->rook_connected;
			TRACE_INC(rook_connected, side);
		}
	}
	mask = allies & pos->piece[ROOK];

	for (; mask;) {
		sq = bb_poplsb(&mask);
		TRACE_INC(piece_value[ROOK], side);

		eval += side == WHITE ? ep->rook_pcsqt[sq]
				      : ep->rook_pcsqt[SQ_FLIP(sq)];
		TRACE_INC(rook_pcsqt[side == WHITE ? sq : SQ_FLIP(sq)], side);

		/* increasing value as pawns disappear */
		eval += ep->rook_adj[pawn_cnt[side]];
		TRACE_INC(rook_adj[pawn_cnt[side]], side);

		/* (semi) open file */
		if (!(mask_file[SQ_FILE(sq)] & pos->piece[PAWN])) {
			eval += ep->rook_open_file;
			TRACE_INC(rook_open_file, side);
		} else if (!(mask_file[SQ_FILE(sq)] & pos->piece[PAWN] &
			     allies)) {
			eval += ep->rook_semiopen_file;
			TRACE_INC(rook_semiopen_file, side);
		}

		/* rook on 7th */
		if ((side == WHITE && 8 - SQ_RANK(sq) >= 7) ||
		    (side == BLACK && 8 - SQ_RANK(sq) <= 2)) {
			eval += ep->rook_7th;
			TRACE_INC(rook_7th, side);
		}

		/* mobility */
		eval += ep->rook_mobility[BB_POPCOUNT(
		    bb_attacks(ROOK, sq, pos->piece[ALL_PIECES]))];
		TRACE_INC(rook_mobility[BB_POPCOUNT(
			      bb_attacks(ROOK, sq, pos->piece[ALL_PIECES]))],
			  side);

		/* TODO: Tarrasch rule */
		/* TODO: penalty for being blocked by king that can't castle */
	}

	return eval;
}

static int eval_queens(const struct position *pos, const enum color side)
{
	int eval = 0;
	enum square sq;
	(void)sq;
	u64 mask = pos->color[side] & pos->piece[QUEEN];

	if (side == BLACK) {
		mask = __builtin_bswap64(mask);
	}

	while (mask) {
		sq = bb_poplsb(&mask);
		TRACE_INC(piece_value[QUEEN], side);

		eval += ep->queen_pcsqt[sq];
		TRACE_INC(queen_pcsqt[sq], side);
	}

	return eval;
}

static int eval_king(const struct position *pos, const enum color side)
{
	int eval = 0;
	enum square ksq;
	u64 mask = pos->color[side] & pos->piece[KING];

	ksq = BB_TO_SQUARE(mask);
	TRACE_INC(piece_value[KING], side);
	if (side == BLACK)
		ksq = SQ_FLIP(ksq);

	eval += ep->king_pcsqt[ksq];
	TRACE_INC(king_pcsqt[ksq], side);

	return eval;
}

int evaluate(const struct position *pos)
{
	int score, phase;
	enum piece_type pt;
	int eval = 0;

	TRACE_RESET();

	pawn_cnt[WHITE] = BB_POPCOUNT(pos->piece[PAWN] & pos->color[WHITE]);
	pawn_cnt[BLACK] = BB_POPCOUNT(pos->piece[PAWN] & pos->color[BLACK]);

	if (1 || !pht_probe(pos->pawn_key, pos->color[WHITE] & pos->piece[PAWN],
			    pos->color[BLACK] & pos->piece[PAWN], &eval)) {
		eval += eval_pawns(pos, WHITE) - eval_pawns(pos, BLACK);
		pht_store(pos->pawn_key, pos->color[WHITE] & pos->piece[PAWN],
			  pos->color[BLACK] & pos->piece[PAWN], eval);
	}
	eval += eval_knights(pos, WHITE) - eval_knights(pos, BLACK);
	eval += eval_bishops(pos, WHITE) - eval_bishops(pos, BLACK);
	eval += eval_rooks(pos, WHITE) - eval_rooks(pos, BLACK);
	eval += eval_queens(pos, WHITE) - eval_queens(pos, BLACK);
	eval += eval_king(pos, WHITE) - eval_king(pos, BLACK);

	/* material */
	for (pt = PAWN; pt <= KING; pt++) {
		eval +=
		    S(SMG(ep->piece_value[pt]) *
			  (BB_POPCOUNT(pos->color[WHITE] & pos->piece[pt]) -
			   BB_POPCOUNT(pos->color[BLACK] & pos->piece[pt])),
		      SEG(ep->piece_value[pt]) *
			  (BB_POPCOUNT(pos->color[WHITE] & pos->piece[pt]) -
			   BB_POPCOUNT(pos->color[BLACK] & pos->piece[pt])));
	}

	phase = 24;
	phase -= BB_POPCOUNT(pos->piece[PAWN]) * 0;
	phase -= BB_POPCOUNT(pos->piece[KNIGHT]) * 1;
	phase -= BB_POPCOUNT(pos->piece[BISHOP]) * 1;
	phase -= BB_POPCOUNT(pos->piece[ROOK]) * 2;
	phase -= BB_POPCOUNT(pos->piece[QUEEN]) * 4;
	phase = (phase * 256 + 24 / 2) / 24;

	TRACE_SET(eval, eval);
	TRACE_SET(phase, phase);

	score = ((SMG(eval) * (256 - phase)) + (SEG(eval) * phase)) / 256;

	return pos->stm == WHITE ? score : -score;
}

void evaluate_init(void)
{
	enum square sq;
	int f, r;

	for (f = 0; f < 8; f++) {
		for (r = 0; r < 8; r++) {
			BB_SET(mask_file[f], 8 * r + f);
			BB_SET(mask_rank[r], 8 * r + f);
		}
		mask_adj_file[f] =
		    bb_shift(mask_file[f], WEST) | bb_shift(mask_file[f], EAST);
	}

	for (sq = SQ_A8; sq <= SQ_H1; sq++) {
		mask_passed[sq] =
		    (mask_adj_file[SQ_FILE(sq)] | mask_file[SQ_FILE(sq)]) &
		    (BB_FROM_SQUARE(sq) - 1) & ~mask_rank[SQ_RANK(sq)];
	}

	BB_SET(mask_center_pawn, SQ_C4);
	BB_SET(mask_center_pawn, SQ_D4);
	BB_SET(mask_center_pawn, SQ_E4);
	BB_SET(mask_center_pawn, SQ_D5);
	BB_SET(mask_center_pawn, SQ_E5);
}
