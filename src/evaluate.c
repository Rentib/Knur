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

#include "bitboards.h"
#include "knur.h"
#include "evaluate.h"
#include "position.h"

/* 
 * Knur uses a very simple evaluation method:
 * - material
 * - mobility
 * - piece square tables
 * - king safety
 * - outposts
 * - bishop pair
 * - open files
 * - passed pawns
 * - doubled pawns
 * - isolated pawns
 */

#define FLIP(square)        ((square) ^ 56)

typedef enum { MID_GAME, END_GAME } GamePhase;
static GamePhase phase;

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

static U64 rank_mask[8];       /* [Rank] */
static U64 file_mask[8];       /* [File] */
static U64 adj_file_mask[8];   /* [File] */
static U64 passed_mask[2][64]; /* [Color][Square] */

static const int passed[8][2]   = {{0,0},{53,100},{31,45},{15,22},{8,14},{5,9},{2,5},{0,0}};
static const int isolated[2] = { -10, -25 };
static const int doubled[2] = { -15, -25 };

static const int outpost[2] = { 15, 9 };
static const int knight_adj[9] = { -10, -8, -6, -4, -2, 0, 2, 4, 8 };

static const int bishop_pair[2] = { 20, 40 };

static const int open_file[2]   = { 10, 20 }; /* semiopen, open */
static const int king_file[2]   = { 10, 20 };
static const int rook_adj[9] = { 13, 12, 10, 7, 3, 0, -3, -6, -9 };

static const int king_shield[2] = { 2, 7 };

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

  for (sq = SQ_A8; sq <= SQ_H1; sq++) {
    passed_mask[WHITE][sq] = (adj_file_mask[sq & 7] | file_mask[sq & 7])
                           &  ((1ULL << sq) - 1) & ~rank_mask[sq >> 3];
    passed_mask[BLACK][sq] = (adj_file_mask[sq & 7] | file_mask[sq & 7])
                           & ~((1ULL << sq) - 1) & ~rank_mask[sq >> 3];
  }
}

int
evaluate(const Position *pos)
{
  int value       = pos->material[WHITE] - pos->material[BLACK];
  U64 white_pawns = pos->color[WHITE] & pos->piece[PAWN];
  U64 black_pawns = pos->color[BLACK] & pos->piece[PAWN];
  U64 occupancy   = ~pos->empty;
  U64 mask;
  Square sq, fsq;
  int wpanws_cnt = POPCOUNT(white_pawns);
  int bpanws_cnt = POPCOUNT(black_pawns);

  /* Game Phase */
  phase = pos->material[WHITE] + pos->material[BLACK] > 3000 ? MID_GAME : END_GAME;

  /* Pawns */

  for (mask = white_pawns; mask; ) {
    sq = pop_lsb(&mask);
    value += pawn_pcsqt[sq][phase];
    if (!(passed_mask[WHITE][sq] & black_pawns))
      value += passed[sq >> 3][phase];
    if (file_mask[sq & 7] & white_pawns & passed_mask[WHITE][sq])
      value += doubled[phase];
    if (!(adj_file_mask[sq & 7] & white_pawns))
      value += isolated[phase];
  }

  for (mask = black_pawns; mask; ) {
    sq = pop_lsb(&mask);
    fsq = FLIP(sq);
    value -= pawn_pcsqt[fsq][phase];
    if (!(passed_mask[BLACK][sq] & white_pawns))
      value -= passed[fsq >> 3][phase];
    if (file_mask[sq & 7] & black_pawns & passed_mask[BLACK][sq])
      value -= doubled[phase];
    if (!(adj_file_mask[sq & 7] & white_pawns))
      value -= isolated[phase];
  }

  /* Knights */

  for (mask = pos->color[WHITE] & pos->piece[KNIGHT]; mask; ) {
    sq = pop_lsb(&mask);
    value += knight_pcsqt[sq][phase];
    value += POPCOUNT(attacks_bb(KNIGHT, sq, occupancy) & ~pos->color[WHITE]);
    if (!(adj_file_mask[sq & 7] & passed_mask[WHITE][sq] & black_pawns))
      value += outpost[phase];
    value += knight_adj[wpanws_cnt];
  }

  for (mask = pos->color[BLACK] & pos->piece[KNIGHT]; mask; ) {
    sq = pop_lsb(&mask);
    value -= knight_pcsqt[FLIP(sq)][phase];
    value -= POPCOUNT(attacks_bb(KNIGHT, sq, occupancy) & ~pos->color[BLACK]);
    if (!(adj_file_mask[sq & 7] & passed_mask[BLACK][sq] & white_pawns))
      value -= outpost[phase];
    value -= knight_adj[bpanws_cnt];
  }

  /* Bishops */

  mask = pos->color[WHITE] & pos->piece[BISHOP];
  if (mask & (mask - 1))
    value += bishop_pair[phase];

  for (; mask; ) {
    sq = pop_lsb(&mask);
    value += bishop_pcsqt[sq][phase];
    value += POPCOUNT(attacks_bb(BISHOP, sq, occupancy) & ~pos->color[WHITE]);
    if (!(adj_file_mask[sq & 7] & passed_mask[WHITE][sq] & black_pawns))
      value += outpost[phase];
  }

  mask = pos->color[BLACK] & pos->piece[BISHOP];
  if (mask & (mask - 1))
    value -= bishop_pair[phase];

  for (; mask; ) {
    sq = pop_lsb(&mask);
    value -= bishop_pcsqt[FLIP(sq)][phase];
    value -= POPCOUNT(attacks_bb(BISHOP, sq, occupancy) & ~pos->color[BLACK]);
    if (!(adj_file_mask[sq & 7] & passed_mask[BLACK][sq] & white_pawns))
      value -= outpost[phase];
  }

  /* Rooks */

  for (mask = pos->color[WHITE] & pos->piece[ROOK]; mask; ) {
    sq = pop_lsb(&mask);
    value += rook_pcsqt[sq];
    if (!(file_mask[sq & 7] & pos->piece[PAWN]))
      value += open_file[1];
    else if (!(file_mask[sq & 7] & white_pawns))
      value += open_file[0];
    if ((sq & 7) == (pos->ksq[BLACK] & 7)
    || (sq >> 3) == (pos->ksq[BLACK] >> 3))
      value += king_file[phase];
    value += rook_adj[wpanws_cnt];
  }

  for (mask = pos->color[BLACK] & pos->piece[ROOK]; mask; ) {
    sq = pop_lsb(&mask);
    value -= rook_pcsqt[FLIP(sq)];
    if (!(file_mask[sq & 7] & pos->piece[PAWN]))
      value -= open_file[1];
    else if (!(file_mask[sq & 7] & black_pawns))
      value -= open_file[0];
    if ((sq & 7) == (pos->ksq[WHITE] & 7)
    || (sq >> 3) == (pos->ksq[WHITE] >> 3))
      value -= king_file[phase];
    value -= rook_adj[bpanws_cnt];
  }

  /* Queens */

  /* Kings */

  value += king_pcsqt[pos->ksq[WHITE]][phase] - king_pcsqt[FLIP(pos->ksq[BLACK])][phase];
  value += king_shield[phase] *
           (POPCOUNT(attacks_bb(KING, pos->ksq[WHITE], 0) & white_pawns) -
            POPCOUNT(attacks_bb(KING, pos->ksq[BLACK], 0) & black_pawns));

  return pos->turn == WHITE ? value : -value;
}
