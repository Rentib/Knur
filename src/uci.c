#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "movegen.h"
#include "perft.h"
#include "position.h"
#include "search.h"
#include "uci.h"
#include "util.h"

struct parser {
	const char *cmd;                         /* command */
	void (*func)(struct position *, char *); /* function */
};

/* helper functions */
static void readline(char *input);
enum move parse_move(struct position *position, char *move_str);

/* uci functions */
static void uci(struct position *position, char *fmt);
static void isready(struct position *position, char *fmt);
static void setoption(struct position *position, char *fmt);
static void ucinewgame(struct position *position, char *fmt);
static void position(struct position *position, char *fmt);
static void go(struct position *position, char *fmt);
static void stop(struct position *position, char *fmt);
static void quit(struct position *position, char *fmt);

/* non-uci functions */
static void display(struct position *position, char *fmt);
static void perft_(struct position *position, char *fmt);

static struct parser parser[] = {
    {"uci",        uci       },
    {"isready",    isready   },
    {"setoption",  setoption },
    {"ucinewgame", ucinewgame},
    {"position",   position  },
    {"go",         go        },
    {"stop",       stop      },
    {"quit",       quit      },
    {"d\0",        display   },
    {"perft",      perft_    },
};

static bool running;

void uci([[maybe_unused]] struct position *pos, [[maybe_unused]] char *fmt)
{
	if (search_running())
		return;
	printf("id name Knur\n");
	printf("id author Stanisław Bitner\n");
	/*printf("option name ...");*/
	printf("uciok\n");
}

void isready([[maybe_unused]] struct position *pos, [[maybe_unused]] char *fmt)
{
	if (!search_running())
		printf("readyok\n");
}

void setoption([[maybe_unused]] struct position *pos,
	       [[maybe_unused]] char *fmt)
{
	if (search_running())
		return;
}

void ucinewgame(struct position *pos, [[maybe_unused]] char *fmt)
{
	if (!search_running())
		pos_set_fen(pos, nullptr);
}

void position(struct position *pos, char *fmt)
{
	char *token, *moves;
	enum move m;

	if (search_running())
		return;

	if ((moves = strstr(fmt, "moves")))
		moves[-1] = '\0';

	if ((token = strstr(fmt, "fen"))) {
		pos_set_fen(pos, token + 3);
	} else if ((token = strstr(fmt, "startpos"))) {
		pos_set_fen(pos, nullptr);
	} else {
		printf("Invalid command \'%s\'.\n", fmt);
		return;
	}

	if (moves) {
		token = strtok(moves, " ");
		while ((token = strtok(NULL, " "))) {
			if ((m = parse_move(pos, token)) == MOVE_NONE) {
				printf("Invalid move: \'%s\'.\n", token);
				break;
			}
			pos_do_move(pos, m);
		}
	}
}

void go(struct position *pos, char *fmt)
{
	char *token, *saveptr = nullptr;

	if (search_running())
		return;

	struct search_limits limits = {
	    .time = -1,
	    .inc = 0,
	    .movestogo = 30,
	    .depth = 0,
	    .movetime = -1,
	    .infinite = false,
	};

	for (token = strtok_r(fmt, " ", &saveptr); token;
	     token = strtok_r(nullptr, " ", &saveptr)) {
		if (!strcmp(token, "wtime")) {
			token = strtok_r(nullptr, " ", &saveptr);
			if (pos->stm == WHITE)
				limits.time = atoi(token);
		} else if (!strcmp(token, "btime")) {
			token = strtok_r(nullptr, " ", &saveptr);
			if (pos->stm == BLACK)
				limits.time = atoi(token);
		} else if (!strcmp(token, "winc")) {
			token = strtok_r(nullptr, " ", &saveptr);
			if (pos->stm == WHITE)
				limits.inc = atoi(token);
		} else if (!strcmp(token, "binc")) {
			token = strtok_r(nullptr, " ", &saveptr);
			if (pos->stm == BLACK)
				limits.inc = atoi(token);
		} else if (!strcmp(token, "movestogo")) {
			token = strtok_r(nullptr, " ", &saveptr);
			limits.movestogo = atoi(token);
		} else if (!strcmp(token, "depth")) {
			token = strtok_r(nullptr, " ", &saveptr);
			limits.depth = atoi(token);
		} else if (!strcmp(token, "movetime")) {
			token = strtok_r(nullptr, " ", &saveptr);
			limits.movetime = atoi(token);
		} else if (!strcmp(token, "infinite")) {
			limits.infinite = true;
		}
	}

	search_start(pos, &limits);
}

void stop([[maybe_unused]] struct position *pos, [[maybe_unused]] char *fmt)
{
	search_stop();
}

void quit(struct position *pos, char *fmt)
{
	stop(pos, fmt);
	running = false;
}

void display(struct position *pos, [[maybe_unused]] char *fmt)
{
	if (!search_running())
		pos_print(pos);
}

void perft_(struct position *pos, char *fmt)
{
	if (!search_running())
		perft(pos, atoi(fmt + strlen("perft")));
}

void uci_loop(void)
{
	char cmd[1024] = {0};
	struct position position, *pos = &position;
	size_t i, len;
	struct parser *p;

	setbuf(stdin, nullptr);
	setbuf(stdout, nullptr);

	pos_init(pos);

	for (running = true; running;) {
		readline(cmd);
		if (!*cmd)
			break;
		for (i = 0, p = parser; i < ARRAY_SIZE(parser); i++, p++) {
			len = strlen(p->cmd);
			if ((isspace(cmd[len]) || !cmd[len]) &&
			    (!strncmp(cmd, p->cmd, len))) {
				p->func(pos, cmd);
				break;
			}
		}
		if (i == ARRAY_SIZE(parser))
			printf("Unknown command: \'%s\'.\n", cmd);
	}

	search_stop();
	pos_destroy(pos);
}

void readline(char *input)
{
	int c;
	while (isspace(c = getchar())) {}
	for (; c != '\n' && c != EOF; c = getchar())
		*input++ = c;
	*input = '\0';
}

enum move parse_move(struct position *pos, char *move_str)
{
	enum move move_list[256], *last, *m;
	last = mg_generate(MGT_ALL, move_list, pos);
	for (m = move_list; m != last; m++) {
		if (!strcmp(MOVE_STR(*m), move_str) && pos_is_legal(pos, *m))
			return *m;
	}
	return MOVE_NONE;
}
