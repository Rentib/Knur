#ifndef KNUR_MOVEPICKER_H_
#define KNUR_MOVEPICKER_H_

#include "knur.h"
#include "position.h"

enum mp_stage {
	MP_STAGE_HASH,
	MP_STAGE_GENERATE_CAPTURES,
	MP_STAGE_CAPTURES,
	MP_STAGE_GENERATE_QUIET,
	MP_STAGE_QUIET,
	MP_STAGE_DONE,
};

struct move_picker {
	enum mp_stage stage;
	int scores[256]; /* move scores */
	enum move moves[256];
	enum move *captures, *quiets;
	enum move hashmove;
};

void mp_init(struct move_picker *mp, struct position *position,
	     enum move hashmove);
enum move mp_next(struct move_picker *mp, struct position *position,
		  bool skip_quiet);

#endif /* KNUR_MOVEPICKER_H_ */
