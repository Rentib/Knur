#include "movepicker.h"
#include "bitboards.h"
#include "knur.h"
#include "movegen.h"
#include "position.h"
#include "search.h"

static int see(struct position *position, enum move move);
static void score_captures(struct move_picker *move_picker,
			   struct position *position);
static void sort_moves(struct move_picker *move_picker, enum move *begin,
		       enum move *end);

static const int mvv[PIECE_TYPE_NB] = {100, 300, 315, 500, 900, 20000, 0};

/* Static Exchange Evaluation - The Swap Algorithm
 * https://www.chessprogramming.org/SEE_-_The_Swap_Algorithm */
int see(struct position *pos, enum move move)
{
	int gain[32], d = 0;
	enum color stm = pos->stm;
	enum square from = MOVE_FROM(move), to = MOVE_TO(move);
	u64 from_bb = BB_FROM_SQUARE(from);
	u64 diagonal = pos->piece[BISHOP] | pos->piece[QUEEN];
	u64 straight = pos->piece[ROOK] | pos->piece[QUEEN];
	u64 occ = pos->piece[ALL_PIECES];
	u64 attackers = pos_attackers(pos, to), allies;
	enum piece_type lva = PIECE_TYPE(pos->board[from]);

	gain[d] = mvv[PIECE_TYPE(pos->board[to])];
	while (++d) {
		gain[d] = mvv[lva] - gain[d - 1];
		if (MAX(-gain[d - 1], gain[d]) < 0)
			break;

		occ ^= from_bb;
		if (lva == PAWN || lva == BISHOP || lva == QUEEN)
			attackers |= bb_attacks(BISHOP, to, occ) & diagonal;
		if (lva == ROOK || lva == QUEEN)
			attackers |= bb_attacks(ROOK, to, occ) & straight;
		attackers &= occ;

		allies = pos->color[stm ^= 1] & attackers;
		if (!(allies & pos->color[stm]))
			break;
		for (lva = PAWN; lva <= KING; lva++) {
			if ((from_bb = allies & pos->piece[lva]))
				break;
		}
		from_bb &= -from_bb;
	}

	while (--d)
		gain[d - 1] = -MAX(-gain[d - 1], gain[d]);

	return gain[0];
}

void score_captures(struct move_picker *mp, struct position *pos)
{
	enum move *m;
	for (m = mp->moves; m != mp->captures; m++)
		mp->scores[m - mp->moves] = see(pos, *m);
}

void sort_moves(struct move_picker *mp, enum move *begin, enum move *end)
{
	enum move *mit, *m, mtmp;
	int *sit = mp->scores + (begin - mp->moves) + 1, *s, stmp;
	for (mit = begin + 1; mit < end; mit++, sit++) {
		mtmp = *mit;
		stmp = *sit;
		for (m = mit, s = sit; m != begin && s[-1] > stmp; m--, s--) {
			*m = m[-1];
			*s = s[-1];
		}
		*m = mtmp;
		*s = stmp;
	}
}

void mp_init(struct move_picker *mp, struct position *pos, enum move hashmove,
	     struct search_stack *ss)
{
	mp->stage = MP_STAGE_HASH + !pos_is_pseudo_legal(pos, hashmove);
	mp->hashmove = hashmove;
	mp->killer[0] = ss->killer[0];
	mp->killer[1] = ss->killer[1];
}

enum move mp_next(struct move_picker *mp, struct position *pos, bool skip_quiet)
{
	enum move bestmove = MOVE_NONE;

	switch (mp->stage) {
	case MP_STAGE_HASH:
		mp->stage = MP_STAGE_GENERATE_CAPTURES;
		return mp->hashmove;
	case MP_STAGE_GENERATE_CAPTURES:
		mp->captures = mg_generate(MGT_CAPTURES, mp->moves, pos);
		score_captures(mp, pos);
		sort_moves(mp, mp->moves, mp->captures);
		mp->stage = MP_STAGE_GOOD_CAPTURES;
		[[fallthrough]];
	case MP_STAGE_GOOD_CAPTURES:
		while (mp->captures != mp->moves &&
		       mp->scores[mp->captures - mp->moves - 1] >= 0) {
			bestmove = *--mp->captures;
			if (bestmove == mp->hashmove)
				continue;
			return bestmove;
		}
		if (skip_quiet) {
			mp->stage = MP_STAGE_DONE;
			return mp_next(mp, pos, skip_quiet);
		}
		mp->stage = MP_STAGE_KILLER1;
		[[fallthrough]];
	case MP_STAGE_KILLER1:
		mp->stage = MP_STAGE_KILLER2;
		if (!skip_quiet && pos_is_pseudo_legal(pos, mp->killer[0]))
			return mp->killer[0];
		[[fallthrough]];
	case MP_STAGE_KILLER2:
		mp->stage = MP_STAGE_GENERATE_QUIET;
		if (!skip_quiet && pos_is_pseudo_legal(pos, mp->killer[1]))
			return mp->killer[1];
		[[fallthrough]];
	case MP_STAGE_GENERATE_QUIET:
		if (!skip_quiet) {
			mp->quiets = mg_generate(MGT_QUIET, mp->captures, pos);
			mp->stage = MP_STAGE_QUIET;
		}
		[[fallthrough]];
	case MP_STAGE_QUIET:
		while (!skip_quiet && mp->quiets != mp->captures) {
			bestmove = *--mp->quiets;
			if (bestmove == mp->hashmove ||
			    bestmove == mp->killer[0] ||
			    bestmove == mp->killer[1])
				continue;
			return bestmove;
		}
		mp->stage = MP_STAGE_BAD_CAPTURES;
		[[fallthrough]];
	case MP_STAGE_BAD_CAPTURES:
		while (mp->captures != mp->moves) {
			bestmove = *--mp->captures;
			if (bestmove == mp->hashmove)
				continue;
			return bestmove;
		}
		mp->stage = MP_STAGE_DONE;
		[[fallthrough]];
	case MP_STAGE_DONE: [[fallthrough]];
	default:            return MOVE_NONE;
	}
}
