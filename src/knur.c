#include <stdio.h>

#include "bitboards.h"
#include "evaluate.h"
#include "position.h"
#include "transposition.h"
#include "uci.h"

int main(void)
{
	printf("Knur " VERSION " by Stanislaw Bitner\n");
	bb_init();
	evaluate_init();
	pos_init();
	tt_init(TT_DEFAULT_SIZE);
	pht_init(4);

	uci_loop();

	tt_free();
	bb_free();
	return 0;
}
