#include <stdio.h>

#include "bitboards.h"

int main(void)
{
	printf("Knur " VERSION " by Stanislaw Bitner\n");
	bb_init();

	bb_free();
	return 0;
}
