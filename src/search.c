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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "evaluate.h"
#include "knur.h"
#include "movegen.h"
#include "movesort.h"
#include "position.h"
#include "search.h"
#include "tt.h"
#include "util.h"

/** \typedef PV
 * Defines structure PV.
 */
typedef struct PV PV;

/** \struct PV
 * A structure for storing principal variation.
 * Dynamic memory management seems to be faster in this case.
 * When running search for 2 323 511 178 nodes
 * moves[64] -> 193427ms
 * moves*    -> 168569ms
 */
struct PV {
  /**{*/
  size_t len; /**< Number of pv moves stored; */
  Move *line; /**< Array for storing pv moves. */
  /**}*/
};

static inline void checkstop(void);
static inline int is_rep(Position *pos);
static const char *mtstr(Move m);
static int negamax(Position *pos, PV *pv, int alpha, int beta, int depth);
static PV *pv_create(size_t len);
static void pv_free(PV *pv);
static int quiescence(Position *pos, int alpha, int beta);

SearchInfo info;

static inline void
checkstop(void)
{
  if (info.timeset == 1 && gettime() > info.stoptime)
    info.stop = 1;
}

static inline int
is_rep(Position *pos)
{
  int i = pos->game_ply - 1, n = 1;
  while (i >= pos->game_ply - pos->st->fifty && n < 3)
    n += (pos->reps[i--] == pos->key);
  return n >= 3;
}

static const char *
mtstr(Move m)
{
  const char ptc[6] = { 'p', 'n', 'b', 'r', 'q', 'k' };
  static char res[6];
  sprintf(res, "%c%c%c%c%c",
      'a' + (FROM_SQ(m) & 7), '8' - (FROM_SQ(m) >> 3),
      'a' + (  TO_SQ(m) & 7), '8' - (  TO_SQ(m) >> 3),
      TYPE_OF(m) == PROMOTION ? ptc[PROMOTION_TYPE(m)] : '\0');
  return res;
}

static int
negamax(Position *pos, PV *pv, int alpha, int beta, int depth)
{
  int isroot = !pos->ply;
  int score = -CHECKMATE;
  int bestscore = -CHECKMATE;
  int oldalpha = alpha;
  int pvnode = beta - alpha != 1;
  int moves_searched = 0;
  Move bestmove = MOVE_NONE;
  Move hashmove;
  Move move_list[256], *last, *m;
  PV *new_pv;
  U64 checkers = attackers_to(pos, pos->ksq[pos->turn], ~pos->empty)
               & pos->color[!pos->turn];

  if (!isroot) {
    /* dont end search if in check */
    if (!depth) {
      if (!checkers)
        return quiescence(pos, alpha, beta);
      depth = 1;
    }

    /* have to end search or it will seg fault */
    if (pos->ply >= MAX_PLY)
      return checkers ? 0 : evaluate(pos);

    /* draw by fifty move rule or 3 fold repetition */
    if (pos->st->fifty >= 100 || is_rep(pos))
      return 0;

    /* mate distance pruning */
    alpha = MAX(alpha, pos->ply - CHECKMATE);
    beta  = MIN(beta,  CHECKMATE - pos->ply - 1);
    if (alpha >= beta)
      return alpha;
  }

  if (!(info.nodes & 4095))
    checkstop();

  info.nodes++;

  hashmove = isroot ? info.bestmove : MOVE_NONE;

  if (!isroot && !pvnode && tt_probe(pos->tt, pos->key, &score, &hashmove,
                                     depth, alpha, beta, pos->ply))
    return score;

  new_pv = pv_create(MAX_PLY - pos->ply);

  if (!pvnode && !checkers) {
    /* null move pruning */
    if (depth >= 4 && info.donull
    && pos->material[pos->turn] > 800) {
      info.donull = 0;
      do_null_move(pos);
      score = -negamax(pos, new_pv, -beta, -beta + 1, depth - 3);
      undo_null_move(pos);
      info.donull = 1;

      if (score >= beta && abs(score) < ISCHECKMATE) {
        pv_free(new_pv);
        return beta;
      }
    }
  }

  last = generate_moves(GT_ALL, move_list, pos);
  last = process_moves(move_list, last, hashmove, pos);

  if (move_list == last) {
    pv_free(new_pv);
    return checkers ? pos->ply - CHECKMATE : STALEMATE;
  }

  sort_moves(move_list, last);

  for (m = move_list; m != last; m++, moves_searched++) {
    do_move(pos, *m);

    /* normal search */
    if (!moves_searched) {
      score = -negamax(pos, new_pv, -beta, -alpha, depth - 1);
    } else {
      score = alpha + 1;
      /* late move reduction */
      if (moves_searched > 4 && depth > 3
      &&  TYPE_OF(*m) == NORMAL && pos->st->captured == NONE
      && !(attackers_to(pos, pos->ksq[pos->turn], ~pos->empty) & pos->color[!pos->turn]))
        score = -negamax(pos, new_pv, -alpha - 1, -alpha, depth - 3 - !pvnode);
      /* principal variation search */
      if (score > alpha) {
        score = -negamax(pos, new_pv, -alpha - 1, -alpha, depth - 1);
        if (score > alpha)
          score = -negamax(pos, new_pv, -beta, -alpha, depth - 1);
      }
    }

    undo_move(pos, *m);

    if (info.stop) {
      pv_free(new_pv);
      return 0;
    }

    if (score > bestscore) {
      bestscore = score;
      bestmove  = *m;
      if (score >= beta) {
        /* killer move */
        if (pos->board[TO_SQ(*m)] == NONE) {
          /* update killer moves */
          /* we dont want both killers to be equal */
          if (pos->killer[0][pos->ply] != (*m & 0xFFFF))
            pos->killer[1][pos->ply] = pos->killer[0][pos->ply];
          pos->killer[0][pos->ply] = *m & 0xFFFF;
        }
        tt_store(pos->tt, pos->key, score, *m & 0xFFFF, depth, TT_BETA);
        pv_free(new_pv);
        return score;
      }

      if (score > alpha) {
        pv->line[0] = *m;
        pv->len = new_pv->len + 1;
        memcpy(pv->line + 1, new_pv->line, new_pv->len * sizeof(Move));
        alpha = score;
        if (pos->board[TO_SQ(*m)] == NONE)
          pos->hh[pos->turn][pos->board[FROM_SQ(*m)]][TO_SQ(*m)] +=
            MIN(depth * depth, 69);
      }
    }
  }

  tt_store(pos->tt, pos->key, alpha, bestmove & 0xFFFF, depth,
           alpha == oldalpha ? TT_ALPHA : TT_PV);

  pv_free(new_pv);
  return alpha;
}

