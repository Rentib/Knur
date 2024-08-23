#include <math.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bitboards.h"
#include "evaluate.h"
#include "position.h"
#include "transposition.h"
#include "util.h"

#define NPOSITIONS (3160531)
#define NTERMS     (sizeof(struct eval_params) / sizeof(int))
#define NEPOCHS    (1000)
#define KPRECISION (6)

#define S(mg, eg)  ((int)((unsigned)(eg) << 16) + (mg))
#define SMG(score) ((int16_t)((uint16_t)((unsigned)((score) + 0x0000) >> 00)))
#define SEG(score) ((int16_t)((uint16_t)((unsigned)((score) + 0x8000) >> 16)))

enum { MG, EG };

typedef double params_t[NTERMS][2];

struct entry {
	double R;
	double E;
	int8_t w[NTERMS], b[NTERMS];
	double rho_mg, rho_eg;
};

static void init_params(params_t params);
static void print_params(params_t params);
static void read_data(char *filename, struct entry *entries);
static double get_best_K(struct entry *entries);
static void tune(params_t params, struct entry *entries, double K);

INLINE double sigmoid(double x, double K)
{
	return 1.0 / (1.0 + exp(-K * x / 400));
}

INLINE double get_error(struct entry *entries, double K)
{
	double E = 0;
	unsigned i;
#pragma omp parallel shared(E)
	{
#pragma omp for schedule(static, NPOSITIONS / 64) reduction(+ : E)
		for (i = 0; i < NPOSITIONS; i++)
			E += pow(entries[i].R - sigmoid(entries[i].E, K), 2);
	}
	return E / NPOSITIONS;
}

void init_params(params_t params)
{
	unsigned i;
	int *cast = (int *)&eval_params;
	for (i = 0; i < NTERMS; i++, cast++) {
		params[i][MG] = SMG(*cast);
		params[i][EG] = SEG(*cast);
	}
}

