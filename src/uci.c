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

#include "knur.h"
#include "movegen.h"
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
  void (*func)(Position *, const char *); /**< Function. */
};

static void display(Position *, const char *);
static void go(Position *, const char *);
static void isready(Position *, const char *);
static Move parse_move(Position *pos, char *move_string);
static void position(Position *, const char *);
static void quit(Position *, const char *);
static void uci(Position *, const char *);
static void ucinewgame(Position *, const char *);

static Parser parser[] = {
  { "d", display },
  { "go", go },
  { "isready", isready },
  { "position", position },
  { "q", quit },
  { "quit", quit },
  { "uci", uci },
  { "ucinewgame", ucinewgame },
};

static void
display(Position *pos, const char *input)
{
  (void)input;
  pos_print(pos);
}

static void
go(Position *pos, const char *input)
{
  char *token = NULL;
  int perft_depth = -1, depth = -1;
  int movestogo = 30, movetime = -1;
  int time = -1, inc = 0;

  if (!info.stop)
    return;

  info.timeset = 0;

  if (pos->turn == WHITE) {
    if ((token = strstr(input, "winc")))
      inc = atoi(token + 5);
    if ((token = strstr(input, "wtime")))
      time = atoi(token + 6);
  } else {
    if ((token = strstr(input, "binc")))
      inc = atoi(token + 5);
    if ((token = strstr(input, "btime")))
      time = atoi(token + 6);
  }

  if ((token = strstr(input, "movestogo")))
    movestogo = atoi(token + 10);
  if ((token = strstr(input, "movetime")))
    movetime = atoi(token + 10);

  if ((token = strstr(input, "perft")))
    perft_depth = atoi(token + 6);

  if ((token = strstr(input, "depth")))
    depth = atoi(token + 6);

  if (perft_depth != -1) {
    perft(pos, perft_depth);
    return;
  }

  info.starttime = gettime();

  if (movetime != -1) {
    movestogo = 1;
    time = movetime;
  }

  if (time != -1) {
    time /= movestogo;
    time -= 50;
    info.timeset = 1;
    info.stoptime = info.starttime + time + inc;
  }

  info.depth = depth;

  search(pos);
}

static void
isready(Position *pos, const char *input)
{
  (void)pos, (void)input;
  printf("readyok\n");
}

static Move
parse_move(Position *pos, char *move_string)
{
  Move move_list[256], *m, *last;
  Square from = move_string[0] - 'a' + (('8' - move_string[1]) << 3);
  Square to   = move_string[2] - 'a' + (('8' - move_string[3]) << 3);

  last = generate_moves(GT_ALL, move_list, pos);
  for (m = move_list; m != last; m++) {
    if (from != FROM_SQ(*m) || to != TO_SQ(*m) || !is_legal(pos, *m))
      continue;
    if (TYPE_OF(*m) == PROMOTION)
      switch (PROMOTION_TYPE(*m)) {
      case KNIGHT: if (move_string[4] == 'n') return *m;
      case BISHOP: if (move_string[4] == 'b') return *m;
      case   ROOK: if (move_string[4] == 'r') return *m;
      case  QUEEN: if (move_string[4] == 'q') return *m;
      }
    else
      return *m;
  }
  return MOVE_NONE;
}

static void
position(Position *pos, const char *input)
{
  char *token = NULL;
  if (!strncmp(input, "startpos", 8)) {
    pos_set(pos, STARTPOS);
    input += 8;
  } else {
    token = strstr(input, "fen");
    if (!token) {
      pos_set(pos, STARTPOS);
    } else {
      token += 4;
      pos_set(pos, token);
    }
  }
  token = strstr(input, "moves");
  if (token) {
    token += 6;
    Move m;
    while (*token) {
      if ((m = parse_move(pos, token)) == MOVE_NONE)
        break;
      do_move(pos, m);
      while (*token && *token++ != ' ');
    }
  }
}

static void
quit(Position *pos, const char *input)
{
  (void)pos, (void)input;
  info.quit = 1;
}

static void
uci(Position *pos, const char *input)
{
  (void)pos, (void)input;
  printf("id name Knur\n");
  printf("id author Stanisław Bitner\n");
  printf("uciok\n");
}

static void
ucinewgame(Position *pos, const char *input)
{
  (void)input;
  pos_set(pos, STARTPOS);
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

  for (info.quit = 0, info.stop = 1; !info.quit; ) {
    memset(input, 0, sizeof(input));
    fflush(stdout);

    readline(input);
    for (i = 0, p = parser; i < LENGTH(parser); i++, p++) {
      len = strlen(p->cmd);
      if ((!input[len] || isspace(input[len]))
      &&  !strncmp(input, p->cmd, len)) {
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
