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

#include "knur.h"
#include "movegen.h"
#include "perft.h"
#include "position.h"

static size_t perft_helper(struct position *position, int depth);

void perft(struct position *pos, int depth)
{
	enum move move_list[256], *m, *last;
	size_t nodes_searched = 0, nodes;

	last = mg_generate(MGT_ALL, move_list, pos);

	for (m = move_list; m != last; m++) {
		if (!pos_is_legal(pos, *m))
			continue;
		pos_do_move(pos, *m);
		nodes_searched += nodes = perft_helper(pos, depth - 1);
		pos_undo_move(pos, *m);

		printf("%s: %lu\n", MOVE_STR(*m), nodes);
	}

	printf("\nNodes searched: %lu\n\n", nodes_searched);
}

size_t perft_helper(struct position *pos, int depth)
{
	enum move move_list[256], *m, *last;
	size_t nodes = 0;

	if (depth == 0)
		return 1;

	last = mg_generate(MGT_ALL, move_list, pos);

	if (depth == 1) {
		for (m = move_list; m != last; m++)
			nodes += pos_is_legal(pos, *m);
		return nodes;
	}

	for (m = move_list; m != last; m++) {
		if (!pos_is_legal(pos, *m))
			continue;
		pos_do_move(pos, *m);
		nodes += perft_helper(pos, depth - 1);
		pos_undo_move(pos, *m);
	}

	return nodes;
}
