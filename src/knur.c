#include <stdio.h>

#include "bitboards.h"
#include "evaluate.h"
#include "nn/nnue.h"
#include "position.h"
#include "search.h"
#include "transposition.h"
#include "uci.h"

int main(void)
{
	printf("Knur " VERSION " by Stanislaw Bitner\n");
	bb_init();
	evaluate_init();
	nnue_init();
	pos_init();
	tt_init(TT_DEFAULT_SIZE);
#if !USE_NNUE
	pht_init(2);
#endif
	search_init();

	uci_loop();

	tt_free();
	pht_free();
	bb_free();
	return 0;
}