static PV *
pv_create(size_t len)
{
  PV *pv = emalloc(sizeof(PV));
  pv->len = 0;
  pv->line = emalloc(len * sizeof(Move));
  return pv;
}

static void
pv_free(PV *pv)
{
  if (pv) {
    free(pv->line);
    free(pv);
  }
}

static int
quiescence(Position *pos, int alpha, int beta)
{
  if (!(info.nodes & 4095))
    checkstop();

  info.nodes++;
  int value = evaluate(pos);
  if (value >= beta)
    return beta;
  if (value > alpha)
    alpha = value;

  Move *m, *last, move_list[256];
  last = generate_moves(GT_CAPTURES, move_list, pos);
  last = process_moves(move_list, last, MOVE_NONE, pos);

  if (move_list == last)
    return alpha;

  sort_moves(move_list, last);
  for (m = move_list; m != last; m++) {
    do_move(pos, *m);
    value = -quiescence(pos, -beta, -alpha);
    undo_move(pos, *m);

    if (info.stop)
      return 0;

    if (value >= beta)
      return value;
    if (value > alpha)
      alpha = value;
  }

  return alpha;
}

void
search(Position *pos)
{
  int score, depth;
  int alpha = -INFINITY, beta = INFINITY;
  unsigned start, i;
  PV *pv;

  /* setup */
  info.stop = 0;
  info.depth = info.depth == -1 ? MAX_PLY : info.depth;
  info.donull = 1;
  info.bestmove = MOVE_NONE;
  pos->ply = 0;
  pos->killer[0] = ecalloc(MAX_PLY, sizeof(Move));
  pos->killer[1] = ecalloc(MAX_PLY, sizeof(Move));
  memset(pos->hh, 0, sizeof(pos->hh));
  pv = pv_create(MAX_PLY);
  tt_update(pos->tt);

  /* iterative deepening */
  for (depth = 1; depth <= info.depth; depth++) {
    info.nodes = 0;
    start = gettime();

    score = negamax(pos, pv, alpha, beta, depth);

    if (info.stop)
      break;

    info.bestmove = *pv->line & 0xFFFF;

    printf("info depth %d ", depth);
    if (CHECKMATE - score <= MAX_PLY)
      printf("mate %d ", (CHECKMATE - score + 1) / 2);
    else
      printf("score cp %d ", score);
    printf("nodes %lu time %d pv", info.nodes, gettime() - start);
    for (i = 0; i < pv->len; i++)
      printf(" %s", mtstr(pv->line[i]));
    printf("\n");
  }

  printf("bestmove %s\n", mtstr(info.bestmove));

  /* cleanup */
  info.stop = 1;
  free(pos->killer[0]);
  free(pos->killer[1]);
  pv_free(pv);
}
