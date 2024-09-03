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
		S(  48,  123),
		S( 249,  306),
		S( 254,  324),
		S( 369,  545),
		S( 740, 1069),
		S(   0,    0),
	},
	.pawn_pcsqt = {
		S(   0,    0), 	S(   0,    0), 	S(   0,    0), 	S(   0,    0), 	S(   0,    0), 	S(   0,    0), 	S(   0,    0), 	S(   0,    0),
		S(  81,   42), 	S(  95,   43), 	S(  73,    0), 	S(  92,  -21), 	S(  25,  -11), 	S( -21,   35), 	S( -42,   41), 	S( -68,   55),
		S(  55,   88), 	S(  32,   84), 	S(  46,   49), 	S(  43,   27), 	S(  49,   12), 	S(  81,   41), 	S(  81,   47), 	S(  76,   57),
		S( -18,   12), 	S( -16,   -6), 	S( -12,  -22), 	S(   4,   -8), 	S(   7,   -9), 	S(  -9,  -24), 	S(  -7,  -20), 	S(  -4,  -14),
		S( -24,  -12), 	S( -26,  -13), 	S( -12,   -4), 	S(  -1,  -12), 	S(   0,   -6), 	S(   0,  -31), 	S(  -2,  -30), 	S( -17,  -30),
		S( -33,  -16), 	S( -28,  -20), 	S( -25,  -23), 	S( -21,  -23), 	S( -13,  -24), 	S( -10,  -25), 	S(   5,  -39), 	S( -15,  -34),
		S( -25,   -4), 	S( -21,  -18), 	S( -25,  -21), 	S( -20,  -34), 	S(  -8,  -18), 	S(  13,  -22), 	S(  22,  -31), 	S( -15,  -30),
		S(   0,    0), 	S(   0,    0), 	S(   0,    0), 	S(   0,    0), 	S(   0,    0), 	S(   0,    0), 	S(   0,    0), 	S(   0,    0),
	},
	.knight_pcsqt = {
		S(-197,   48), 	S(-139,   34), 	S( -96,   28), 	S(  19,    5), 	S(   7,  -12), 	S( -20,    9), 	S(  98,   -9), 	S(-176,   -4),
		S(  -3,    4), 	S( -10,   12), 	S(   1,   11), 	S(  46,   -1), 	S(  41,  -10), 	S(  33,  -13), 	S( -30,    6), 	S(  -5,  -14),
		S(  -3,    8), 	S(  11,    4), 	S(  15,   19), 	S(  23,   16), 	S(  71,   -2), 	S(  77,    1), 	S(  65,  -24), 	S(  14,  -16),
		S(   0,   16), 	S(  -1,   16), 	S(  11,   15), 	S(  18,   24), 	S(  10,   11), 	S(  35,    1), 	S(  15,    4), 	S(  35,   -6),
		S(   0,   17), 	S(  -7,   18), 	S(  10,   21), 	S(   5,   20), 	S(  13,   19), 	S(  19,    7), 	S(  24,    6), 	S(   4,   12),
		S( -18,   10), 	S(  -4,   -1), 	S(  -3,   11), 	S(   3,   10), 	S(  13,   11), 	S(   7,   -8), 	S(  17,  -15), 	S(  -3,    0),
		S( -33,   -3), 	S( -12,    6), 	S(  -9,    1), 	S(   0,   -1), 	S(  -1,    0), 	S(  -1,    2), 	S( -21,    3), 	S(   0,   -7),
		S( -22,   -7), 	S( -10,    7), 	S( -33,   10), 	S(  -5,    8), 	S(   0,   -4), 	S(   2,   -9), 	S(  -9,   -1), 	S(-109,   37),
	},
	.bishop_pcsqt = {
		S( -27,   -4), 	S(  -6,   -2), 	S( -65,   14), 	S( -18,    2), 	S( -19,    9), 	S( -49,    4), 	S(  41,    0), 	S(  -1,  -15),
		S( -12,    9), 	S(  -2,    5), 	S(   2,    6), 	S(   8,   11), 	S(  14,   -2), 	S(  19,   12), 	S( -23,    5), 	S( -24,  -10),
		S(   0,   14), 	S(  20,    6), 	S(  15,    4), 	S(  32,   -2), 	S(  37,   -3), 	S(  48,    3), 	S(  34,   12), 	S(  34,   -6),
		S(   4,    3), 	S(  12,   13), 	S(  25,   12), 	S(  31,   19), 	S(  23,   13), 	S(  17,    8), 	S(   8,   10), 	S(  18,  -11),
		S(   0,    0), 	S(  12,    1), 	S(   8,   21), 	S(  28,   12), 	S(  25,    6), 	S(  10,    6), 	S(  20,   -5), 	S(  21,  -10),
		S(   9,    8), 	S(  20,    8), 	S(  11,   12), 	S(  15,   18), 	S(  10,   15), 	S(  16,   -7), 	S(  24,  -12), 	S(  27,  -21),
		S(  10,   10), 	S(  11,   -9), 	S(  15,    0), 	S(   9,    1), 	S(  15,   -1), 	S(  23,   -2), 	S(  24,  -13), 	S(  27,  -25),
		S(   5,  -13), 	S(  23,    1), 	S(   1,    0), 	S(  -7,   -3), 	S(   1,   -5), 	S(   1,   -5), 	S(  23,  -24), 	S(   1,   -3),
	},
	.rook_pcsqt = {
		S(   5,   -9), 	S(  -8,   -2), 	S( -14,    2), 	S(   2,    1), 	S( -13,    2), 	S(   9,    1), 	S(  59,  -19), 	S(   3,   -5),
		S( -22,    6), 	S( -31,   10), 	S(  -6,    8), 	S(   0,    4), 	S(   2,   -2), 	S(  44,  -16), 	S(  31,  -13), 	S(  24,  -14),
		S(  -1,   33), 	S(   3,   25), 	S(  18,   22), 	S(  31,   18), 	S(  52,    3), 	S(  55,    1), 	S(  40,   12), 	S(  56,    7),
		S( -10,   26), 	S(  -7,   31), 	S(   8,   18), 	S(   7,   17), 	S(  10,    7), 	S(  24,    9), 	S(  13,   10), 	S(   8,    3),
		S(  -9,   13), 	S( -19,   20), 	S(  -5,   18), 	S(   7,    9), 	S(   2,    5), 	S(   5,    5), 	S(  24,   -4), 	S( -23,    3),
		S( -26,    6), 	S( -21,    8), 	S( -11,    8), 	S(  -3,   -4), 	S(   4,  -13), 	S(   9,   -6), 	S(  12,  -11), 	S(   0,   -6),
		S( -12,  -17), 	S( -17,   -3), 	S( -10,   -4), 	S(  -6,   -8), 	S(   7,  -20), 	S(   0,  -19), 	S( -18,    2), 	S( -42,   -8),
		S(  -2,    0), 	S(   0,   -1), 	S(  -5,    4), 	S(  -1,    1), 	S(   9,  -10), 	S(  -2,   -1), 	S( -16,    0), 	S(  14,  -12),
	},
	.queen_pcsqt = {
		S(   3,   -6), 	S(  25,   15), 	S(   7,   48), 	S(  40,    7), 	S(  -1,   51), 	S(  49,   30), 	S( 125,  -48), 	S(  55,    0),
		S( -18,   12), 	S( -25,   58), 	S( -15,   64), 	S( -18,   57), 	S(  -2,   69), 	S(  34,   49), 	S(  28,   51), 	S(  32,    5),
		S(  -3,  -13), 	S( -12,   24), 	S( -19,   60), 	S(   5,   49), 	S(  14,   81), 	S(  57,   47), 	S(  47,   45), 	S(  21,   29),
		S(  -9,   12), 	S( -11,   32), 	S(  -8,   37), 	S(  -8,   63), 	S(  -2,   71), 	S(  -2,   82), 	S(  -2,   49), 	S(   1,   34),
		S( -13,    0), 	S( -12,   34), 	S( -11,   47), 	S(  -2,   49), 	S(  -5,   58), 	S(  -8,   55), 	S(  -1,   38), 	S(   0,   18),
		S(  -8,    1), 	S(  -5,    7), 	S(  -7,   30), 	S(  -7,   13), 	S(   4,   12), 	S(   2,   22), 	S(  11,    1), 	S( -14,   -7),
		S( -11,  -29), 	S( -11,    4), 	S(   2,  -25), 	S(   0,  -13), 	S(   2,  -13), 	S(  12,  -49), 	S(   3,  -83), 	S(   0,  -63),
		S( -20,  -10), 	S(  -1,  -41), 	S(  -7,   -9), 	S(  -4,  -11), 	S(   2,  -41), 	S( -25,  -60), 	S(  -7, -126), 	S(  -8,  -50),
	},
	.king_pcsqt = {
		S( 208, -114), 	S( 214,  -68), 	S( 197,  -48), 	S(  69,  -18), 	S( 124,  -19), 	S(  98,  -18), 	S(  16,    2), 	S(  83,  -40),
		S(  59,  -23), 	S(  85,   -4), 	S(  44,   14), 	S(  61,    5), 	S(  29,   19), 	S( -11,   34), 	S( -36,   35), 	S(  48,   10),
		S(  54,   -3), 	S( -17,   23), 	S( -19,   28), 	S(  -7,   25), 	S(   3,   26), 	S(   7,   42), 	S( -44,   48), 	S( -28,   22),
		S(  35,  -11), 	S(  -2,   23), 	S( -24,   18), 	S( -33,   30), 	S( -55,   38), 	S(  -6,   35), 	S( -20,   33), 	S( -74,   14),
		S(  48,  -33), 	S(  10,   -9), 	S( -23,   10), 	S( -36,   19), 	S( -35,   23), 	S( -39,   19), 	S( -41,   13), 	S( -49,    0),
		S(  -2,  -39), 	S( -31,   -7), 	S( -52,   10), 	S( -50,   12), 	S( -40,   11), 	S( -50,    6), 	S( -21,   -6), 	S( -24,  -16),
		S( -10,  -49), 	S( -22,  -20), 	S( -44,    0), 	S( -49,    0), 	S( -52,    2), 	S( -29,   -3), 	S(  25,  -22), 	S(  32,  -37),
		S( -27,  -80), 	S(  23,  -41), 	S(  -9,  -31), 	S( -83,  -16), 	S( -14,  -58), 	S( -33,  -28), 	S(  47,  -54), 	S(  43,  -69),
	},
	.pawn_backward = S(-1, 4),
	.pawn_blocked = {
		S( -55,  -78),
		S( -11,  -11),
	},
	.pawn_doubled = S(-13, -23),
	.pawn_connected = {
		S(   0,    0),
		S( 164,   35),
		S(  63,   33),
		S(  13,   13),
		S(   7,    4),
		S(   8,    4),
		S(  -1,   -2),
		S(   0,    0),
	},
	.pawn_isolated = S(-7, -9),
	.pawn_passed = {
		S(   0,    0),
		S(   1,  137),
		S( -21,   25),
		S(   1,   49),
		S(  -7,   32),
		S(  -3,    8),
		S(   0,    1),
		S(   0,    0),
	},
	.pawn_center = {
		S(  19,   21),
		S(  10,   -1),
		S(   4,  -18),
		S(  14,  -21),
		S( -32,  253),
		S(  18,   -2),
	},
	.knight_adj = {
		S(-140,  -50),
		S(  72,  -68),
		S(  25,  -30),
		S(  -4,   -9),
		S( -16,    0),
		S( -10,    2),
		S(  -7,   19),
		S(  -8,   42),
		S(   1,   22),
	},
	.knight_defended_by_pawn = S(-1, -2),
	.knight_outpost = S(25, 13),
	.knight_protector = S(0, -2),
	.knight_mobility = {
		S( -15, -140),
		S(  -4,  -46),
		S(   0,  -14),
		S(   1,    1),
		S(   5,   13),
		S(   7,   20),
		S(   9,   21),
		S(  15,   17),
		S(  25,    8),
	},
	.bishop_pair = S(7, 68),
	.bishop_rammed_pawns = S(0, 2),
	.bishop_long_diagonal = S(12, 5),
	.bishop_protector = S(0, -1),
	.bishop_mobility = {
		S( -10,  -81),
		S(  -5,  -35),
		S(   0,  -27),
		S(   0,  -16),
		S(   0,   -7),
		S(   0,    2),
		S(   4,    4),
		S(   5,    9),
		S(   6,   10),
		S(   7,   10),
		S(   5,   10),
		S(  13,    1),
		S(  14,    3),
		S(  38,   -8),
	},
	.rook_connected = S(5, 2),
	.rook_adj = {
		S( 363, -184),
		S( -51,  -11),
		S( -76,   11),
		S( -66,   11),
		S( -48,    7),
		S( -39,    6),
		S( -32,    8),
		S( -25,   17),
		S( -27,   37),
	},
	.rook_open_file = S(23, 5),
	.rook_semiopen_file = S(7, 17),
	.rook_7th = S(37, 20),
	.rook_mobility = {
		S(   0,    0),
		S(   1,    0),
		S( -17,  -51),
		S( -10,  -24),
		S( -11,  -11),
		S(  -6,   -4),
		S(  -6,    3),
		S(  -6,    4),
		S(  -4,    5),
		S(  -3,   10),
		S(   0,   14),
		S(  -3,   15),
		S(   2,   14),
		S(   8,   13),
		S(   5,   10),
	},
};
/* clang-format on */