void print_params(params_t params)
{
	unsigned i;
	struct eval_params ep;
	int *cast = (int *)&ep;
	for (i = 0; i < NTERMS; i++)
		cast[i] = S((int)params[i][MG], (int)params[i][EG]);

#define print_val(term)                                                        \
	do {                                                                   \
		printf(".%s = S(%d, %d),\n", #term, SMG(ep.term),              \
		       SEG(ep.term));                                          \
	} while (0)
#define print_arr(term)                                                        \
	do {                                                                   \
		printf(".%s = {\n", #term);                                    \
		for (i = 0; i < ARRAY_SIZE(ep.term); i++) {                    \
			printf("\tS(%4d, %4d),%c", SMG(ep.term[i]),            \
			       SEG(ep.term[i]),                                \
			       ARRAY_SIZE(ep.term) == 64 && i % 8 != 7         \
				   ? ' '                                       \
				   : '\n');                                    \
		}                                                              \
		printf("},\n");                                                \
	} while (0)

	print_arr(piece_value);

	print_arr(pawn_pcsqt);
	print_arr(knight_pcsqt);
	print_arr(bishop_pcsqt);
	print_arr(rook_pcsqt);
	print_arr(queen_pcsqt);
	print_arr(king_pcsqt);

	print_val(pawn_backward);
	print_arr(pawn_blocked);
	print_val(pawn_doubled);
	print_arr(pawn_connected);
	print_val(pawn_isolated);
	print_arr(pawn_passed);
	print_arr(pawn_center);

	print_arr(knight_adj);
	print_val(knight_defended_by_pawn);
	print_val(knight_outpost);
	print_arr(knight_mobility);

	print_val(bishop_pair);
	print_val(bishop_rammed_pawns);
	print_arr(bishop_mobility);

	print_val(rook_connected);
	print_arr(rook_adj);
	print_val(rook_open_file);
	print_val(rook_semiopen_file);
	print_val(rook_7th);
	print_arr(rook_mobility);

	fflush(stdout);
}

void read_data(char *filename, struct entry *entries)
{
	unsigned i, j;
	char input[256], *p;
	FILE *fp;
	struct position position, *pos = &position;
	int8_t *cast;

	fprintf(stderr, "Reading data from \'%s\'\n", filename);

	if (!(fp = fopen(filename, "r")))
		die("fopen:");

	for (i = 0; i < NPOSITIONS; i++) {
		if (fgets(input, ARRAY_SIZE(input), fp) == NULL)
			die("tune: not enough data");
		if (!(p = strchr(input, '|')))
			die("tune: invalid data");
		*p++ = '\0';

		pos_set_fen(pos, input);
		entries[i].R = atof(p);
		entries[i].E = evaluate(pos) * (pos->stm == BLACK ? -1 : 1);
		cast = (int8_t *)&eval_trace;
		for (j = 0; j < NTERMS; j++) {
			entries[i].w[j] = cast[j * 2 + WHITE];
			entries[i].b[j] = cast[j * 2 + BLACK];
		}
		entries[i].rho_mg = 1 - eval_trace.phase / 256.0;
		entries[i].rho_eg = 0 + eval_trace.phase / 256.0;
	}

	if (fclose(fp))
		die("fclose:");
}

double get_best_K(struct entry *entries)
{
	const double eps = pow(10, -KPRECISION);
	double low = -10, high = 10, mid;
	double E1, E2;

	fprintf(stderr, "Searching for best K\n");

	while (low + eps <= high) {
		mid = (low + high) / 2;
		E1 = get_error(entries, mid);
		E2 = get_error(entries, mid + eps);
		if (E1 < E2) {
			high = mid;
		} else {
			low = mid;
		}
	}

	fprintf(stderr, "Best K = %.6f\n", low);

	return low;
}

void tune(params_t params, struct entry *entries, double K)
{
	unsigned epoch, i;
	params_t gradient;
	double s, x;
	struct entry *et;
	u64 timestamp;

	fprintf(stderr, "Tuning parameters\n");

	print_params(params);

	for (epoch = 1; epoch <= NEPOCHS; epoch++) {
		timestamp = gettime();
#pragma omp parallel private(i, et, s, x) shared(params, entries, K, gradient)
		{
			for (i = 0; i < NTERMS; i++) {
				gradient[i][MG] = 0;
				gradient[i][EG] = 0;
				for (et = entries; et - entries < NPOSITIONS;
				     et++) {
					s = sigmoid(et->E, K);
					x = (s - et->R) * K * s * (1 - s) *
					    (et->w[i] - et->b[i]);
					gradient[i][MG] += x * et->rho_mg;
					gradient[i][EG] += x * et->rho_eg;
				}
				gradient[i][MG] *= 2.0 / NPOSITIONS;
				gradient[i][EG] *= 2.0 / NPOSITIONS;
			}
		}

#pragma omp parallel for private(i) shared(params, gradient)
		{
			for (i = 0; i < NTERMS; i++) {
				params[i][MG] -= gradient[i][MG];
				params[i][EG] -= gradient[i][EG];
			}
		}

#pragma omp parallel for private(et) shared(entries, params)
		{
			for (et = entries; et - entries < NPOSITIONS; et++) {
				et->E = 0;
				for (i = 0; i < NTERMS; i++) {
					et->E += (et->w[i] - et->b[i]) *
						     params[i][MG] *
						     et->rho_mg +
						 (et->w[i] - et->b[i]) *
						     params[i][EG] * et->rho_eg;
				}
			}
		}

		fprintf(stderr, "Epoch %d: error = %.6f\n", epoch,
			get_error(entries, K));
		fprintf(stderr, "Time: %lu ms\n", gettime() - timestamp);

		if (epoch % 10 == 0)
			print_params(params);
	}

	print_params(params);
}

int main(int argc, char *argv[])
{
	struct entry *entries = ecalloc(NPOSITIONS, sizeof(struct entry));
	params_t params;
	double K;

	bb_init();
	evaluate_init();
	pos_init();
	tt_init(TT_DEFAULT_SIZE);
	pht_init(2);

	if (argc != 2)
		die("usage: tuner data");

	init_params(params);
	read_data(argv[1], entries);
	K = get_best_K(entries);
	tune(params, entries, K);

	pht_free();
	tt_free();
	bb_free();
	free(entries);
	return 0;
}
