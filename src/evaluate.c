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
		S(  44,  119),
		S( 236,  301),
		S( 235,  327),
		S( 335,  550),
		S( 598, 1106),
		S(   0,    0),
	},
	.pawn_pcsqt = {
		S(   0,    0),  S(   0,    0),  S(   0,    0),  S(   0,    0),  S(   0,    0),  S(   0,    0),  S(   0,    0),  S(   0,    0),
		S( 209,   43),  S( 173,   37),  S( 182,   29),  S( 185,  -13),  S( 113,   -7),  S(  95,   18),  S(  41,   67),  S( 126,   36),
		S(  68,   94),  S(  64,   74),  S(  71,   53),  S(  67,   28),  S(  78,   20),  S( 116,   33),  S( 108,   52),  S(  67,   65),
		S( -25,   26),  S( -16,    3),  S( -20,   -6),  S(   0,  -17),  S(  10,   -9),  S(  -4,  -22),  S(   0,  -19),  S( -12,   -9),
		S( -27,   -1),  S( -29,   -7),  S( -12,   -3),  S(  -5,  -10),  S(  -7,  -10),  S(  -3,  -27),  S(  -5,  -24),  S( -23,  -26),
		S( -33,  -10),  S( -31,  -18),  S( -25,  -21),  S( -24,  -28),  S( -19,  -20),  S( -11,  -22),  S(   5,  -33),  S( -15,  -35),
		S( -25,   -4),  S( -22,  -10),  S( -22,  -21),  S( -20,  -17),  S( -14,   -9),  S(  16,  -20),  S(  29,  -31),  S( -15,  -32),
		S(   0,    0),  S(   0,    0),  S(   0,    0),  S(   0,    0),  S(   0,    0),  S(   0,    0),  S(   0,    0),  S(   0,    0),
	},
	.knight_pcsqt = {
		S(-125,  -26),  S( -47,   36),  S( -77,   16),  S(   8,    1),  S(  70,   -6),  S(  17,   -2),  S(  -3,    2),  S( -92,  -33),
		S( -28,   -5),  S( -20,   11),  S(   2,    0),  S(  53,    0),  S(  39,  -11),  S(  38,  -13),  S( -31,  -28),  S( -24,    0),
		S( -40,    9),  S(   7,    2),  S(   4,   20),  S(  23,   17),  S(  66,    2),  S(  82,    7),  S(  23,    2),  S(  24,  -20),
		S( -20,    7),  S(  -6,    8),  S(  12,   11),  S(  23,   20),  S(  12,   24),  S(  33,   11),  S(  12,    1),  S(  30,    9),
		S( -11,    9),  S(  -4,    9),  S(  -1,   26),  S(   4,   14),  S(   8,   27),  S(  18,   19),  S(  11,   14),  S(  -6,   15),
		S( -10,    1),  S( -10,   -3),  S(  -4,    0),  S(   2,   12),  S(   6,   13),  S(   2,   -3),  S(   5,    2),  S( -14,    1),
		S( -40,   21),  S( -23,   -3),  S( -11,  -14),  S(  -3,   -1),  S(   1,    0),  S( -17,   12),  S( -26,   22),  S( -15,   19),
		S( -71,    5),  S( -18,   -7),  S( -11,   -7),  S( -21,    5),  S( -17,  -12),  S(   1,  -12),  S(  -7,    0),  S( -31,   27),
	},
	.bishop_pcsqt = {
		S(  12,    1),  S( -57,   11),  S( -53,   13),  S( -56,   13),  S( -64,    4),  S( -97,   19),  S( -29,   10),  S( -75,   14),
		S( -10,   -6),  S(  -5,    3),  S( -10,   10),  S(  -3,   -5),  S(   1,   -4),  S(  -1,    8),  S( -58,   14),  S( -26,    2),
		S(  -3,    8),  S(   5,   13),  S(   9,    1),  S(  35,    0),  S(  20,    2),  S(  56,   12),  S(  32,   13),  S(  12,   17),
		S(  -7,    4),  S(   3,   11),  S(  18,   11),  S(  40,    7),  S(  17,   11),  S(  18,    5),  S(   1,    2),  S(   3,    0),
		S(   0,   -5),  S(   1,    2),  S(   0,   11),  S(   9,   25),  S(  23,    7),  S(   4,   10),  S(   1,    0),  S(  11,   -6),
		S(   0,    0),  S(  12,    2),  S(   1,   24),  S(   2,    8),  S(   0,   24),  S(  10,   -1),  S(   9,    2),  S(  19,  -27),
		S(  11,   11),  S(   9,   -8),  S(   8,   -8),  S(  -3,    1),  S(   7,   -1),  S(   7,   -6),  S(  25,  -17),  S(  20,  -26),
		S(  -7,    4),  S(   0,   -4),  S(  -1,   -7),  S(  -6,   -5),  S( -19,    6),  S(  -4,  -11),  S( -23,    9),  S( -37,    0),
	},
	.rook_pcsqt = {
		S( -20,    1),  S( -19,    4),  S(   1,    0),  S(  -5,   -3),  S(   6,   -1),  S(  -4,    0),  S(  40,   -7),  S(  45,  -11),
		S( -19,    0),  S( -38,   10),  S( -23,    8),  S(   6,    2),  S(  -6,    1),  S(  47,  -20),  S(   7,   -6),  S(  34,  -14),
		S(  -6,   30),  S(  14,   29),  S(  -2,   31),  S(  15,   21),  S(  43,    5),  S(  60,    4),  S(  76,    2),  S(  10,   24),
		S(  -8,   21),  S(  -8,   23),  S(  -2,   28),  S(   6,   19),  S(  -7,    8),  S(  24,    4),  S(  14,    9),  S( -24,   14),
		S( -29,   17),  S( -20,   19),  S( -22,   15),  S(   0,    8),  S(  -9,    9),  S(   2,  -10),  S(   0,   -1),  S( -21,   -1),
		S( -37,    5),  S( -32,   14),  S( -14,    1),  S(  -9,    7),  S(  -3,   -2),  S(  -2,   -5),  S(   6,   -3),  S( -19,   -9),
		S( -33,    6),  S( -23,   -5),  S( -25,    3),  S( -13,   -7),  S(  -8,  -13),  S(  -8,   -7),  S( -35,   -1),  S( -45,    9),
		S(  -7,   -2),  S(  -8,   -5),  S(  -9,    7),  S(  -5,   -1),  S(   3,   -8),  S(   1,  -11),  S( -27,   -3),  S(  20,  -40),
	},
	.queen_pcsqt = {
		S( -29,   33),  S(   0,   18),  S(  32,   12),  S(  22,   28),  S(  65,   16),  S(  57,   24),  S( 131,  -30),  S(  55,    3),
		S( -24,    9),  S( -18,   31),  S( -20,   62),  S(   0,   55),  S(  -1,   72),  S(  47,   46),  S( -31,   81),  S(  19,   48),
		S( -15,    1),  S( -21,   25),  S(  -3,   40),  S(   3,   49),  S(  19,   70),  S(  51,   55),  S(  58,   21),  S(  15,   24),
		S( -20,    4),  S(  -4,   30),  S(   0,   35),  S(  -1,   61),  S(   0,   64),  S(  -4,   84),  S(   1,   75),  S(  -3,   32),
		S( -10,    0),  S( -12,   21),  S(  -6,   45),  S( -14,   68),  S( -11,   47),  S( -20,   59),  S( -13,   48),  S(  -9,   41),
		S( -17,   -4),  S(  -5,   12),  S(  -6,   36),  S(  -7,   21),  S(   1,   15),  S(  -3,   15),  S(   0,   22),  S(  -2,  -19),
		S(  -7,  -46),  S( -11,    3),  S(  -8,   -1),  S(  -1,  -15),  S(  -3,  -11),  S(  13,  -57),  S(   0,  -57),  S(  -3,  -61),
		S( -19,  -25),  S( -16,    3),  S(  -2,  -42),  S(  -4,  -22),  S(   0,  -26),  S( -12,  -66),  S( -36,  -73),  S( -31,  -47),
	},
	.king_pcsqt = {
		S( 176, -106),  S( 106,  -32),  S( 159,  -40),  S(  32,  -12),  S( -22,    9),  S(  62,   -2),  S(-113,   27),  S(  68,  -53),
		S(  58,  -25),  S(  43,   -2),  S(   4,   22),  S( -24,   28),  S(   9,   21),  S( -49,   32),  S(  -2,   24),  S(  82,   -8),
		S(  59,    1),  S( -19,   27),  S( -30,   31),  S( -54,   35),  S(  15,   38),  S( -34,   49),  S( -55,   50),  S( -22,   18),
		S(   1,    1),  S( -53,   26),  S( -29,   29),  S( -24,   27),  S( -44,   36),  S( -77,   44),  S( -42,   35),  S( -55,   16),
		S(  46,  -41),  S( -26,    0),  S( -70,   19),  S( -23,   21),  S( -37,   26),  S( -48,   23),  S( -40,   17),  S( -88,   -1),
		S(  83,  -58),  S( -17,  -10),  S( -33,    5),  S( -82,   18),  S( -54,   20),  S( -55,   12),  S( -24,   -4),  S( -32,  -20),
		S( -19,  -33),  S(   7,  -14),  S( -40,   -1),  S( -65,    8),  S( -48,    3),  S( -31,   -6),  S(  23,  -25),  S(  38,  -50),
		S( -17,  -49),  S(  29,  -64),  S( -19,  -25),  S( -78,  -19),  S(  -8,  -50),  S( -36,  -31),  S(  45,  -59),  S(  42,  -79),
	},
	.pawn_backward = S(0, 3),
	.pawn_blocked = {
		S( -79,  -78),
		S( -13,  -14),
	},
	.pawn_doubled = S(-4, -25),
	.pawn_connected = {
		S(   0,    0),
		S( 147,   84),
		S(  28,   37),
		S(  15,   14),
		S(   8,    4),
		S(   7,    5),
		S(  -2,   -1),
		S(   0,    0),
	},
	.pawn_isolated = S(-6, -9),
	.pawn_passed = {
		S(   0,    0),
		S(-101,  152),
		S( -42,   37),
		S(  12,   48),
		S(  -3,   31),
		S(   6,    4),
		S(   4,    3),
		S(   0,    0),
	},
	.pawn_center = {
		S(  17,   17),
		S(   9,    1),
		S(   7,  -17),
		S(  11,  -37),
		S(-189,  283),
		S(  18,   -2),
	},
	.knight_adj = {
		S(  14,  -49),
		S(  81,  -61),
		S(   4,  -22),
		S( -18,   -8),
		S( -17,   -1),
		S( -24,    8),
		S( -21,   21),
		S( -15,   29),
		S(  -8,   14),
	},
	.knight_defended_by_pawn = S(-1, -1),
	.knight_outpost = S(22, 13),
	.knight_mobility = {
		S( -17, -105),
		S( -12,  -44),
		S(  -5,  -18),
		S(  -3,   -3),
		S(  -1,   12),
		S(   0,   21),
		S(   2,   21),
		S(   6,   17),
		S(  14,    8),
	},
	.bishop_pair = S(4, 59),
	.bishop_rammed_pawns = S(0, 1),
	.bishop_mobility = {
		S( -20,  -51),
		S( -15,  -38),
		S( -11,  -22),
		S( -12,  -14),
		S(  -8,   -4),
		S(  -5,    5),
		S(   0,    6),
		S(   0,   14),
		S(   0,   17),
		S(   0,   15),
		S(   5,   13),
		S(   5,   11),
		S(   2,   23),
		S(  25,    4),
	},
	.rook_connected = S(1, 9),
	.rook_adj = {
		S( 277, -165),
		S( -53,   -7),
		S( -72,   17),
		S( -60,   16),
		S( -55,   15),
		S( -43,   11),
		S( -34,   10),
		S( -30,   11),
		S( -27,   27),
	},
	.rook_open_file = S(21, 6),
	.rook_semiopen_file = S(7, 20),
	.rook_7th = S(37, 21),
	.rook_mobility = {
		S(   0,    0),
		S(   1,    0),
		S( -19,  -33),
		S( -16,  -18),
		S( -13,  -13),
		S(  -9,  -12),
		S(  -9,   -1),
		S( -11,    6),
		S(  -8,    5),
		S(  -6,    9),
		S(  -2,   11),
		S(  -2,   13),
		S(  -3,   13),
		S(   4,   13),
		S(   7,    6),
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