#ifdef TUNE
struct eval_trace eval_trace;
#define TRACE_RESET()                                                          \
	eval_trace = (struct eval_trace) { 0 }
#define TRACE_INC(field, color, cnt) eval_trace.field[color] += cnt
#define TRACE_SET(field, val)        eval_trace.field = val
#else
#define TRACE_RESET()
#define TRACE_INC(field, color, val)
#define TRACE_SET(field, val)
#endif

#define EVAL_INC_CNT(field, color, cnt)                                        \
	do {                                                                   \
		eval += eval_params.field * cnt;                               \
		TRACE_INC(field, color, cnt);                                  \
	} while (0)
#define EVAL_INC(field, color) EVAL_INC_CNT(field, color, 1)

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
static u64 mask_center = (BB_RANK_4 | BB_RANK_5) & (BB_FILE_D | BB_FILE_E);

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
	EVAL_INC(pawn_center[BB_POPCOUNT(mask)], side);

	for (mask = allied_pawns; mask;) {
		sq = bb_poplsb(&mask);
		EVAL_INC(piece_value[PAWN], side);

		blocked = enemy_pawns & BB_FROM_SQUARE(sq + NORTH);
		lever = enemy_pawns & bb_pawn_attacks(side, sq);
		lever_push = enemy_pawns & bb_pawn_attacks(side, sq + NORTH);
		neighbours = allied_pawns & mask_adj_file[SQ_FILE(sq)];
		phalanx = neighbours & mask_rank[SQ_RANK(sq)];
		stoppers = enemy_pawns & mask_passed[sq];
		support = neighbours & mask_rank[SQ_RANK(sq + SOUTH)];

		/* piece square table */
		EVAL_INC(pawn_pcsqt[sq], side);

		/* backward */
		if (!(neighbours & (~BB_RANK_8 << 8 * SQ_RANK(sq)) &&
		      (blocked | lever_push)))
			EVAL_INC(pawn_backward, side);

		/* blocked */
		if (blocked && sq <= SQ_H5)
			EVAL_INC(pawn_blocked[SQ_RANK(sq) - 2], side);

		/* doubled */
		if (allied_pawns & BB_FROM_SQUARE(sq + SOUTH))
			EVAL_INC(pawn_doubled, side);

		/* connected */
		if (phalanx | support)
			EVAL_INC(pawn_connected[SQ_RANK(sq)], side);

		/* isolated */
		else if (!neighbours)
			EVAL_INC(pawn_isolated, side);

		/* passed */
		if (!(lever ^ stoppers) ||
		    (!(stoppers ^ lever_push) &&
		     BB_POPCOUNT(phalanx) >= BB_POPCOUNT(lever_push)))
			EVAL_INC(pawn_passed[SQ_RANK(sq)], side);
	}

	return eval;
}

