#include "movepicker.h"
#include "knur.h"
#include "movegen.h"
#include "position.h"

static void score_captures(struct move_picker *move_picker,
			   struct position *position);
static void sort_moves(struct move_picker *move_picker, enum move *begin,
		       enum move *end);

static int mvv[PIECE_TYPE_NB] = {100, 300, 315, 500, 900, 0, 0};

void score_captures(struct move_picker *mp, struct position *pos)
{
	enum move *m;
	for (m = mp->moves; m != mp->captures; m++)
		mp->scores[m - mp->moves] =
		    mvv[PIECE_TYPE(pos->board[MOVE_TO(*m)])];
}

void sort_moves(struct move_picker *mp, enum move *begin, enum move *end)
{
	enum move *mit, *m, mtmp;
	int *sit = mp->scores + (begin - mp->moves) + 1, *s, stmp;
	for (mit = begin + 1; mit < end; mit++, sit++) {
		mtmp = *mit;
		stmp = *sit;
		for (m = mit, s = sit; m != begin && s[-1] > *s; m--, s--) {
			*m = m[-1];
			*s = s[-1];
		}
		*m = mtmp;
		*s = stmp;
	}
}

void mp_init(struct move_picker *mp, struct position *pos, enum move hashmove)
{
	mp->stage = MP_STAGE_HASH + !pos_is_pseudo_legal(pos, hashmove);
	mp->hashmove = hashmove;
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
		mp->stage = MP_STAGE_CAPTURES;
		[[fallthrough]];
	case MP_STAGE_CAPTURES:
		while (mp->captures != mp->moves) {
			bestmove = *--mp->captures;
			if (bestmove == mp->hashmove)
				continue;
			return bestmove;
		}
		if (skip_quiet) {
			mp->stage = MP_STAGE_DONE;
			return mp_next(mp, pos, skip_quiet);
		}
		mp->stage = MP_STAGE_GENERATE_QUIET;
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
