#include <stdio.h>

#include "bitboards.h"
#include "uci.h"

int main(void)
{
	printf("Knur " VERSION " by Stanislaw Bitner\n");
	bb_init();

	uci_loop();

	bb_free();
	return 0;
}
