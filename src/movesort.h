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

#ifndef KNUR_MOVESORT_H_
#define KNUR_MOVESORT_H_

#include "knur.h"
#include "position.h"

/** \brief Excludes illegal moves and gives the rest values.
 * \param[in,out] begin    - pointer to move array's begin;
 * \param[in,out] end      - pointer to move array's end;
 * \param[in,out] hashmove - https://www.chessprogramming.org/Hash_Move
 * \param[in,out] pos      - pointer to a structure containing a position.
 * \return Pointer to end of move array.
 */
Move *process_moves(Move *begin, Move *end, Move hashmove, const Position *pos);

/** \brief Sort moves by their values.
 * \param[in,out] begin - pointer to move array's begin;
 * \param[in,out] end   - pointer to move array's end.
 */
void sort_moves(Move *begin, Move *end);

#endif /* KNUR_MOVESORT_H_ */