static int eval_knights(const struct position *pos, const enum color side)
{
	int eval = 0;
	enum square sq;
	enum square ksq = BB_TO_SQUARE(pos->piece[KING] & pos->color[side]);
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
		EVAL_INC(piece_value[KNIGHT], side);

		/* piece square table */
		EVAL_INC(knight_pcsqt[sq], side);

		/* decrease value as allied pawns disappear */
		EVAL_INC(knight_adj[pawn_cnt[side]], side);

		if (bb_pawn_attacks(BLACK, sq) & allied_pawns) {
			/* knight defended by a pawn */
			EVAL_INC(knight_defended_by_pawn, side);
			/* outposts */
			if (((SQ_A6 <= sq && sq <= SQ_H5) ||
			     (SQ_C4 <= sq && sq <= SQ_F4)) &&
			    !(enemy_pawns & mask_passed[sq] &
			      mask_adj_file[SQ_FILE(sq)]))
				EVAL_INC(knight_outpost, side);
		}

		/* knight king protector */
		EVAL_INC_CNT(knight_protector, side, bb_distance(sq, ksq));

		/* mobility */
		EVAL_INC(knight_mobility[BB_POPCOUNT(
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
	enum square ksq = BB_TO_SQUARE(pos->piece[KING] & pos->color[side]);
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
	if (BB_SEVERAL(mask))
		EVAL_INC(bishop_pair, side);

	for (; mask;) {
		sq = bb_poplsb(&mask);
		EVAL_INC(piece_value[BISHOP], side);

		/* piece square table */
		EVAL_INC(bishop_pcsqt[sq], side);

		/* bad bishop */
		if (sq % 2 == 0) {
			EVAL_INC_CNT(
			    bishop_rammed_pawns, side,
			    BB_POPCOUNT(allied_pawns & BB_WHITE_SQUARES));
		} else {
			EVAL_INC_CNT(
			    bishop_rammed_pawns, side,
			    BB_POPCOUNT(allied_pawns & BB_BLACK_SQUARES));
		}

		/* long diagonal bishop */
		if (BB_SEVERAL(bb_attacks(BISHOP, sq, occ) & mask_center))
			EVAL_INC(bishop_long_diagonal, side);

		/* bishop king protector */
		EVAL_INC_CNT(bishop_protector, side, bb_distance(sq, ksq));

		/* mobility */
		EVAL_INC(bishop_mobility[BB_POPCOUNT(
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
		if (bb_attacks(ROOK, sq, pos->piece[ALL_PIECES]) & mask)
			EVAL_INC(rook_connected, side);
	}
	mask = allies & pos->piece[ROOK];

	for (; mask;) {
		sq = bb_poplsb(&mask);
		if (side == BLACK)
			sq = SQ_FLIP(sq);
		EVAL_INC(piece_value[ROOK], side);

		/* piece square table */
		EVAL_INC(rook_pcsqt[sq], side);

		/* increasing value as pawns disappear */
		EVAL_INC(rook_adj[pawn_cnt[side]], side);

		/* (semi) open file */
		if (!(mask_file[SQ_FILE(sq)] & pos->piece[PAWN]))
			EVAL_INC(rook_open_file, side);
		else if (!(mask_file[SQ_FILE(sq)] & pos->piece[PAWN] & allies))
			EVAL_INC(rook_semiopen_file, side);

		/* rook on 7th */
		if (8 - SQ_RANK(sq) >= 7)
			EVAL_INC(rook_7th, side);

		/* mobility */
		EVAL_INC(rook_mobility[BB_POPCOUNT(
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
	u64 mask = pos->color[side] & pos->piece[QUEEN];

	if (side == BLACK) {
		mask = __builtin_bswap64(mask);
	}

	while (mask) {
		sq = bb_poplsb(&mask);
		EVAL_INC(piece_value[QUEEN], side);

		/* piece square table */
		EVAL_INC(queen_pcsqt[sq], side);
	}

	return eval;
}

static int eval_king(const struct position *pos, const enum color side)
{
	int eval = 0;
	enum square ksq;
	u64 mask = pos->color[side] & pos->piece[KING];

	ksq = BB_TO_SQUARE(mask);
	if (side == BLACK)
		ksq = SQ_FLIP(ksq);
	EVAL_INC(piece_value[KING], side);

	/* piece square table */
	EVAL_INC(king_pcsqt[ksq], side);

	return eval;
}

int evaluate(const struct position *pos)
{
	int score, phase;
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
