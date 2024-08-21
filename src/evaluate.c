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
		S(  39,  110),
		S( 248,  307),
		S( 251,  327),
		S( 363,  554),
		S( 746, 1078),
		S(   0,    0),
	},
	.pawn_pcsqt = {
		S(   0,    0), 	S(   0,    0), 	S(   0,    0), 	S(   0,    0), 	S(   0,    0), 	S(   0,    0), 	S(   0,    0), 	S(   0,    0),
		S( 128,   28), 	S( 191,   32), 	S( 146,   31), 	S( 149,  -12), 	S(  84,   -4), 	S(  86,   16), 	S(  65,   41), 	S( -28,   86),
		S( 100,   39), 	S(  44,   46), 	S(  41,   40), 	S(  60,   -9), 	S(  64,  -13), 	S(  86,    4), 	S(  43,   19), 	S(  53,   19),
		S(  -7,   27), 	S( -21,   17), 	S(  -5,    2), 	S(  -1,   -5), 	S(   2,   -3), 	S(   4,   -2), 	S( -13,    7), 	S(  -6,    7),
		S( -20,    4), 	S( -20,   -3), 	S(  -7,    1), 	S(  -6,  -12), 	S(   0,   -9), 	S(  -7,  -15), 	S(  -7,  -13), 	S( -18,  -11),
		S( -25,   -7), 	S( -26,  -10), 	S( -17,  -21), 	S( -10,  -21), 	S(  -9,  -13), 	S(  -6,  -13), 	S(   4,  -18), 	S(  -8,  -20),
		S( -25,    9), 	S( -17,   -2), 	S( -16,   -5), 	S( -10,   -2), 	S( -11,    0), 	S(  10,   -1), 	S(  20,  -11), 	S(  -9,  -17),
		S(   0,    0), 	S(   0,    0), 	S(   0,    0), 	S(   0,    0), 	S(   0,    0), 	S(   0,    0), 	S(   0,    0), 	S(   0,    0),
	},
	.knight_pcsqt = {
		S(-130,   -3), 	S(  -2,   -1), 	S( -63,   26), 	S(  32,  -40), 	S( -40,   10), 	S( -76,   -1), 	S( -85,   31), 	S( -58,   -1),
		S( -15,    1), 	S( -22,   12), 	S(  28,   -5), 	S(  42,    8), 	S(  21,    2), 	S(   8,   12), 	S(  29,   -9), 	S( -65,   13),
		S( -13,   15), 	S(   5,   14), 	S(   5,   16), 	S(  29,   16), 	S(  69,   -7), 	S(  77,    2), 	S(  19,   -3), 	S(  30,  -27),
		S( -16,   10), 	S( -11,    5), 	S(  15,   21), 	S(  22,   27), 	S(   9,   24), 	S(  22,   -1), 	S(   2,   17), 	S(  36,  -30),
		S(   4,  -10), 	S( -20,   19), 	S(  -4,   29), 	S(  -1,   28), 	S(  12,   29), 	S(  19,    8), 	S(  12,  -10), 	S(  11,   -6),
		S( -23,   -2), 	S(  -9,   -1), 	S(  -5,    4), 	S(  13,    5), 	S(   7,   10), 	S(   3,    1), 	S(  10,    5), 	S(  -8,   -7),
		S( -29,  -43), 	S( -27,   13), 	S(  -6,  -13), 	S(  -1,    3), 	S(  -6,   11), 	S(  -8,   13), 	S(  -5,    7), 	S(   1,    4),
		S( -39,   -7), 	S( -22,  -16), 	S( -29,   -8), 	S( -23,    2), 	S(  -8,   -9), 	S(  -5,  -10), 	S( -20,  -12), 	S(  25,  -16),
	},
	.bishop_pcsqt = {
		S( -65,   28), 	S( -36,   36), 	S( -38,    3), 	S( -59,   13), 	S(   3,  -11), 	S( -55,    0), 	S(  94,   -9), 	S( -44,   -6),
		S( -31,    4), 	S( -15,    4), 	S( -18,    4), 	S( -41,    4), 	S( -13,   -9), 	S( -28,   17), 	S( -30,   -5), 	S( -42,   21),
		S(   1,    2), 	S(  10,    8), 	S(  21,    5), 	S(  12,   -2), 	S(  10,    7), 	S(  48,    2), 	S(  -5,   23), 	S(  27,  -14),
		S(  -9,    0), 	S(  -1,    7), 	S(  -1,    2), 	S(  25,   17), 	S(  15,   15), 	S(  -8,   26), 	S(  -2,   17), 	S(   8,   18),
		S(  -4,   11), 	S(  -7,   -7), 	S(   2,   16), 	S(  11,   15), 	S(  10,   15), 	S(   5,    6), 	S(   5,   -1), 	S(  13,  -30),
		S( -10,    1), 	S(   9,   23), 	S(   7,   -9), 	S(  -1,   10), 	S(   4,    1), 	S(  18,   -2), 	S(  21,  -16), 	S(   1,  -17),
		S(  10,  -17), 	S(   5,  -19), 	S(  11,    3), 	S(   4,  -13), 	S(   7,   -7), 	S(  21,    0), 	S(  23,  -12), 	S(  22,  -30),
		S(   5,  -11), 	S( -12,  -10), 	S(  -7,   -9), 	S(   4,   18), 	S( -16,    4), 	S(  -9,    3), 	S(   9,   -8), 	S(  -2,   -8),
	},
	.rook_pcsqt = {
		S( -22,    2), 	S( -24,    0), 	S(  16,   -2), 	S(  21,  -11), 	S(   8,   -7), 	S( -37,    0), 	S( -25,   17), 	S(  92,  -20),
		S( -38,    7), 	S( -24,   12), 	S( -17,   14), 	S(  15,   -6), 	S( -36,   13), 	S(  61,  -22), 	S(   0,    7), 	S(  30,  -11),
		S( -16,   24), 	S(  -1,   17), 	S(  20,   21), 	S(  -6,   24), 	S(  40,   -2), 	S(   5,   18), 	S( 104,  -10), 	S(  23,    4),
		S( -26,   18), 	S( -19,   24), 	S( -12,   12), 	S(  -9,   12), 	S(   9,   -6), 	S(   1,   -4), 	S(  21,    7), 	S(  -7,   10),
		S( -20,   12), 	S( -61,   31), 	S( -21,   15), 	S( -10,    6), 	S(   2,   -9), 	S( -17,   12), 	S(  31,   -3), 	S( -16,    3),
		S( -28,   16), 	S( -46,   14), 	S( -33,   11), 	S(  -5,    0), 	S( -11,   -7), 	S( -27,   11), 	S(  -1,   -3), 	S(  -5,  -13),
		S( -30,   -4), 	S( -38,    7), 	S( -21,    9), 	S( -15,   -5), 	S(   0,  -24), 	S( -11,   -4), 	S( -11,   -5), 	S( -37,  -14),
		S( -17,   -7), 	S( -13,   -5), 	S( -14,    5), 	S(  -9,   -4), 	S(   0,  -13), 	S(  -5,   -4), 	S(  12,  -20), 	S( -15,   -8),
	},
	.queen_pcsqt = {
		S(  34,  -24), 	S(  10,   34), 	S(  50,   26), 	S(  66,   25), 	S(  64,   26), 	S(  80,   12), 	S( 143,  -36), 	S(  74,   16),
		S( -18,   22), 	S( -42,   72), 	S( -35,   98), 	S( -32,  106), 	S(  -1,   77), 	S(  20,   79), 	S( -21,   64), 	S(  48,   20),
		S( -12,   13), 	S( -10,   38), 	S( -21,   80), 	S(  -7,   75), 	S(   3,   70), 	S(  49,   41), 	S(  36,   64), 	S(  18,   33),
		S( -14,  -12), 	S( -19,   30), 	S( -13,   42), 	S( -17,   87), 	S(   3,   54), 	S(   4,   63), 	S(  15,   75), 	S(   8,   50),
		S( -10,  -18), 	S( -14,   23), 	S( -14,   39), 	S( -11,   45), 	S(  -4,   58), 	S( -12,   45), 	S(   1,   49), 	S(  -1,   39),
		S( -22,  -16), 	S(  -8,    8), 	S(  -5,    8), 	S(  -8,    5), 	S(  12,   -6), 	S(   5,   11), 	S(   6,   10), 	S(  15,  -31),
		S( -25,  -12), 	S(  -9,  -35), 	S(  -2,  -32), 	S(   5,  -20), 	S(  12,  -33), 	S(   3,  -20), 	S(   4,  -38), 	S(  -9,  -80),
		S( -41,    6), 	S( -18,  -30), 	S(  -1,  -24), 	S(  -5,  -22), 	S(  13,  -62), 	S(  15,  -55), 	S( -16,  -73), 	S( -15,  -38),
	},
	.king_pcsqt = {
		S(-260,  -58), 	S( 145,  -28), 	S( -34,   28), 	S(-104,  -23), 	S(  37,    4), 	S( -34,    7), 	S( 161,   10), 	S(-184,  -32),
		S( -28,  -35), 	S( -26,   29), 	S(   8,   20), 	S(  35,  -10), 	S( -30,   23), 	S(   3,   37), 	S( -83,   35), 	S(  16,   -5),
		S(  26,   10), 	S(-108,   45), 	S( -40,   46), 	S( -85,   52), 	S(-110,   49), 	S( -45,   52), 	S(-112,   59), 	S( -12,   -2),
		S(  64,  -19), 	S( -43,   31), 	S(-115,   47), 	S( -90,   52), 	S( -48,   36), 	S( -29,   43), 	S(   9,   24), 	S( -44,   11),
		S( -59,  -29), 	S( -37,   11), 	S( -51,   25), 	S( -33,   23), 	S( -38,   29), 	S( -71,   26), 	S( -47,    8), 	S( -15,  -17),
		S(  46,  -48), 	S( -23,   -5), 	S( -40,    9), 	S( -76,   15), 	S( -31,   16), 	S( -54,    9), 	S(  16,  -18), 	S(  -4,  -31),
		S( -13,   -9), 	S(   8,  -15), 	S( -26,   -3), 	S( -62,    7), 	S( -32,    0), 	S( -12,  -12), 	S(  13,  -26), 	S(  42,  -50),
		S( -50,  -17), 	S(   2,  -29), 	S(  -1,  -36), 	S( -64,  -26), 	S(  11,  -51), 	S( -24,  -29), 	S(  38,  -57), 	S(  34,  -71),
	},
	.pawn_backward = S(0, 1),
	.pawn_blocked = {
		S( -66,  -27),
		S( -11,  -15),
	},
	.pawn_doubled = S(-12, -23),
	.pawn_connected = {
		S(   0,    0),
		S( 249,   54),
		S(  49,   41),
		S(  14,   12),
		S(   9,    8),
		S(   8,    3),
		S(   1,   -5),
		S(   0,    0),
	},
	.pawn_isolated = S(-8, -9),
	.pawn_passed = {
		S(   0,    0),
		S( -48,  144),
		S( -26,   81),
		S(   7,   51),
		S(  -6,   36),
		S(  -7,   16),
		S(   4,    5),
		S(   0,    0),
	},
	.pawn_center = {
		S(  15,   13),
		S(  15,    0),
		S(  14,   -7),
		S(  27,  -24),
		S( -16, -248),
		S(  18,   -2),
	},
	.knight_adj = {
		S(-259,  -36),
		S(  26,  -61),
		S( -13,  -19),
		S( -29,  -13),
		S( -16,   -1),
		S( -18,    7),
		S( -14,   22),
		S( -12,   30),
		S(  -7,   69),
	},
	.knight_defended_by_pawn = S(0, -5),
	.knight_outpost = S(22, 13),
	.knight_mobility = {
		S( -21,  -81),
		S(  -7,  -27),
		S(  -1,   -7),
		S(   7,   -2),
		S(  10,    9),
		S(  10,   14),
		S(  14,   18),
		S(  14,   14),
		S(  15,    5),
	},
	.bishop_pair = S(-2, 57),
	.bishop_rammed_pawns = S(0, 1),
	.bishop_mobility = {
		S(  -7,  -55),
		S(  -4,  -23),
		S(   4,  -28),
		S(   7,  -15),
		S(   7,   -4),
		S(   9,    4),
		S(  14,   12),
		S(  15,   13),
		S(  17,   22),
		S(  17,   19),
		S(  18,   21),
		S(  24,   11),
		S(  25,   12),
		S(  57,   -2),
	},
	.rook_connected = S(-3, 11),
	.rook_adj = {
		S( 139,  -80),
		S( -27,  -24),
		S( -72,   13),
		S( -51,   11),
		S( -51,   11),
		S( -36,   11),
		S( -29,   13),
		S( -22,   18),
		S( -25,   35),
	},
	.rook_open_file = S(26, 9),
	.rook_semiopen_file = S(4, 25),
	.rook_7th = S(31, 21),
	.rook_mobility = {
		S(   0,    0),
		S(   1,    0),
		S( -15,  -65),
		S( -10,  -19),
		S( -13,   -6),
		S( -11,    1),
		S( -11,    5),
		S(  -9,    6),
		S(  -5,    5),
		S(  -2,   10),
		S(   1,   13),
		S(   0,   16),
		S(   5,   12),
		S(   5,   15),
		S(   6,   10),
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
