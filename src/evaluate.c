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

#include <byteswap.h>

#include "bitboards.h"
#include "evaluate.h"
#include "knur.h"
#include "position.h"

#define FLIP(bitboard)        bitboard = bswap_64(bitboard)
#define SQRANK(square)        ((square) >> 3)
#define SQFILE(square)        ((square) & 7)
#define SQFLIP(square)        ((square) ^ 56)

typedef enum { MID_GAME, END_GAME } GamePhase;

static int eval_pawns(const Color side, const Position *pos);
static int eval_knights(const Color side, const Position *pos);
static int eval_bishops(const Color side, const Position *pos);
static int eval_rooks(const Color side, const Position *pos);

/* Piece Square Tables */
static const int pawn_pcsqt[64][2] = {
  { 0,0}, { 0,0}, { 0,0}, { 0,0}, { 0,0}, { 0,0}, { 0,0}, { 0,0},
  { 3,7}, { 0,5}, { 0,5}, { 0,5}, { 0,5}, { 0,5}, { 0,5}, { 3,7},
  { 2,6}, { 0,4}, { 0,4}, { 0,4}, { 0,4}, { 0,4}, { 0,4}, { 2,6},
  { 1,5}, { 0,3}, { 0,3}, { 0,3}, { 0,3}, { 0,3}, { 0,3}, { 1,5},
  { 1,4}, { 0,2}, { 5,2}, {20,2}, {20,2}, { 5,2}, { 0,2}, { 1,4},
  { 5,3}, {10,1}, { 0,1}, {10,1}, {10,1}, {-5,1}, {10,1}, { 5,3},
  {10,2}, {10,0}, { 9,0}, { 5,0}, { 5,0}, {10,0}, {10,0}, {10,2},
  { 0,0}, { 0,0}, { 0,0}, { 0,0}, { 0,0}, { 0,0}, { 0,0}, { 0,0},
};

static const int knight_pcsqt[64][2] = {
  {-15,-25}, { 0,0}, { 0,0}, { 0,0}, { 0,0}, { 0,0}, { 0,0}, {-15,-25},
  {-10,-20}, { 0,0}, { 0,0}, { 0,0}, { 0,0}, { 0,0}, { 0,0}, {-10,-20},
  {-10,-20}, { 0,0}, {10,0}, {10,0}, {10,0}, {10,0}, { 0,0}, {-10,-20},
  {-10,-20}, { 5,0}, {10,0}, {20,0}, {20,0}, {10,0}, { 5,0}, {-10,-20},
  {-10,-20}, { 5,0}, {10,0}, {20,0}, {20,0}, {10,0}, { 5,0}, {-10,-20},
  {-10,-20}, { 0,0}, {10,0}, { 5,0}, { 5,0}, {10,0}, { 0,0}, {-10,-20},
  {-10,-20}, { 0,0}, { 0,0}, { 0,0}, { 0,0}, { 0,0}, { 0,0}, {-10,-20},
  {-15,-25}, {-6,0}, { 0,0}, { 0,0}, { 0,0}, { 0,0}, {-8,0}, {-15,-25},
};

static const int bishop_pcsqt[64][2] = {
  { 0,0}, { 0,0}, { 0,0}, { 0,0}, { 0,0}, { 0,0}, { 0,0}, { 0,0},
  { 0,0}, { 0,5}, { 0,5}, { 0,5}, { 0,5}, { 0,5}, { 0,5}, { 0,0},
  { 0,0}, { 0,5}, { 0,9}, { 0,9}, { 0,9}, { 0,9}, { 0,5}, { 0,0},
  { 0,0}, {18,5}, { 0,9}, { 0,9}, { 0,9}, { 0,9}, {18,5}, { 0,0},
  { 0,0}, { 0,5}, {20,9}, {20,9}, {20,9}, {20,9}, { 0,5}, { 0,0},
  { 5,0}, { 0,5}, { 7,9}, {10,9}, {10,9}, { 7,9}, { 0,5}, { 5,0},
  { 0,0}, {10,5}, { 0,5}, { 7,5}, { 7,5}, { 0,5}, {10,5}, { 0,0},
  { 0,0}, { 0,0}, {-6,0}, { 0,0}, { 0,0}, {-8,0}, { 0,0}, { 0,0},
};

static const int rook_pcsqt[64] = {
   5,  5,  7, 10, 10,  7,  5,  5,
  20, 20, 20, 20, 20, 20, 20, 20,
   0,  0,  5, 10, 10,  5,  0,  0,
   0,  0,  5, 10, 10,  5,  0,  0,
   0,  0,  5, 10, 10,  5,  0,  0,
   0,  0,  5, 10, 10,  5,  0,  0,
   0,  0,  5, 10, 10,  5,  0,  0,
};

