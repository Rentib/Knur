#ifndef KNUR_MOVEPICKER_H_
#define KNUR_MOVEPICKER_H_

#include "knur.h"
#include "position.h"

enum mp_stage {
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
};

void mp_init(struct move_picker *mp);
enum move mp_next(struct move_picker *mp, struct position *position);

#endif /* KNUR_MOVEPICKER_H_ */
