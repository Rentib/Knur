#include <math.h>
#include <mm_malloc.h>
#include <stdio.h>
#include <stdlib.h>

#include "bitboards.h"
#include "evaluate.h"
#include "knur.h"
#include "position.h"
#include "transposition.h"
#include "util.h"

#define S(mg, eg)  ((int)((unsigned)(eg) << 16) + (mg))
#define SMG(score) ((int16_t)((uint16_t)((unsigned)((score) + 0x0000) >> 00)))
#define SEG(score) ((int16_t)((uint16_t)((unsigned)((score) + 0x8000) >> 16)))

#define N (4219832)

struct entry {
	struct eval_trace trace;
	double result;
	double static_eval;
};

void print_params(void)
{
#define PARAM(obj, field)                                                      \
	do {                                                                   \
		printf(".%s = S(%d, %d),\n", #field, SMG(obj.field),           \
		       SEG(obj.field));                                        \
	} while (0)

#define PARAM_ARRAY(obj, field)                                                \
	do {                                                                   \
		printf(".%s = {\n", #field);                                   \
		for (unsigned i = 0; i < ARRAY_SIZE(obj.field); i++) {         \
			printf("\tS(%d, %d)\n", SMG(obj.field[i]),             \
			       SEG(obj.field[i]));                             \
		}                                                              \
		printf("},\n");                                                \
	} while (0)

	PARAM_ARRAY(eval_params, piece_value);

	PARAM_ARRAY(eval_params, pawn_pcsqt);
	PARAM_ARRAY(eval_params, knight_pcsqt);
	PARAM_ARRAY(eval_params, bishop_pcsqt);
	PARAM_ARRAY(eval_params, rook_pcsqt);
	PARAM_ARRAY(eval_params, queen_pcsqt);
	PARAM_ARRAY(eval_params, king_pcsqt);

	PARAM(eval_params, pawn_backward);
	PARAM_ARRAY(eval_params, pawn_blocked);
	PARAM(eval_params, pawn_doubled);
	PARAM_ARRAY(eval_params, pawn_connected);
	PARAM(eval_params, pawn_isolated);
	PARAM_ARRAY(eval_params, pawn_passed);
	PARAM_ARRAY(eval_params, pawn_center);

	PARAM_ARRAY(eval_params, knight_adj);
	PARAM(eval_params, knight_defended_by_pawn);
	PARAM(eval_params, knight_outpost);
	PARAM_ARRAY(eval_params, knight_mobility);

	PARAM(eval_params, bishop_pair);
	PARAM(eval_params, bishop_rammed_pawns);
	PARAM_ARRAY(eval_params, bishop_mobility);

	PARAM(eval_params, rook_connected);
	PARAM_ARRAY(eval_params, rook_adj);
	PARAM(eval_params, rook_open_file);
	PARAM(eval_params, rook_semiopen_file);
	PARAM(eval_params, rook_7th);
	PARAM_ARRAY(eval_params, rook_mobility);
}

static void read_data(const char *filename, struct entry *entries)
{
	char buf[1024], *p;
	unsigned i;
	char *fen, *R_str;
	FILE *fp;
	struct position position, *pos = &position;

	fp = fopen(filename, "r");
	for (i = 0; i < N; i++) {
		fen = buf;
		for (p = buf; (*p = fgetc(fp)) && *p != '\n'; p++) {
			if (*p == '|') {
				*p = '\0';
				R_str = p + 1;
			}
		}
		*p = '\0';

		pos_set_fen(pos, fen);
		entries[i].result = atof(R_str);
		entries[i].static_eval = evaluate(pos);
		entries[i].trace = eval_trace;
	}
	fclose(fp);
}

INLINE double sigmoid(double K, double s)
{
	return 1.0 / (1.0 + exp(-K * s / 400.0));
}

INLINE double get_error(struct entry *entries, double K)
{
	double E = 0;
	unsigned i;

#pragma omp parallel shared(total)
	{
#pragma omp for schedule(static, N / 64) reduction(+ : E)
		for (i = 0; i < N; i++)
			E += pow(entries[i].result -
				     sigmoid(K, entries[i].static_eval),
				 2);
	}
	return E / (double)N;
}

double get_best_K(struct entry *entries)
{
	const double eps = 1e-6;
	double low = -10, high = 10, mid;
	double E1, E2;
	while (low + eps <= high) {
		mid = (low + high) / 2;
		E1 = get_error(entries, mid);
		E2 = get_error(entries, mid + eps);
		if (E1 < E2) {
			high = mid;
		} else {
			low = mid + eps;
		}
	}

	return low;
}

#if 0
int trace_eval(struct eval_trace *trace)
{
	int eval = 0;
	struct eval_params *ep = &eval_params;
#define TRACE_SCORE_VAL(field)                                                 \
	do {                                                                   \
		eval += S((trace->field[WHITE] - trace->field[BLACK]) *        \
			      SMG(ep->field),                                  \
			  (trace->field[WHITE] - trace->field[BLACK]) *        \
			      SEG(ep->field));                                 \
	} while (0)
#define TRACE_SCORE_ARR(field)                                                 \
	do {                                                                   \
		for (unsigned __i = 0; __i < ARRAY_SIZE(ep->field); __i++)     \
			TRACE_SCORE_VAL(field[__i]);                           \
	} while (0)

	TRACE_SCORE_ARR(piece_value);
	TRACE_SCORE_ARR(pawn_pcsqt);
	TRACE_SCORE_ARR(knight_pcsqt);
	TRACE_SCORE_ARR(bishop_pcsqt);
	TRACE_SCORE_ARR(rook_pcsqt);
	TRACE_SCORE_ARR(queen_pcsqt);
	TRACE_SCORE_ARR(king_pcsqt);
	TRACE_SCORE_VAL(pawn_backward);
	TRACE_SCORE_ARR(pawn_blocked);
	TRACE_SCORE_VAL(pawn_doubled);
	TRACE_SCORE_ARR(pawn_connected);
	TRACE_SCORE_VAL(pawn_isolated);
	TRACE_SCORE_ARR(pawn_passed);
	TRACE_SCORE_ARR(pawn_center);
	TRACE_SCORE_ARR(knight_adj);
	TRACE_SCORE_VAL(knight_defended_by_pawn);
	TRACE_SCORE_VAL(knight_outpost);
	TRACE_SCORE_ARR(knight_mobility);
	TRACE_SCORE_VAL(bishop_pair);
	TRACE_SCORE_VAL(bishop_rammed_pawns);
	TRACE_SCORE_ARR(bishop_mobility);
	TRACE_SCORE_VAL(rook_connected);
	TRACE_SCORE_ARR(rook_adj);
	TRACE_SCORE_VAL(rook_open_file);
	TRACE_SCORE_VAL(rook_semiopen_file);
	TRACE_SCORE_VAL(rook_7th);
	TRACE_SCORE_ARR(rook_mobility);
	return eval;
}
#endif

void optimize(struct entry *entries, double K)
{
#define UPDATE_VAL(field)                                                      \
	do {                                                                   \
		eval_params.field += S(1, 0);                                  \
		for (et = entries; et - entries < N; et++) {                   \
			eval = et->trace.eval;                                 \
			eval += S(1 * (et->trace.field[WHITE] -                \
				       et->trace.field[BLACK]),                \
				  0);                                          \
			phase = et->trace.phase;                               \
			score = ((SMG(eval) * (256 - phase)) +                 \
				 (SEG(eval) * phase)) /                        \
				256;                                           \
			et->static_eval = score;                               \
			et->trace.eval = eval;                                 \
		}                                                              \
		E = get_error(entries, K);                                     \
		if (E < bestE) {                                               \
			bestE = E, improved = true;                            \
		} else {                                                       \
			eval_params.field -= S(2, 0);                          \
			for (et = entries; et - entries < N; et++) {           \
				eval = et->trace.eval;                         \
				eval -= S(2 * (et->trace.field[WHITE] -        \
					       et->trace.field[BLACK]),        \
					  0);                                  \
				phase = et->trace.phase;                       \
				score = ((SMG(eval) * (256 - phase)) +         \
					 (SEG(eval) * phase)) /                \
					256;                                   \
				et->static_eval = score;                       \
				et->trace.eval = eval;                         \
			}                                                      \
			if (E < bestE) {                                       \
				bestE = E, improved = true;                    \
			} else {                                               \
				eval_params.field += S(1, 0);                  \
				for (et = entries; et - entries < N; et++) {   \
					eval = et->trace.eval;                 \
					eval +=                                \
					    S(1 * (et->trace.field[WHITE] -    \
						   et->trace.field[BLACK]),    \
					      0);                              \
					phase = et->trace.phase;               \
					score = ((SMG(eval) * (256 - phase)) + \
						 (SEG(eval) * phase)) /        \
						256;                           \
					et->static_eval = score;               \
					et->trace.eval = eval;                 \
				}                                              \
			}                                                      \
		}                                                              \
		eval_params.field += S(0, 1);                                  \
		for (et = entries; et - entries < N; et++) {                   \
			eval = et->trace.eval;                                 \
			eval += S(0, 1 * (et->trace.field[WHITE] -             \
					  et->trace.field[BLACK]));            \
			phase = et->trace.phase;                               \
			score = ((SMG(eval) * (256 - phase)) +                 \
				 (SEG(eval) * phase)) /                        \
				256;                                           \
			et->static_eval = score;                               \
			et->trace.eval = eval;                                 \
		}                                                              \
		E = get_error(entries, K);                                     \
		if (E < bestE) {                                               \
			bestE = E, improved = true;                            \
		} else {                                                       \
			eval_params.field -= S(0, 2);                          \
			for (et = entries; et - entries < N; et++) {           \
				eval = et->trace.eval;                         \
				eval -= S(0, 2 * (et->trace.field[WHITE] -     \
						  et->trace.field[BLACK]));    \
				phase = et->trace.phase;                       \
				score = ((SMG(eval) * (256 - phase)) +         \
					 (SEG(eval) * phase)) /                \
					256;                                   \
				et->static_eval = score;                       \
				et->trace.eval = eval;                         \
			}                                                      \
			if (E < bestE) {                                       \
				bestE = E, improved = true;                    \
			} else {                                               \
				eval_params.field += S(0, 1);                  \
				for (et = entries; et - entries < N; et++) {   \
					eval = et->trace.eval;                 \
					eval += S(                             \
					    0, 1 * (et->trace.field[WHITE] -   \
						    et->trace.field[BLACK]));  \
					phase = et->trace.phase;               \
					score = ((SMG(eval) * (256 - phase)) + \
						 (SEG(eval) * phase)) /        \
						256;                           \
					et->static_eval = score;               \
					et->trace.eval = eval;                 \
				}                                              \
			}                                                      \
		}                                                              \
	} while (0)
#define UPDATE_ARR(field)                                                      \
	do {                                                                   \
		for (unsigned i = 0; i < ARRAY_SIZE(eval_params.field); i++)   \
			UPDATE_VAL(field[i]);                                  \
	} while (0)

	size_t iteration = 0;
	double bestE = get_error(entries, K), E;
	bool improved = true;
	struct entry *et;
	int eval, phase, score;

	fprintf(stderr, "Initial error: E = %f\n", bestE);
	while (improved) {
		iteration++;
		improved = false;

#if 1
		UPDATE_ARR(piece_value);
#endif
#if 1
		UPDATE_ARR(pawn_pcsqt);
		UPDATE_ARR(knight_pcsqt);
		UPDATE_ARR(bishop_pcsqt);
		UPDATE_ARR(rook_pcsqt);
		UPDATE_ARR(queen_pcsqt);
		UPDATE_ARR(king_pcsqt);
#endif
#if 1
		UPDATE_VAL(pawn_backward);
		UPDATE_ARR(pawn_blocked);
		UPDATE_VAL(pawn_doubled);
		UPDATE_ARR(pawn_connected);
		UPDATE_VAL(pawn_isolated);
		UPDATE_ARR(pawn_passed);
		UPDATE_ARR(pawn_center);
#endif
#if 1
		UPDATE_ARR(knight_adj);
		UPDATE_VAL(knight_defended_by_pawn);
		UPDATE_VAL(knight_outpost);
		UPDATE_ARR(knight_mobility);
#endif
#if 1
		UPDATE_VAL(bishop_pair);
		UPDATE_VAL(bishop_rammed_pawns);
		UPDATE_ARR(bishop_mobility);
#endif
#if 1
		UPDATE_VAL(rook_connected);
		UPDATE_ARR(rook_adj);
		UPDATE_VAL(rook_open_file);
		UPDATE_VAL(rook_semiopen_file);
		UPDATE_VAL(rook_7th);
		UPDATE_ARR(rook_mobility);
#endif

		fprintf(stderr, "Iteration %zu: E = %f\n", iteration, bestE);
	}

	fprintf(stderr, "Optimization finished after %zu iterations\n",
		iteration);
	print_params();
}

int main(int argc, char *argv[])
{
	char *filename;
	struct entry *entries;
	double K;

	if (argc != 2)
		die("usage: tune file");
	filename = argv[1];

	bb_init();
	evaluate_init();
	pos_init();
	tt_init(TT_DEFAULT_SIZE);
	pht_init(2);

	fprintf(stderr, "Allocating memory\n");
	entries = ecalloc(N, sizeof(struct entry));

	fprintf(stderr, "Reading data\n");
	read_data(filename, entries);

	fprintf(stderr, "Getting best K\n");
	K = get_best_K(entries);
	fprintf(stderr, "K = %f\n", K);

	print_params();

	fprintf(stderr, "Optimizing\n");
	optimize(entries, K);

	tt_free();
	pht_free();
	bb_free();
	free(entries);
}
