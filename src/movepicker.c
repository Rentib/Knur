#include "movepicker.h"
#include "knur.h"
#include "movegen.h"
#include "position.h"

void mp_init(struct move_picker *mp) { mp->stage = MP_STAGE_GENERATE_CAPTURES; }

enum move mp_next(struct move_picker *mp, struct position *pos, bool skip_quiet)
{
	enum move bestmove = MOVE_NONE;

	switch (mp->stage) {
	case MP_STAGE_GENERATE_CAPTURES:
		mp->captures = mg_generate(MGT_CAPTURES, mp->moves, pos);
		mp->stage = MP_STAGE_CAPTURES;
		[[fallthrough]];
	case MP_STAGE_CAPTURES:
		while (mp->captures != mp->moves) {
			bestmove = *--mp->captures;
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
			return bestmove;
		}
		mp->stage = MP_STAGE_DONE;
		[[fallthrough]];
	case MP_STAGE_DONE: [[fallthrough]];
	default:            return MOVE_NONE;
	}
}