static const int king_pcsqt[64][2] = {
  {-10,-50}, {-10,-20}, {-10,-20}, {-10,-20}, {-10,-20}, {-10,-20}, {-10,-20}, {-10,-50},
  {-10,-10}, {-10,  0}, {-10,  0}, {-10,  0}, {-10,  0}, {-10,  0}, {-10,  0}, {-10,-10},
  {-10,-10}, {-10,  0}, {-10,  0}, {-10,  0}, {-10,  0}, {-10,  0}, {-10,  0}, {-10,-10},
  {-10,-10}, {-10,  0}, {-10,  0}, {-10,  0}, {-10,  0}, {-10,  0}, {-10,  0}, {-10,-10},
  {-10,-10}, {-10,  0}, {-10,  0}, {-10,  0}, {-10,  0}, {-10,  0}, {-10,  0}, {-10,-10},
  {-10,-10}, {-10,  0}, {-10,  0}, {-10,  0}, {-10,  0}, {-10,  0}, {-10,  0}, {-10,-10},
  {-10,-10}, {-10,  0}, {-10,  0}, {-10,  0}, {-10,  0}, {-10,  0}, {-10,  0}, {-10,-10},
  { 10,-50}, { 20,-20}, { 20,-20}, {-10,-20}, {  0,-20}, {-10,-20}, { 20,-20}, { 20,-50},
};

static GamePhase phase;
static int pawn_cnt[2]; /* side */

/* bonuses in centipawns */
static const int passed[8][2]   = {{0,0},{53,100},{31,45},{15,22},{8,14},{5,9},{2,5},{0,0}};
static const int isolated[2] = { -10, -25 };
static const int doubled[2] = { -15, -25 };
static const int outpost[][2] = { {54,34}, {31,25} }; /* [Knight/Bishop][Phase] */
static const int knight_adj[9] = { -10, -8, -6, -4, -2, 0, 2, 4, 8 };
static const int bishop_pair[2] = { 20, 40 };
static const int bishop_on_long_diagonal[2] = { 13, 9 };
static const int open_file[2]   = { 18, 30 }; /* semiopen, open */
static const int king_file[2]   = { 10, 20 };
static const int rook_adj[9] = { 13, 12, 10, 7, 3, 0, -3, -6, -9 };
static const int king_shield[2] = { 2, 7 };

/* helper masks */
U64 rank_mask[8];
U64 file_mask[8];
U64 adj_file_mask[8];
U64 passed_mask[64];
U64 long_diagonal_mask;

static int
eval_pawns(const Color side, const Position *pos)
{
  int value = 0;
  Square sq;
  U64 mask;
  U64 allied_pawns = pos->color[side] & pos->piece[PAWN];
  U64 enemy_pawns = pos->color[!side] & pos->piece[PAWN];

  pawn_cnt[side] = POPCOUNT(allied_pawns);

  if (!allied_pawns)
    return 0;

  if (side == BLACK) {
    FLIP(allied_pawns);
    FLIP(enemy_pawns);
  }

  for (mask = allied_pawns; mask; ) {
    sq = pop_lsb(&mask);
    value += pawn_pcsqt[sq][phase];

    /* passed */
    if (!(passed_mask[sq] & enemy_pawns))
      value += passed[SQRANK(sq)][phase];

    /* doubled */
    if (file_mask[SQFILE(sq)] & passed_mask[sq] & allied_pawns)
      value += doubled[phase];

    /* isolated */
    if (!(adj_file_mask[SQFILE(sq)] & allied_pawns))
      value += isolated[phase];
  }

  return value;
}

static int
eval_knights(const Color side, const Position *pos)
{
  int value = 0;
  Square sq;
  U64 allies = pos->color[side];
  U64 enemies = pos->color[!side];
  U64 knights = allies & pos->piece[KNIGHT];
  U64 allied_pawns = allies & pos->piece[PAWN];
  U64 enemy_pawns = enemies & pos->piece[PAWN];

  if (!knights)
    return 0;

  if (side == BLACK) {
    FLIP(allies);
    FLIP(enemies);
    FLIP(knights);
    FLIP(allied_pawns);
    FLIP(enemy_pawns);
  }

  while (knights) {
    sq = pop_lsb(&knights);
    value += knight_pcsqt[sq][phase];

    /* mobility */
    value += POPCOUNT(attacks_bb(KNIGHT, sq, 0) & ~allies);

    /* outpost */
    if (SQ_A6 <= sq && sq <= SQ_H4 && (allied_pawns & pawn_attacks_bb(!side, sq))
    && !(adj_file_mask[SQFILE(sq)] & passed_mask[sq] & enemy_pawns))
      value += outpost[0][phase];

    /* adjust value depending on number of pawns */
    value += knight_adj[pawn_cnt[side]];
  }

  return value;
}

