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
#include <stdint.h>
#include <stdio.h>

#include "bitboards.h"
#include "knur.h"
#include "position.h"
#include "rand.h"
#include "util.h"

static inline void add_enpas(Position *pos, Square sq);
static inline void add_piece(Position *pos, PieceType pt, Color c, Square sq);
static inline void rem_enpas(Position *pos);
static inline void rem_piece(Position *pos, PieceType pt, Color c, Square sq);
static inline void flip_furn(Position *pos);

static struct {
  Key turn;
  Key piece[2][6][64];
  Key castle[16];
  Key enpas[8];
} zobrist;

static inline void
add_enpas(Position *pos, Square sq)
{
  pos->key ^= zobrist.enpas[sq & 7];
  pos->st->enpas = sq;
}

static inline void
add_piece(Position *pos, PieceType pt, Color c, Square sq)
{
  U64 bb = GET_BITBOARD(sq);
  pos->key ^= zobrist.piece[c][pt][sq];
  pos->color[c] |= bb;
  pos->piece[pt] |= bb;
  pos->board[sq] = pt;
}

static inline void
rem_enpas(Position *pos)
{
  if (pos->st->enpas != SQ_NONE) {
    pos->key ^= zobrist.enpas[pos->st->enpas & 7];
    pos->st->enpas = SQ_NONE;
  }
}

static inline void
rem_piece(Position *pos, PieceType pt, Color c, Square sq)
{
  U64 bb = GET_BITBOARD(sq);
  pos->key ^= zobrist.piece[c][pt][sq];
  pos->color[c] ^= bb;
  pos->piece[pt] ^= bb;
  pos->board[sq] = NONE;
}

static inline void
flip_furn(Position *pos)
{
  pos->key ^= zobrist.turn;
  pos->turn ^= 1;
}

void
pos_print(const Position *pos)
{
  Square sq;
  const char *turns[] = { "WHITE", "BLACK" };
  const char ptc[] = { 'P', 'N', 'B', 'R', 'Q', 'K', ' ' };
  const char *sqs[] = {
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
    "NO"
  };
  Rank r;
  File f;
  const char *sep = "  +---+---+---+---+---+---+---+---+";

  printf("%s\n", sep);
  for (sq = SQ_A8, r = 8; r >= 1; r--) {
    printf("%d ", r);
    for (f = 0; f < 8; f++, sq++)
      printf("| %c ",
          ptc[pos->board[sq]] | (GET_BIT(pos->color[BLACK], sq) ? 32 : 0));
    printf("|\n%s\n", sep);
  }
  printf("    a   b   c   d   e   f   g   h\n\n");
  printf("    Turn: %s     Enpassant: %s\n",
      turns[pos->turn], sqs[pos->st->enpas]);
  printf("    Castling:       %c%c%c%c\n",
      pos->st->castle & 4 ? 'K' : '-', pos->st->castle & 1 ? 'Q' : '-',
      pos->st->castle & 8 ? 'k' : '-', pos->st->castle & 2 ? 'q' : '-');
  printf("    Hash key:       %lu\n", pos->key);
}

void
pos_set(Position *pos, const char *fen)
{
  unsigned i;
  State *st;
  Square sq;
  Color c;
  char z;
  Rank r;
  File f;

  pos->turn = WHITE;
  for (i = 0; i < LENGTH(pos->color); i++)
    pos->color[i] = 0ULL;
  for (i = 0; i < LENGTH(pos->piece); i++)
    pos->piece[i] = 0ULL;
  for (i = 0; i < LENGTH(pos->board); i++)
    pos->board[i] = NONE;
  for (i = 0; i < LENGTH(pos->ksq); i++)
    pos->ksq[i] = SQ_NONE;
  pos->game_ply = 0;

  while (pos->st) {
    st = pos->st->prev;
    free(pos->st);
    pos->st = st;
  }
  
  pos->st = emalloc(sizeof(State));
  pos->st->enpas = SQ_NONE;
  pos->st->castle = 0;
  pos->st->fifty = 0;
  pos->st->captured = NONE;
  pos->st->prev = NULL;

  for (sq = SQ_A8; sq <= SQ_H1; fen++) {
    if (*fen == '/') {
      continue;
    } else if (isdigit(*fen)) {
      sq += *fen - '0';
    } else {
      z = *fen | 32; /* lower case letters */
      c = (z == *fen ? BLACK : WHITE);
      switch (z) {
      case 'p': add_piece(pos,   PAWN, c, sq); break;
      case 'n': add_piece(pos, KNIGHT, c, sq); break;
      case 'b': add_piece(pos, BISHOP, c, sq); break;
      case 'r': add_piece(pos,   ROOK, c, sq); break;
      case 'q': add_piece(pos,  QUEEN, c, sq); break;
      case 'k': add_piece(pos,   KING, c, sq); break;
      }
      sq++;
    }
  }
  pos->empty = ~(pos->color[WHITE] | pos->color[BLACK]);
  pos->ksq[WHITE] = GET_SQUARE(pos->color[WHITE] & pos->piece[KING]);
  pos->ksq[BLACK] = GET_SQUARE(pos->color[BLACK] & pos->piece[KING]);

  if (*(++fen) == 'b')
    flip_furn(pos);
  while (*(++fen) == ' ');

  while (*fen != ' ') {
    switch (*fen++) {
    case 'K': pos->st->castle |= 4; break;
    case 'Q': pos->st->castle |= 1; break;
    case 'k': pos->st->castle |= 8; break;
    case 'q': pos->st->castle |= 2; break;
    default: break;
    }
  }
  pos->key ^= zobrist.castle[pos->st->castle];

  if (*(++fen) != '-') {
    f = fen[0] - 'a';
    r = 8 - (fen[1] - '0');
    add_enpas(pos, f + r * 8);
  }

  /* TODO */
  /* fifty move rule */
}

void
zobrist_init(void)
{
  unsigned i, j, k;
  for (i = 0; i < 2; i++)
    for (j = 0; j < 6; j++)
      for (k = 0; k < 64; k++)
        zobrist.piece[i][j][k] = rand_u64();
  for (i = 0; i < 16; i++)
    zobrist.castle[i] = rand_u64();
  for (i = 0; i < 8; i++)
    zobrist.enpas[i] = rand_u64();
}
