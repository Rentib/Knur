#ifndef KNUR_EVALUATE_H_
#define KNUR_EVALUATE_H_

#include "position.h"

struct eval {
	int16_t mg;
	int16_t eg;
};

struct eval_params {
	int piece_value[PIECE_TYPE_NB];

	struct eval pawn_pcsqt[64];
	struct eval knight_pcsqt[64];
	struct eval bishop_pcsqt[64];
	struct eval rook_pcsqt[64];
	struct eval queen_pcsqt[64];
	struct eval king_pcsqt[64];

	struct eval pawn_backward;
	struct eval pawn_blocked[2];
	struct eval pawn_doubled;
	struct eval pawn_connected[8];
	struct eval pawn_isolated;
	struct eval pawn_passed[8];
	struct eval pawn_center[6];

	struct eval knight_adj[9];
	struct eval knight_outpost;

	struct eval bishop_pair;

	struct eval rook_connected;
	struct eval rook_adj[9];
	struct eval rook_open_file;
	struct eval rook_semiopen_file;
	struct eval rook_7th;
};

int evaluate(const struct position *position);
void evaluate_init(void);

extern struct eval_params eval_params;

#endif /* KNUR_EVALUATE_H_ */
