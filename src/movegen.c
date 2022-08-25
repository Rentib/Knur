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
#include "movegen.h"
#include "position.h"

static Move *pawn_moves(GenType gt, Move *move_list, Position *pos, U64 target);
static Move *piece_moves(PieceType pt, Move *move_list, Position *pos, U64 target);

static Move *
pawn_moves(GenType gt, Move *move_list, Position *pos, U64 target)
{
  const Color us = pos->turn, them = !us;
  const Direction up = us == WHITE ? NORTH : SOUTH;
  const Direction upw = up + WEST, upe = up + EAST;
  const U64 rank4 = us == WHITE ? Rank4BB : Rank5BB;
  const U64 rank7 = us == WHITE ? Rank7BB : Rank2BB;
  const U64 empty = pos->empty & target; /* empty squares that are targets */
  const U64 enemies = pos->color[them] & target; /* enemies that are targets */
  const U64 pawns = pos->piece[PAWN] & pos->color[us] & ~rank7;
  const U64 promo = pos->piece[PAWN] & pos->color[us] &  rank7;
  U64 b1, b2;
  Square to;

  /* quiet moves - single and double pawn pushes */
  if (gt != GT_CAPTURES) {
    b1 = shift(up, pawns) & pos->empty;
    b2 = shift(up, b1) & empty & rank4;
    b1 &= target;
    while (b1) {
      to = pop_lsb(&b1);
      *move_list++ = MAKE_MOVE(to - up, to);
    }
    while (b2) {
      to = pop_lsb(&b2);
      *move_list++ = MAKE_MOVE(to - up - up, to);
    }
  }

  /* captures - normal and en_passant */
  if (gt != GT_QUIETS) {
    b1 = shift(upw, pawns) & enemies;
    b2 = shift(upe, pawns) & enemies;
    while (b1) {
      to = pop_lsb(&b1);
      *move_list++ = MAKE_MOVE(to - upw, to);
    }
    while (b2) {
      to = pop_lsb(&b2);
      *move_list++ = MAKE_MOVE(to - upe, to);
    }

    if (pos->st->enpas != SQ_NONE)
      for (b1 = pawns & pawn_attacks_bb(them, pos->st->enpas); b1; )
        *move_list++ = MAKE_EN_PASSANT(pop_lsb(&b1), pos->st->enpas);
  }

  /* promotions - quiet and captures */
  return move_list;
}

static Move *
piece_moves(PieceType pt, Move *move_list, Position *pos, U64 target)
{
  U64 pieces = pos->piece[pt] & pos->color[pos->turn];
  U64 attacks;
  Square from;
  while (pieces) {
    from = pop_lsb(&pieces);
    attacks = attacks_bb(pt, from, ~pos->empty) & target;
    while (attacks)
      *move_list++ = MAKE_MOVE(from, pop_lsb(&attacks));
  }
  return move_list;
}

Move *
generate_moves(GenType gt, Move *move_list, Position *pos)
{
  const Color us = pos->turn, them = !us;
  const Square ksq = pos->ksq[us];
  const U64 empty = pos->empty;
  const U64 checkers = attackers_to(pos, ksq, ~empty) & pos->color[them];
  U64 target = gt == GT_CAPTURES ? pos->color[them]
                                 : ~pos->color[us];
  move_list = piece_moves(KING, move_list, pos, target);
  if (POPCOUNT(checkers) < 2) {
    if (checkers)
      target &= between_bb(ksq, GET_SQUARE(checkers));
    else ;
      /* generate castle moves */
    move_list = pawn_moves(gt, move_list, pos, target);
    move_list = piece_moves(KNIGHT, move_list, pos, target);
    move_list = piece_moves(BISHOP, move_list, pos, target);
    move_list = piece_moves(  ROOK, move_list, pos, target);
    move_list = piece_moves( QUEEN, move_list, pos, target);
  }
  return move_list;
}
