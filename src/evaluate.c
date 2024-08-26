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
		S(  37,  107),
		S( 245,  308),
		S( 247,  318),
		S( 360,  552),
		S( 745, 1077),
		S(   0,    0),
	},
	.pawn_pcsqt = {
		S(   0,    0), 	S(   0,    0), 	S(   0,    0), 	S(   0,    0), 	S(   0,    0), 	S(   0,    0), 	S(   0,    0), 	S(   0,    0),
		S( 128,   29), 	S( 189,   32), 	S( 146,   31), 	S( 147,  -11), 	S(  84,   -2), 	S(  86,   16), 	S(  63,   40), 	S( -26,   85),
		S(  97,   39), 	S(  42,   44), 	S(  41,   38), 	S(  60,   -7), 	S(  64,  -11), 	S(  86,    4), 	S(  43,   19), 	S(  54,   20),
		S(  -9,   24), 	S( -18,   18), 	S(  -6,    0), 	S(   2,   -3), 	S(   5,    0), 	S(   1,   -3), 	S( -11,    5), 	S(  -4,    7),
		S( -18,    3), 	S( -19,   -2), 	S(  -8,   -1), 	S(  -4,  -10), 	S(  -2,   -6), 	S(  -4,  -11), 	S(  -7,  -13), 	S( -16,  -13),
		S( -24,   -4), 	S( -24,   -8), 	S( -16,  -17), 	S( -13,  -20), 	S(  -8,  -14), 	S(  -7,  -13), 	S(   7,  -17), 	S(  -8,  -20),
		S( -23,    5), 	S( -19,    0), 	S( -17,   -4), 	S( -11,   -2), 	S(  -8,    0), 	S(  14,   -2), 	S(  21,  -13), 	S( -11,  -17),
		S(   0,    0), 	S(   0,    0), 	S(   0,    0), 	S(   0,    0), 	S(   0,    0), 	S(   0,    0), 	S(   0,    0), 	S(   0,    0),
	},
	.knight_pcsqt = {
		S(-128,   -1), 	S(  -2,   -1), 	S( -61,   26), 	S(  32,  -38), 	S( -38,   10), 	S( -74,    0), 	S( -83,   31), 	S( -58,   -1),
		S( -14,    1), 	S( -21,   12), 	S(  26,   -3), 	S(  40,    6), 	S(  21,    2), 	S(   8,   11), 	S(  27,   -7), 	S( -63,   13),
		S( -13,   13), 	S(   3,   12), 	S(   5,   17), 	S(  29,   16), 	S(  69,   -5), 	S(  77,    2), 	S(  19,   -1), 	S(  28,  -25),
		S( -14,   10), 	S(  -9,    5), 	S(  13,   19), 	S(  22,   27), 	S(   9,   23), 	S(  23,    3), 	S(   3,   18), 	S(  36,  -27),
		S(   1,  -10), 	S( -18,   17), 	S(  -3,   27), 	S(  -1,   25), 	S(  10,   27), 	S(  16,    8), 	S(  12,   -8), 	S(   9,   -4),
		S( -24,   -2), 	S(  -9,   -1), 	S(  -7,    2), 	S(  11,    5), 	S(   6,   10), 	S(   1,    0), 	S(   7,    3), 	S(  -8,   -7),
		S( -27,  -41), 	S( -25,   11), 	S(  -7,  -13), 	S(  -1,    1), 	S(  -3,    8), 	S(  -8,   10), 	S(  -5,    5), 	S(   0,    2),
		S( -39,   -7), 	S( -18,  -15), 	S( -29,   -8), 	S( -21,    0), 	S(  -8,   -8), 	S(  -3,  -10), 	S( -17,  -10), 	S(  23,  -16),
	},
	.bishop_pcsqt = {
		S( -63,   26), 	S( -36,   34), 	S( -38,    3), 	S( -59,   11), 	S(   1,   -9), 	S( -53,    0), 	S(  92,   -8), 	S( -42,   -4),
		S( -29,    4), 	S( -13,    2), 	S( -16,    2), 	S( -39,    4), 	S( -11,   -7), 	S( -26,   15), 	S( -28,   -3), 	S( -40,   19),
		S(   1,    0), 	S(  10,    6), 	S(  19,    3), 	S(  12,    0), 	S(  10,    7), 	S(  48,    2), 	S(  -3,   21), 	S(  25,  -12),
		S(  -9,    0), 	S(   0,    6), 	S(   1,    3), 	S(  25,   17), 	S(  14,   14), 	S(  -5,   26), 	S(  -2,   15), 	S(   8,   16),
		S(  -2,    9), 	S(  -5,   -5), 	S(   0,   14), 	S(   9,   13), 	S(  10,   13), 	S(   2,    4), 	S(   3,   -1), 	S(  13,  -28),
		S(  -7,    0), 	S(   6,   20), 	S(   9,   -5), 	S(   0,    9), 	S(   3,    3), 	S(  14,   -4), 	S(  21,  -16), 	S(   1,  -17),
		S(   8,  -17), 	S(   7,  -17), 	S(   9,    1), 	S(   0,  -13), 	S(   6,   -7), 	S(  19,    0), 	S(  20,  -10), 	S(  19,  -30),
		S(   3,  -11), 	S(  -9,   -8), 	S(  -6,   -9), 	S(   1,   14), 	S( -16,    2), 	S(  -7,    0), 	S(   6,   -8), 	S(  -2,   -6),
	},
	.rook_pcsqt = {
		S( -22,    0), 	S( -22,    0), 	S(  14,   -2), 	S(  19,  -11), 	S(   6,   -7), 	S( -36,    0), 	S( -25,   13), 	S(  90,  -22),
		S( -37,    6), 	S( -24,   10), 	S( -15,   13), 	S(  13,   -4), 	S( -33,   13), 	S(  61,  -20), 	S(   0,    7), 	S(  30,   -9),
		S( -14,   24), 	S(   0,   18), 	S(  18,   20), 	S(  -4,   24), 	S(  40,    0), 	S(   5,   19), 	S( 104,   -7), 	S(  23,    5),
		S( -24,   19), 	S( -17,   25), 	S( -10,   13), 	S(  -6,   13), 	S(   9,   -3), 	S(   2,    0), 	S(  21,    7), 	S(  -5,   10),
		S( -20,   12), 	S( -59,   29), 	S( -19,   15), 	S(  -9,    6), 	S(   0,   -6), 	S( -15,   12), 	S(  29,   -2), 	S( -16,    1),
		S( -28,   13), 	S( -46,   12), 	S( -31,    9), 	S(  -6,   -1), 	S( -11,   -7), 	S( -25,    9), 	S(   0,   -3), 	S(  -5,  -13),
		S( -30,   -5), 	S( -37,    4), 	S( -22,    6), 	S( -13,   -5), 	S(   0,  -22), 	S( -11,   -5), 	S( -11,   -5), 	S( -37,  -14),
		S( -17,   -6), 	S( -14,   -6), 	S( -14,    2), 	S(  -6,   -3), 	S(   0,  -13), 	S(  -3,   -3), 	S(   9,  -20), 	S( -15,  -10),
	},
	.queen_pcsqt = {
		S(  32,  -24), 	S(  10,   32), 	S(  48,   24), 	S(  64,   23), 	S(  62,   24), 	S(  80,   12), 	S( 143,  -36), 	S(  74,   14),
		S( -18,   20), 	S( -40,   72), 	S( -33,   96), 	S( -32,  104), 	S(  -1,   75), 	S(  20,   79), 	S( -19,   64), 	S(  48,   20),
		S( -10,   11), 	S( -10,   36), 	S( -19,   78), 	S(  -5,   75), 	S(   3,   70), 	S(  49,   41), 	S(  36,   64), 	S(  17,   33),
		S( -12,  -10), 	S( -17,   30), 	S( -11,   42), 	S( -15,   85), 	S(   3,   54), 	S(   4,   63), 	S(  12,   73), 	S(   6,   48),
		S(  -8,  -16), 	S( -12,   23), 	S( -11,   39), 	S(  -8,   45), 	S(  -2,   58), 	S( -10,   45), 	S(  -1,   47), 	S(  -1,   37),
		S( -20,  -14), 	S(  -8,    6), 	S(  -3,    8), 	S(  -5,    5), 	S(   9,   -6), 	S(   2,    9), 	S(   4,    8), 	S(  13,  -31),
		S( -23,  -12), 	S(  -6,  -33), 	S(   2,  -30), 	S(   2,  -20), 	S(   8,  -33), 	S(   3,  -20), 	S(   2,  -38), 	S(  -9,  -80),
		S( -39,    4), 	S( -16,  -28), 	S(  -1,  -24), 	S(  -1,  -21), 	S(  10,  -62), 	S(  12,  -55), 	S( -16,  -73), 	S( -17,  -38),
	},
	.king_pcsqt = {
		S(-258,  -56), 	S( 145,  -26), 	S( -32,   28), 	S(-102,  -21), 	S(  37,    4), 	S( -32,    8), 	S( 161,   10), 	S(-182,  -30),
		S( -26,  -33), 	S( -24,   29), 	S(   8,   20), 	S(  35,   -7), 	S( -28,   23), 	S(   3,   38), 	S( -81,   36), 	S(  16,   -2),
		S(  26,   10), 	S(-106,   45), 	S( -40,   46), 	S( -85,   49), 	S(-110,   48), 	S( -45,   52), 	S(-110,   61), 	S( -10,    1),
		S(  63,  -17), 	S( -43,   29), 	S(-115,   44), 	S( -90,   48), 	S( -48,   34), 	S( -29,   42), 	S(   9,   27), 	S( -42,   12),
		S( -57,  -27), 	S( -37,    9), 	S( -51,   22), 	S( -33,   19), 	S( -38,   25), 	S( -70,   24), 	S( -45,   10), 	S( -13,  -14),
		S(  44,  -48), 	S( -22,   -5), 	S( -40,    6), 	S( -74,   13), 	S( -31,   11), 	S( -52,    9), 	S(  14,  -17), 	S(  -4,  -29),
		S( -11,   -9), 	S(   6,  -15), 	S( -26,   -4), 	S( -62,    3), 	S( -32,   -1), 	S( -13,  -14), 	S(  16,  -22), 	S(  37,  -49),
		S( -50,  -17), 	S(   0,  -29), 	S(   0,  -34), 	S( -64,  -26), 	S(  14,  -52), 	S( -25,  -30), 	S(  39,  -53), 	S(  33,  -72),
	},
	.pawn_backward = S(-3, 1),
	.pawn_blocked = {
		S( -64,  -25),
		S(  -8,  -14),
	},
	.pawn_doubled = S(-8, -21),
	.pawn_connected = {
		S(   0,    0),
		S( 247,   54),
		S(  47,   39),
		S(  15,   13),
		S(   7,    6),
		S(   7,    4),
		S(   1,   -3),
		S(   0,    0),
	},
	.pawn_isolated = S(-6, -9),
	.pawn_passed = {
		S(   0,    0),
		S( -46,  146),
		S( -24,   81),
		S(   7,   50),
		S(  -7,   33),
		S(  -3,   13),
		S(   4,    1),
		S(   0,    0),
	},
	.pawn_center = {
		S(  14,   10),
		S(  12,    0),
		S(  14,   -5),
		S(  28,  -22),
		S( -14, -246),
		S(  18,   -2),
	},
	.knight_adj = {
		S(-260,  -45),
		S(  24,  -59),
		S( -14,  -17),
		S( -29,   -9),
		S( -19,   -1),
		S( -18,    9),
		S( -12,   23),
		S(  -9,   32),
		S(  -3,   67),
	},
	.knight_defended_by_pawn = S(-1, -1),
	.knight_outpost = S(23, 16),
	.knight_mobility = {
		S( -19,  -79),
		S(  -5,  -27),
		S(   0,   -8),
		S(   3,   -1),
		S(   6,    8),
		S(   9,   15),
		S(  13,   16),
		S(  15,   15),
		S(  15,    6),
	},
	.bishop_pair = S(1, 57),
	.bishop_rammed_pawns = S(0, 6),
	.bishop_mobility = {
		S(  -5,  -55),
		S(  -1,  -23),
		S(   3,  -26),
		S(   4,  -13),
		S(   5,   -2),
		S(  10,    5),
		S(  13,   10),
		S(  12,   10),
		S(  13,   20),
		S(  15,   16),
		S(  16,   18),
		S(  22,    8),
		S(  23,   10),
		S(  55,   -2),
	},
	.rook_connected = S(0, 12),
	.rook_adj = {
		S( 127, -130),
		S( -26,  -18),
		S( -71,   22),
		S( -54,   16),
		S( -51,   18),
		S( -35,   17),
		S( -25,   20),
		S( -14,   24),
		S( -18,   35),
	},
	.rook_open_file = S(21, 5),
	.rook_semiopen_file = S(4, 1),
	.rook_7th = S(30, 16),
	.rook_mobility = {
		S(   0,    0),
		S(   1,    0),
		S( -13,  -63),
		S( -11,  -19),
		S( -11,   -5),
		S(  -9,    0),
		S( -10,    3),
		S(  -7,    6),
		S(  -5,    7),
		S(  -3,   11),
		S(   0,   13),
		S(   0,   14),
		S(   4,   11),
		S(   5,   13),
		S(   4,    6),
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
