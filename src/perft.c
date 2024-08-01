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
