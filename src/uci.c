/*
  Knur, a chess engine.
  Copyright (C) 2022 Stanisław Bitner <sbitner420@tutanota.com>

  Knur is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Knur is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "perft.h"
#include "position.h"
#include "search.h"
#include "uci.h"
#include "util.h"

#define STARTPOS "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

/** \typedef Parser
 * Defines structure Parser.
 */
typedef struct Parser Parser;

/** \struct Parser
 * A structure used for parsing user input.
 * When a user input is read we need to check if it is a valid command.
 * If it is then a function is executed.
 */
struct Parser {
  const char *cmd;                              /**< Command; */
  void (*func)(const Position *, const char *); /**< Function. */
};

static void display(const Position *, const char *);
static void quit(const Position *, const char *);

static Parser parser[] = {
  { "d", display },
  { "q", quit },
  { "quit", quit },
};

static void
display(const Position *pos, const char *input)
{
  (void)input;
  pos_print(pos);
}

static void
quit(const Position *pos, const char *input)
{
  (void)pos, (void)input;
  info.quit = 1;
}

void
uci_loop(void)
{
  char input[1024];
  Position pos = { .st = NULL };
  State *st;
  Parser *p;
  size_t i, len;
  pos_set(&pos, STARTPOS);

  for (info.quit = 0; !info.quit; ) {
    readline(input);
    for (i = 0, p = parser; i < LENGTH(parser); i++, p++) {
      len = strlen(p->cmd);
      if ((!input[len] || isspace(input[len]))
      && !strncmp(input, p->cmd, len)) {
        p->func(&pos, input + len);
        break;
      }
    }
    if (i == LENGTH(parser))
      printf("Unknown command: \'%s\'.\n", input);
  }

  while ((st = pos.st)) {
    pos.st = pos.st->prev;
    free(st);
  }
}