static int
eval_bishops(const Color side, const Position *pos)
{
  int value = 0;
  Square sq;
  U64 allies = pos->color[side];
  U64 enemies = pos->color[!side];
  U64 bishops = allies & pos->piece[BISHOP];
  U64 allied_pawns = allies & pos->piece[PAWN];
  U64 enemy_pawns = enemies & pos->piece[PAWN];

  if (!bishops)
    return 0;

  if (side == BLACK) {
    FLIP(allies);
    FLIP(enemies);
    FLIP(bishops);
    FLIP(allied_pawns);
    FLIP(enemy_pawns);
  }

  /* bishop pair */
  if (bishops & (bishops - 1))
    value += bishop_pair[phase];

  while (bishops) {
    sq = pop_lsb(&bishops);
    value += bishop_pcsqt[sq][phase];

    /* mobility */
    value += POPCOUNT(attacks_bb(BISHOP, sq, allies | enemies) & ~allies);

    /* outpost */
    if (SQ_A6 <= sq && sq <= SQ_H4 && (allied_pawns & pawn_attacks_bb(!side, sq))
    && !(adj_file_mask[SQFILE(sq)] & passed_mask[sq] & enemy_pawns))
      value += outpost[1][phase];

    /* long diagonal */
    if (GET_BIT(long_diagonal_mask, sq))
      value += bishop_on_long_diagonal[phase];
  }

  return value;
}

static int
eval_rooks(const Color side, const Position *pos)
{
  int value = 0;
  Square sq;
  U64 allies = pos->color[side];
  U64 rooks = allies & pos->piece[ROOK];
  Square ksq = pos->ksq[!side]; /* enemy king */

  if (!rooks)
    return 0;

  if (side == BLACK) {
    FLIP(allies);
    FLIP(rooks);
    ksq = SQFLIP(ksq);
  }

  while (rooks) {
    sq = pop_lsb(&rooks);
    value += rook_pcsqt[sq];

    /* (semi) open file */
    if (!(file_mask[SQFILE(sq)] & pos->piece[PAWN])) /* pos->piece doesnt need flip */
      value += open_file[1];
    else if (!(file_mask[SQFILE(sq)] & pos->piece[PAWN] & pos->color[side]))
      value += open_file[0];

    /* same file/rank as enemy king */
    if (SQFILE(sq) == SQFILE(ksq) || SQRANK(sq) == SQRANK(ksq))
      value += king_file[phase];

    /* adjust value depending on number of pawns */
    value += rook_adj[pawn_cnt[side]];
  }

  return value;
}

void
evaluation_init(void)
{
  Square sq; File f; Rank r;

  for (f = 0; f < 8; f++) {
    for (r = 0; r < 8; r++) {
      SET_BIT(file_mask[f], 8 * r + f);
      SET_BIT(rank_mask[r], 8 * r + f);
    }
    adj_file_mask[f] = shift(WEST, file_mask[f]) | shift(EAST, file_mask[f]);
  }

  for (sq = SQ_A8; sq <= SQ_H1; sq++)
    passed_mask[sq] = (adj_file_mask[SQFILE(sq)] | file_mask[SQFILE(sq)])
                    & (GET_BITBOARD(sq) - 1) & ~rank_mask[SQRANK(sq)];

  long_diagonal_mask = attacks_bb(BISHOP, SQ_A1, 0) | attacks_bb(BISHOP, SQ_H1, 0);
}

int
evaluate(const Position *pos)
{
  int value = 0;

  /* probe tt for evaluation */

  phase = pos->material[WHITE] + pos->material[BLACK] > 3000 ? MID_GAME : END_GAME;

  /* material */
  value += pos->material[WHITE] - pos->material[BLACK];

  value += eval_pawns(WHITE, pos) - eval_pawns(BLACK, pos);
  value += eval_knights(WHITE, pos) - eval_knights(BLACK, pos);
  value += eval_bishops(WHITE, pos) - eval_bishops(BLACK, pos);
  value += eval_rooks(WHITE, pos) - eval_rooks(BLACK, pos);

  value += king_pcsqt[pos->ksq[WHITE]][phase] - king_pcsqt[SQFLIP(pos->ksq[BLACK])][phase];
  value += king_shield[phase] *
           (POPCOUNT(attacks_bb(KING, pos->ksq[WHITE], 0) & pos->piece[PAWN] & pos->color[WHITE]) -
            POPCOUNT(attacks_bb(KING, pos->ksq[BLACK], 0) & pos->piece[PAWN] & pos->color[BLACK]));

  return pos->turn == WHITE ? value : -value;
}
