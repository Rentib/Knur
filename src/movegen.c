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

static Move *piece_moves(PieceType pt, Move *move_list, Position *pos, U64 target);

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
    /* generate pawn moves */
    move_list = piece_moves(KNIGHT, move_list, pos, target);
    move_list = piece_moves(BISHOP, move_list, pos, target);
    move_list = piece_moves(  ROOK, move_list, pos, target);
    move_list = piece_moves( QUEEN, move_list, pos, target);
  }
  return move_list;
}
