#ifndef KNUR_EVALUATE_H_
#define KNUR_EVALUATE_H_

#include "position.h"

struct eval_params {
	int piece_value[PIECE_TYPE_NB];

	int pawn_pcsqt[64];
	int knight_pcsqt[64];
	int bishop_pcsqt[64];
	int rook_pcsqt[64];
	int queen_pcsqt[64];
	int king_pcsqt[64];

	int pawn_backward;
	int pawn_blocked[2];
	int pawn_doubled;
	int pawn_connected[8];
	int pawn_isolated;
	int pawn_passed[8];
	int pawn_center[6];

	int knight_adj[9];
	int knight_outpost;

	int bishop_pair;

	int rook_connected;
	int rook_adj[9];
	int rook_open_file;
	int rook_semiopen_file;
	int rook_7th;
};

int evaluate(const struct position *position);
void evaluate_init(void);

extern struct eval_params eval_params;

#endif /* KNUR_EVALUATE_H_ */
