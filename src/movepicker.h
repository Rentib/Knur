#ifndef KNUR_MOVEPICKER_H_
#define KNUR_MOVEPICKER_H_

#include "knur.h"
#include "position.h"
#include "search.h"

enum mp_stage {
	MP_STAGE_HASH,
	MP_STAGE_GENERATE_CAPTURES,
	MP_STAGE_GOOD_CAPTURES,
	MP_STAGE_KILLER1,
	MP_STAGE_KILLER2,
	MP_STAGE_GENERATE_QUIET,
	MP_STAGE_QUIET,
	MP_STAGE_BAD_CAPTURES,
	MP_STAGE_DONE,
};

struct move_picker {
	enum mp_stage stage;
	int scores[256]; /* move scores */
	enum move moves[256];
	enum move *captures, *quiets;
	enum move hashmove;
	enum move killer[2];
};

void mp_init(struct move_picker *mp, struct position *position,
	     enum move hashmove, struct search_stack *search_stack);
enum move mp_next(struct move_picker *mp, struct position *position,
		  bool skip_quiet);

#endif /* KNUR_MOVEPICKER_H_ */
