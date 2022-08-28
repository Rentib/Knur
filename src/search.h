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

#ifndef KNUR_SEARCH_H_
#define KNUR_SEARCH_H_

#include "position.h"

/**< \typedef SearchInfo
 * Defines structure SearchInfo.
 */
typedef struct SearchInfo SearchInfo;

/**< \struct SearchInfo
 * A structure for storing parameters used in search defined by a user.
 */
struct SearchInfo {
  /**{*/
  int quit;       /**< Flag for quitting program; */
  int depth;      /**< Depth of search; */
  uint64_t nodes; /** Nodes visited during search. */
  /**}*/
};

extern SearchInfo info;

void search(Position *pos);

#endif /* KNUR_SEARCH_H_ */
