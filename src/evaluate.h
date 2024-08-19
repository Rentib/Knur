#ifndef KNUR_EVALUATE_H_
#define KNUR_EVALUATE_H_

#include "knur.h"
#include "position.h"

struct eval_params {
	int piece_value[6];

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
	int knight_defended_by_pawn;
	int knight_outpost;
	int knight_mobility[9];

	int bishop_pair;
	int bishop_rammed_pawns;
	int bishop_mobility[14];

	int rook_connected;
	int rook_adj[9];
	int rook_open_file;
	int rook_semiopen_file;
	int rook_7th;
	int rook_mobility[15];
};

#ifdef TUNE
struct eval_trace {
	int piece_value[6][COLOR_NB];

	bool pawn_pcsqt[SQUARE_NB][COLOR_NB];
	bool knight_pcsqt[SQUARE_NB][COLOR_NB];
	bool bishop_pcsqt[SQUARE_NB][COLOR_NB];
	bool rook_pcsqt[SQUARE_NB][COLOR_NB];
	bool queen_pcsqt[SQUARE_NB][COLOR_NB];
	bool king_pcsqt[SQUARE_NB][COLOR_NB];

	int pawn_backward[COLOR_NB];
	int pawn_blocked[2][COLOR_NB];
	int pawn_doubled[COLOR_NB];
	int pawn_connected[8][COLOR_NB];
	int pawn_isolated[COLOR_NB];
	int pawn_passed[8][COLOR_NB];
	int pawn_center[6][COLOR_NB];

	int knight_adj[9][COLOR_NB];
	int knight_defended_by_pawn[COLOR_NB];
	int knight_outpost[COLOR_NB];
	int knight_mobility[9][COLOR_NB];

	int bishop_pair[COLOR_NB];
	int bishop_rammed_pawns[COLOR_NB];
	int bishop_mobility[14][COLOR_NB];

	int rook_connected[COLOR_NB];
	int rook_adj[9][COLOR_NB];
	int rook_open_file[COLOR_NB];
	int rook_semiopen_file[COLOR_NB];
	int rook_7th[COLOR_NB];
	int rook_mobility[15][COLOR_NB];

	int eval;
	int phase;
};
extern struct eval_trace eval_trace;
#endif

int evaluate(const struct position *position);
void evaluate_init(void);

extern struct eval_params eval_params;

#endif /* KNUR_EVALUATE_H_ */
