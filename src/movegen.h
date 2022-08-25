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

#ifndef KNUR_MOVEGEN_H_
#define KNUR_MOVEGEN_H_

#include "knur.h"
#include "position.h"

typedef enum {
  GT_ALL,
  GT_CAPTURES,
} GenType;

Move *generate_moves(GenType gt, Move *move_list, Position *pos);

#endif /* KNUR_MOVEGEN_H_ */
