/*
  Knur, a UCI chess engine.
  Copyright (C) 2024-2026 Stanisław Bitner

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>

#include "bitboards.h"
#include "evaluate.h"
#include "nnue.h"
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
