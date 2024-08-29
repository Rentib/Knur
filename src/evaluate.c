#include "evaluate.h"
#include "bitboards.h"
#include "knur.h"
#include "position.h"
#include "transposition.h"

#define S(mg, eg)  ((int)((unsigned)(eg) << 16) + (mg))
#define SMG(score) ((int16_t)((uint16_t)((unsigned)((score) + 0x0000) >> 00)))
#define SEG(score) ((int16_t)((uint16_t)((unsigned)((score) + 0x8000) >> 16)))

/* clang-format off */
struct eval_params eval_params = {
	.piece_value = {
		S(  48,  119),
		S( 249,  300),
		S( 256,  325),
		S( 364,  544),
		S( 731, 1070),
		S(   0,    0),
	},
	.pawn_pcsqt = {
		S(   0,    0), 	S(   0,    0), 	S(   0,    0), 	S(   0,    0), 	S(   0,    0), 	S(   0,    0), 	S(   0,    0), 	S(   0,    0),
		S(  60,   44), 	S(  82,   31), 	S(  87,    5), 	S( 109,  -19), 	S(  35,    0), 	S( -38,   32), 	S( -29,   46), 	S( -73,   45),
		S(  39,   90), 	S(  15,   82), 	S(  50,   48), 	S(  42,   33), 	S(  55,   19), 	S( 101,   40), 	S(  97,   39), 	S(  80,   57),
		S( -20,   13), 	S( -12,   -3), 	S( -10,  -19), 	S(   5,  -14), 	S(  10,   -7), 	S(  -8,  -25), 	S(  -7,  -24), 	S(  -8,  -18),
		S( -21,  -14), 	S( -26,  -18), 	S( -11,   -4), 	S(  -3,  -10), 	S(  -1,  -12), 	S(  -4,  -31), 	S(  -3,  -29), 	S( -25,  -28),
		S( -28,  -15), 	S( -30,  -20), 	S( -23,  -20), 	S( -26,  -23), 	S( -15,  -29), 	S( -10,  -28), 	S(   5,  -40), 	S( -19,  -38),
		S( -25,  -11), 	S( -24,  -16), 	S( -22,  -16), 	S( -19,  -30), 	S( -10,  -22), 	S(  13,  -21), 	S(  27,  -33), 	S( -19,  -32),
		S(   0,    0), 	S(   0,    0), 	S(   0,    0), 	S(   0,    0), 	S(   0,    0), 	S(   0,    0), 	S(   0,    0), 	S(   0,    0),
	},
	.knight_pcsqt = {
		S(-210,   59), 	S(-151,   30), 	S(-112,   22), 	S(  23,    0), 	S(   0,   -9), 	S( -10,   17), 	S( 119,    5), 	S(-159,   24),
		S(   1,   -2), 	S( -12,    0), 	S(  -5,   16), 	S(  57,    1), 	S(  56,   -5), 	S(  31,  -15), 	S( -44,    2), 	S( -15,  -12),
		S(  -8,    3), 	S(  19,   -5), 	S(  29,   12), 	S(  24,   19), 	S(  58,   -4), 	S(  70,   -2), 	S(  63,  -20), 	S(   7,  -14),
		S(  -6,   11), 	S(   1,    4), 	S(  13,   15), 	S(  19,   31), 	S(  16,    8), 	S(  38,   13), 	S(   6,   12), 	S(  36,  -19),
		S(  -9,   11), 	S( -13,   13), 	S(   8,   17), 	S(   9,   19), 	S(   8,   23), 	S(  15,   18), 	S(  28,    6), 	S(   2,   19),
		S( -21,    1), 	S(   0,   -4), 	S(  -2,    0), 	S(  -1,   11), 	S(  15,    7), 	S(   5,    0), 	S(  11,  -12), 	S(  -5,    5),
		S(  -1,   -8), 	S(  -4,   -9), 	S(  -5,  -10), 	S(   0,    3), 	S(   5,    4), 	S(  -1,   -5), 	S( -21,   11), 	S(   3,  -11),
		S(   9,   14), 	S( -13,   -1), 	S( -28,    3), 	S(  -8,    2), 	S( -18,    1), 	S(   3,  -29), 	S( -10,   14), 	S(-124,   28),
	},
	.bishop_pcsqt = {
		S( -15,   11), 	S(   0,    0), 	S( -72,   28), 	S( -21,    0), 	S( -28,   21), 	S( -53,   -4), 	S(  39,   -6), 	S(  11,  -11),
		S(  -8,    1), 	S(   4,   19), 	S(   5,   -5), 	S(   0,    9), 	S(  21,   -8), 	S(  11,    7), 	S( -15,   16), 	S( -18,  -11),
		S(   0,   10), 	S(  16,    0), 	S(  14,   10), 	S(  29,   -4), 	S(  38,   -7), 	S(  62,    7), 	S(  19,    8), 	S(  29,   -1),
		S(   1,   -1), 	S(   4,   15), 	S(  21,    4), 	S(  33,   19), 	S(  28,    9), 	S(  23,    9), 	S(   5,   10), 	S(  13,  -14),
		S(   4,    0), 	S(  19,   -8), 	S(   8,   15), 	S(  26,   14), 	S(  22,   11), 	S(   7,    7), 	S(  21,  -10), 	S(  10,   -7),
		S(   5,   -3), 	S(  19,    3), 	S(  11,   11), 	S(   8,   13), 	S(  10,   17), 	S(  22,    4), 	S(  15,    1), 	S(  27,  -35),
		S(  15,   14), 	S(  16,   -9), 	S(  19,  -13), 	S(   7,   -3), 	S(  12,   -2), 	S(  10,   -7), 	S(  33,  -11), 	S(  17,  -32),
		S(  26,  -17), 	S(  31,  -17), 	S(   0,   -5), 	S(  -2,    3), 	S(   0,   -2), 	S(   1,   -1), 	S(  36,  -19), 	S(   9,   16),
	},
	.rook_pcsqt = {
		S(  11,  -14), 	S(  -2,   -2), 	S( -14,    3), 	S(  -1,   -3), 	S( -11,    7), 	S(   4,   -2), 	S(  76,  -15), 	S(   0,   -5),
		S( -29,    4), 	S( -32,   12), 	S(  -8,    7), 	S(   2,    6), 	S(   8,   -1), 	S(  52,  -16), 	S(  44,  -15), 	S(  25,  -15),
		S(  -4,   34), 	S(  10,   22), 	S(  29,   21), 	S(  25,   16), 	S(  54,    6), 	S(  57,   13), 	S(  42,   14), 	S(  57,    5),
		S( -14,   30), 	S(  -2,   25), 	S(   2,   21), 	S(   7,   17), 	S(   4,   10), 	S(  21,   11), 	S(  21,    8), 	S(  25,    3),
		S(  -5,   14), 	S( -18,   21), 	S(   3,   11), 	S(   6,    8), 	S(  -6,    7), 	S(   9,    4), 	S(  36,   -1), 	S( -23,   10),
		S( -20,   10), 	S( -12,    5), 	S( -10,    2), 	S(   3,   -5), 	S(   7,   -9), 	S(  17,  -18), 	S(  20,  -11), 	S( -17,   -9),
		S(  -8,   -8), 	S( -21,    3), 	S(  -1,   -8), 	S(  -2,   -5), 	S(   5,  -17), 	S(   5,  -29), 	S( -30,    2), 	S( -45,    0),
		S(  -2,   -1), 	S(  -1,    1), 	S(  -1,    6), 	S(   5,    0), 	S(   8,  -12), 	S(   7,    0), 	S(  -7,   -4), 	S( -18,  -11),
	},
	.queen_pcsqt = {
		S(  12,   -3), 	S(  23,    7), 	S(  -7,   37), 	S(  45,   10), 	S(   0,   54), 	S(  45,   30), 	S( 133,  -61), 	S(  49,  -12),
		S( -13,   20), 	S( -23,   49), 	S( -23,   62), 	S( -17,   61), 	S(  -4,   63), 	S(  30,   51), 	S(  28,   45), 	S(  35,    9),
		S(   0,  -12), 	S(  -8,   28), 	S( -13,   70), 	S(   2,   40), 	S(   7,   86), 	S(  38,   36), 	S(  44,   43), 	S(  18,   21),
		S( -16,   20), 	S(  -8,   41), 	S(   0,   36), 	S(  -6,   59), 	S(  -6,   69), 	S(  -9,   87), 	S(   4,   44), 	S(   5,   26),
		S(  -9,    1), 	S( -17,   34), 	S( -11,   55), 	S(   1,   48), 	S( -11,   65), 	S( -11,   58), 	S(  -4,   37), 	S(  -5,    7),
		S( -21,   -4), 	S(  -7,   13), 	S(  -7,   30), 	S( -10,   14), 	S(  -3,   17), 	S(  -1,   28), 	S(  12,    8), 	S(  -7,    3),
		S(  -8,  -34), 	S( -11,   11), 	S(  -2,  -23), 	S(  -1,  -10), 	S(   3,  -15), 	S(  15,  -57), 	S(  20,  -80), 	S( -16,  -80),
		S(  -9,   -3), 	S(   6,  -46), 	S(  -7,   -2), 	S(  -1,  -16), 	S(   3,  -47), 	S( -21,  -56), 	S( -24, -153), 	S( -15,  -72),
	},
	.king_pcsqt = {
		S( 214, -106), 	S( 220,  -71), 	S( 205,  -48), 	S(  78,  -13), 	S( 150,   -5), 	S( 117,   -9), 	S(  15,   10), 	S(  68,  -35),
		S(  62,  -23), 	S(  90,    6), 	S(  41,   13), 	S(  78,   12), 	S(  44,   25), 	S(  -7,   35), 	S( -17,   49), 	S(  55,   12),
		S(  65,    5), 	S( -21,   37), 	S( -10,   37), 	S(  -7,   27), 	S(  16,   33), 	S(   9,   38), 	S( -50,   47), 	S( -27,   32),
		S(  43,  -11), 	S(  -8,   21), 	S( -21,   26), 	S( -31,   29), 	S( -68,   33), 	S(  -3,   35), 	S( -15,   32), 	S( -68,   20),
		S(  59,  -24), 	S(  28,   -3), 	S( -18,   12), 	S( -31,   22), 	S( -36,   23), 	S( -40,   20), 	S( -32,   12), 	S( -59,   -2),
		S(  13,  -29), 	S( -39,   -3), 	S( -61,    9), 	S( -61,   11), 	S( -35,   10), 	S( -39,    5), 	S( -17,  -11), 	S( -18,  -24),
		S( -28,  -51), 	S( -18,  -18), 	S( -64,   -1), 	S( -61,    0), 	S( -45,   -2), 	S( -31,   -8), 	S(  16,  -25), 	S(  17,  -42),
		S( -28,  -93), 	S(  13,  -48), 	S( -16,  -30), 	S( -82,  -13), 	S(  17,  -66), 	S( -26,  -34), 	S(  25,  -53), 	S(  24,  -69),
	},
	.pawn_backward = S(0, 3),
	.pawn_blocked = {
		S( -58,  -83),
		S( -12,  -12),
	},
	.pawn_doubled = S(-8, -32),
	.pawn_connected = {
		S(   0,    0),
		S( 153,   37),
		S(  59,   26),
		S(  14,   12),
		S(   7,    4),
		S(   8,    6),
		S(  -1,   -2),
		S(   0,    0),
	},
	.pawn_isolated = S(-6, -8),
	.pawn_passed = {
		S(   0,    0),
		S(   2,  138),
		S( -19,   28),
		S(   3,   49),
		S(  -2,   30),
		S(  -3,    8),
		S(   4,    0),
		S(   0,    0),
	},
	.pawn_center = {
		S(  18,   21),
		S(  10,    0),
		S(   8,  -19),
		S(   9,  -33),
		S( -24,  271),
		S(  18,   -2),
	},
	.knight_adj = {
		S(-148,  -53),
		S(  75,  -68),
		S(  27,  -33),
		S(   0,  -12),
		S( -16,   -1),
		S( -12,    0),
		S(  -9,   20),
		S(  -9,   37),
		S(   5,   21),
	},
	.knight_defended_by_pawn = S(-1, -3),
	.knight_outpost = S(20, 12),
	.knight_mobility = {
		S( -12, -155),
		S(  -6,  -46),
		S(   1,  -20),
		S(   3,   -1),
		S(   5,   11),
		S(   7,   20),
		S(   9,   22),
		S(  12,   18),
		S(  25,    6),
	},
	.bishop_pair = S(1, 60),
	.bishop_rammed_pawns = S(0, 0),
	.bishop_mobility = {
		S(  -9,  -71),
		S(  -9,  -45),
		S(  -4,  -33),
		S(  -2,  -21),
		S(   0,   -6),
		S(   4,    2),
		S(   6,    3),
		S(   5,   11),
		S(   8,   14),
		S(   8,   11),
		S(  12,    8),
		S(  13,    5),
		S(  26,   12),
		S(  39,   -6),
	},
	.rook_connected = S(4, 5),
	.rook_adj = {
		S( 347, -181),
		S( -58,  -16),
		S( -73,    6),
		S( -65,   12),
		S( -45,    6),
		S( -37,    9),
		S( -32,   12),
		S( -28,   16),
		S( -36,   37),
	},
	.rook_open_file = S(22, 7),
	.rook_semiopen_file = S(6, 19),
	.rook_7th = S(43, 19),
	.rook_mobility = {
		S(   0,    0),
		S(   1,    0),
		S( -17,  -59),
		S( -10,  -21),
		S( -11,   -9),
		S(  -8,   -2),
		S(  -9,   -1),
		S(  -7,    4),
		S(  -5,    6),
		S(  -3,   11),
		S(  -1,   14),
		S(  -1,   15),
		S(   0,   17),
		S(   8,   13),
		S(   8,    9),
	},
};
/* clang-format on */
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
#define TRACE_INC_VAL(field, color, val)
#define TRACE_SET(field, val)
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

#ifndef TUNE
	if (!pht_probe(pos->pawn_key, pos->color[WHITE] & pos->piece[PAWN],
		       pos->color[BLACK] & pos->piece[PAWN], &eval)) {
#endif
		eval += eval_pawns(pos, WHITE) - eval_pawns(pos, BLACK);
#ifndef TUNE
		pht_store(pos->pawn_key, pos->color[WHITE] & pos->piece[PAWN],
			  pos->color[BLACK] & pos->piece[PAWN], eval);
	}
#endif
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
