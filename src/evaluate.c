#include "evaluate.h"
#include "bitboards.h"
#include "knur.h"
#include "position.h"
#include "transposition.h"

enum game_phase { GP_MG, GP_EG, GP_NB };

struct eval_params eval_params = {
    .piece_value = {100, 300, 315, 500, 900, 20000},

    /* clang-format off */
    .pawn_pcsqt = {0},
    .knight_pcsqt = {
	    {-15, -25}, {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {-15, -25},
	    {-10, -20}, {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {-10, -20},
	    {-10, -20}, {0, 0},  {10, 0}, {10, 0}, {10, 0}, {10, 0}, {0, 0},  {-10, -20},
	    {-10, -20}, {5, 0},  {10, 0}, {20, 0}, {20, 0}, {10, 0}, {5, 0},  {-10, -20},
	    {-10, -20}, {5, 0},  {10, 0}, {20, 0}, {20, 0}, {10, 0}, {5, 0},  {-10, -20},
	    {-10, -20}, {0, 0},  {10, 0}, {5, 0},  {5, 0},  {10, 0}, {0, 0},  {-10, -20},
	    {-10, -20}, {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {-10, -20},
	    {-15, -25}, {-6, 0}, {0, 0},  {0, 0},  {0, 0},  {0, 0},  {-8, 0}, {-15, -25},
    },
    .bishop_pcsqt = {
	    {0, 0}, {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},
	    {0, 0}, {0, 5},  {0, 5},  {0, 5},  {0, 5},  {0, 5},  {0, 5},  {0, 0},
	    {0, 0}, {0, 5},  {0, 9},  {0, 9},  {0, 9},  {0, 9},  {0, 5},  {0, 0},
	    {0, 0}, {18, 5}, {0, 9},  {0, 9},  {0, 9},  {0, 9},  {18, 5}, {0, 0},
	    {0, 0}, {0, 5},  {20, 9}, {20, 9}, {20, 9}, {20, 9}, {0, 5},  {0, 0},
	    {5, 0}, {0, 5},  {7, 9},  {10, 9}, {10, 9}, {7, 9},  {0, 5},  {5, 0},
	    {0, 0}, {10, 5}, {0, 5},  {7, 5},  {7, 5},  {0, 5},  {10, 5}, {0, 0},
	    {0, 0}, {0, 0},  {-6, 0}, {0, 0},  {0, 0},  {-8, 0}, {0, 0},  {0, 0},
    },
    .rook_pcsqt = {0},
    .queen_pcsqt = {0},
    .king_pcsqt = {
	    {-10, -50}, {-10, -20}, {-10, -20}, {-10, -20}, {-10, -20}, {-10, -20}, {-10, -20}, {-10, -50},
	    {-10, -10}, {-10, 0},   {-10, 0},   {-10, 0},   {-10, 0},   {-10, 0},   {-10, 0},   {-10, -10},
	    {-10, -10}, {-10, 0},   {-10, 0},   {-10, 0},   {-10, 0},   {-10, 0},   {-10, 0},   {-10, -10},
	    {-10, -10}, {-10, 0},   {-10, 0},   {-10, 0},   {-10, 0},   {-10, 0},   {-10, 0},   {-10, -10},
	    {-10, -10}, {-10, 0},   {-10, 0},   {-10, 0},   {-10, 0},   {-10, 0},   {-10, 0},   {-10, -10},
	    {-10, -10}, {-10, 0},   {-10, 0},   {-10, 0},   {-10, 0},   {-10, 0},   {-10, 0},   {-10, -10},
	    {-10, -10}, {-10, 0},   {-10, 0},   {-10, 0},   {-10, 0},   {-10, 0},   {-10, 0},   {-10, -10},
	    {10, -50},  {20, -20},  {20, -20},  {-10, -20}, {0, -20},   {-10, -20}, {20, -20},  {20, -50},
    },
    /* clang-format on */

    .pawn_backward = {-6, -19},
    .pawn_blocked = {{-19, -8}, {-7, 3}},
    .pawn_doubled = {-11, -51},
    .pawn_connected =
	{{0, 0}, {86, 86}, {54, 54}, {15, 15}, {7, 7}, {7, 7}, {3, 3}, {0, 0}},
    .pawn_isolated = {-1, -20},
    .pawn_passed = {{0, 0},
		    {113, 94},
		    {51, 65},
		    {22, 30},
		    {8, 15},
		    {5, 8},
		    {2, 5},
		    {0, 0}},
    .pawn_center = {{0, 0}, {14, 0}, {22, 0}, {25, 0}, {21, 0}, {17, 0}},

    .knight_adj = {{-10, -10},
		    {-8, -8},
		    {-6, -6},
		    {-4, -4},
		    {-2, -2},
		    {0, 0},
		    {2, 2},
		    {4, 4},
		    {8, 8}},
    .knight_outpost = {54, 31},

    .bishop_pair = {20, 40},

    .rook_connected = {7, 15},
    .rook_adj = {{13, 13},
		    {12, 12},
		    {10, 10},
		    {7, 7},
		    {3, 3},
		    {0, 0},
		    {-3, -3},
		    {-6, -6},
		    {-9, -9}},
    .rook_open_file = {19, 24},
    .rook_semiopen_file = {12, 23},
    .rook_7th = {9, 16},
};
static struct eval_params *ep = &eval_params;

INLINE struct eval eval_add(struct eval lhs, struct eval rhs)
{
	return (struct eval){
	    .mg = lhs.mg + rhs.mg,
	    .eg = lhs.eg + rhs.eg,
	};
}

INLINE struct eval eval_sub(struct eval lhs, struct eval rhs)
{
	return (struct eval){
	    .mg = lhs.mg - rhs.mg,
	    .eg = lhs.eg - rhs.eg,
	};
}

static struct eval eval_pawns(const struct position *position,
			      const enum color side);
static struct eval eval_knights(const struct position *position,
				const enum color side);
static struct eval eval_bishops(const struct position *position,
				const enum color side);
static struct eval eval_rooks(const struct position *position,
			      const enum color side);

static u64 mask_rank[8];
static u64 mask_file[8];
static u64 mask_adj_file[8];
static u64 mask_passed[64];
static u64 mask_center_pawn;

static int pawn_cnt[2];

struct eval eval_pawns(const struct position *pos, const enum color side)
{
	struct eval eval = {0};
	enum square sq;
	u64 mask;
	u64 blocked, lever, lever_push, neighbours, phalanx, stoppers, support;
	u64 allied_pawns = pos->color[side] & pos->piece[PAWN];
	u64 enemy_pawns = pos->color[!side] & pos->piece[PAWN];

	if (side == BLACK) {
		allied_pawns = __builtin_bswap64(allied_pawns);
		enemy_pawns = __builtin_bswap64(enemy_pawns);
	}

	// center
	mask = allied_pawns & mask_center_pawn;
	eval = eval_add(eval, ep->pawn_center[BB_POPCOUNT(mask)]);

	for (mask = allied_pawns; mask;) {
		sq = bb_poplsb(&mask);

		blocked = enemy_pawns & BB_FROM_SQUARE(sq + NORTH);
		lever = enemy_pawns & bb_pawn_attacks(side, sq);
		lever_push = enemy_pawns & bb_pawn_attacks(side, sq + NORTH);
		neighbours = allied_pawns & mask_adj_file[SQ_FILE(sq)];
		phalanx = neighbours & mask_rank[SQ_RANK(sq)];
		stoppers = enemy_pawns & mask_passed[sq];
		support = neighbours & mask_rank[SQ_RANK(sq + SOUTH)];

		// backward
		if (!(neighbours & (~BB_RANK_8 << 8 * SQ_RANK(sq)) &&
		      (blocked | lever_push)))
			eval = eval_add(eval, ep->pawn_backward);

		// blocked
		if (blocked && sq <= SQ_H5)
			eval =
			    eval_add(eval, ep->pawn_blocked[SQ_RANK(sq) - 2]);

		// doubled
		if (allied_pawns & BB_FROM_SQUARE(sq + SOUTH))
			eval = eval_add(eval, ep->pawn_doubled);

		// connected
		if (phalanx | support)
			eval = eval_add(eval, ep->pawn_connected[SQ_RANK(sq)]);

		// isolated
		else if (!neighbours)
			eval = eval_add(eval, ep->pawn_isolated);

		// passed
		if (!(lever ^ stoppers) ||
		    (!(stoppers ^ lever_push) &&
		     BB_POPCOUNT(phalanx) >= BB_POPCOUNT(lever_push)))
			eval = eval_add(eval, ep->pawn_passed[SQ_RANK(sq)]);
	}

	return eval;
}

static struct eval eval_knights(const struct position *pos,
				const enum color side)
{
	struct eval eval = {0};
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

		// piece square table
		eval = eval_add(eval, ep->knight_pcsqt[sq]);

		// decrease value as allied pawns disappear
		eval = eval_add(eval, ep->knight_adj[pawn_cnt[side]]);

		if (bb_pawn_attacks(BLACK, sq) & allied_pawns) {
			// TODO: add to eval_params
			// knight defended by a pawn
			eval.mg += 1;
			eval.eg += 1;
			// outposts
			if (((SQ_A6 <= sq && sq <= SQ_H5) ||
			     (SQ_C4 <= sq && sq <= SQ_F4)) &&
			    !(enemy_pawns & mask_passed[sq] &
			      mask_adj_file[SQ_FILE(sq)]))
				eval = eval_add(eval, ep->knight_outpost);
		}

		// TODO: add to eval_params
		// mobility
		eval.mg +=
		    BB_POPCOUNT(bb_attacks(KNIGHT, sq, 0) &
				~(allies | bb_shift(enemy_pawns, SOUTH_EAST) |
				  bb_shift(enemy_pawns, SOUTH_WEST)));
		eval.eg +=
		    BB_POPCOUNT(bb_attacks(KNIGHT, sq, 0) &
				~(allies | bb_shift(enemy_pawns, SOUTH_EAST) |
				  bb_shift(enemy_pawns, SOUTH_WEST)));

		// TODO: knight trapped on A8/H8/A7/H7 or A1/H1/A2/H2
		// TODO: penalty for an undefended minor piece
	}

	return eval;
}

static struct eval eval_bishops(const struct position *pos,
				const enum color side)
{
	struct eval eval = {0};

	enum square sq;
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

	// bishop pair
	if (BB_SEVERAL(mask))
		eval = eval_add(eval, ep->bishop_pair);

	for (; mask;) {
		sq = bb_poplsb(&mask);

		// piece square table
		eval = eval_add(eval, ep->bishop_pcsqt[sq]);

		// TODO: add to eval_params
		// bad bishop
		if (sq % 2 == 0) {
			eval.mg -= BB_POPCOUNT(allied_pawns & BB_WHITE_SQUARES);
			eval.eg -= BB_POPCOUNT(allied_pawns & BB_WHITE_SQUARES);
		} else {
			eval.mg -= BB_POPCOUNT(allied_pawns & BB_BLACK_SQUARES);
			eval.eg -= BB_POPCOUNT(allied_pawns & BB_BLACK_SQUARES);
		}

		// TODO: add to eval_params
		// mobility
		eval.mg +=
		    BB_POPCOUNT(bb_attacks(BISHOP, sq, ~occ) & ~allies) * 2;
		eval.eg +=
		    BB_POPCOUNT(bb_attacks(BISHOP, sq, ~occ) & ~allies) * 1;

		// TODO: bishop vs knight
		// TODO: color weakness
		// TODO: bishop trapped on A2/H2/A7/H7 or on A3/H3/A6/H6
		// TODO: fianchetto
		// TODO: returnign bishop
		// TODO: penalty for an undefended minor piece
	}

	return eval;
}

static struct eval eval_rooks(const struct position *pos, const enum color side)
{
	struct eval eval = {0};
	enum square sq;
	u64 allies = pos->color[side];
	u64 mask = allies & pos->piece[ROOK];

	// connected rooks
	if (BB_SEVERAL(mask)) {
		sq = bb_poplsb(&mask);
		if (bb_attacks(ROOK, sq, pos->piece[ALL_PIECES]) & mask)
			eval = eval_add(eval, ep->rook_connected);
	}
	mask = allies & pos->piece[ROOK];

	for (; mask;) {
		sq = bb_poplsb(&mask);

		// increasing value as pawns disappear
		eval = eval_add(eval, ep->rook_adj[pawn_cnt[side]]);

		// (semi) open file
		if (!(mask_file[SQ_FILE(sq)] & pos->piece[PAWN]))
			eval = eval_add(eval, ep->rook_open_file);
		else if (!(mask_file[SQ_FILE(sq)] & pos->piece[PAWN] & allies))
			eval = eval_add(eval, ep->rook_semiopen_file);

		// rook on 7th
		if ((side == WHITE && 8 - SQ_RANK(sq) >= 7) ||
		    (side == BLACK && 8 - SQ_RANK(sq) <= 2))
			eval = eval_add(eval, ep->rook_7th);

		// TODO: add to eval_params
		// mobility
		eval.mg +=
		    BB_POPCOUNT(bb_attacks(ROOK, sq, pos->piece[ALL_PIECES]));
		eval.eg += 0;

		// TODO: Tarrasch rule
		// TODO: penalty for being blocked by king that can't castle
	}

	return eval;
}

int evaluate(const struct position *pos)
{
	int score, phase;
	enum piece_type pt;
	struct eval eval = {0};
	enum square ksq[COLOR_NB];

	pawn_cnt[WHITE] = BB_POPCOUNT(pos->piece[PAWN] & pos->color[WHITE]);
	pawn_cnt[BLACK] = BB_POPCOUNT(pos->piece[PAWN] & pos->color[BLACK]);

	if (!pht_probe(pos->pawn_key, pos->color[WHITE] & pos->piece[PAWN],
		       pos->color[BLACK] & pos->piece[PAWN], &eval.mg,
		       &eval.eg)) {
		eval = eval_add(eval, eval_pawns(pos, WHITE));
		eval = eval_sub(eval, eval_pawns(pos, BLACK));
		pht_store(pos->pawn_key, pos->color[WHITE] & pos->piece[PAWN],
			  pos->color[BLACK] & pos->piece[PAWN], eval.mg,
			  eval.eg);
	}
	eval = eval_add(eval, eval_knights(pos, WHITE));
	eval = eval_sub(eval, eval_knights(pos, BLACK));
	eval = eval_add(eval, eval_bishops(pos, WHITE));
	eval = eval_sub(eval, eval_bishops(pos, BLACK));
	eval = eval_add(eval, eval_rooks(pos, WHITE));
	eval = eval_sub(eval, eval_rooks(pos, BLACK));

	ksq[WHITE] = BB_TO_SQUARE(pos->color[WHITE] & pos->piece[KING]);
	ksq[BLACK] = BB_TO_SQUARE(pos->color[BLACK] & pos->piece[KING]);
	eval = eval_add(eval, ep->king_pcsqt[ksq[WHITE]]);
	eval = eval_sub(eval, ep->king_pcsqt[SQ_FLIP(ksq[BLACK])]);

	phase = 24;
	phase -= BB_POPCOUNT(pos->piece[PAWN]) * 0;
	phase -= BB_POPCOUNT(pos->piece[KNIGHT]) * 1;
	phase -= BB_POPCOUNT(pos->piece[BISHOP]) * 1;
	phase -= BB_POPCOUNT(pos->piece[ROOK]) * 2;
	phase -= BB_POPCOUNT(pos->piece[QUEEN]) * 4;
	phase = (phase * 256 + 24 / 2) / 24;

	score = ((eval.mg * (256 - phase)) + (eval.eg * phase)) / 256;

	/* material */
	for (pt = PAWN; pt <= KING; pt++) {
		score += ep->piece_value[pt] *
			 (BB_POPCOUNT(pos->color[WHITE] & pos->piece[pt]) -
			  BB_POPCOUNT(pos->color[BLACK] & pos->piece[pt]));
	}

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
