# Knur, a chess engine.
# Copyright (C) 2022 Stanisław Bitner <sbitner420@tutanota.com>
#
# Knur is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Knur is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

CFLAGS = -std=c99 -Wall -Wextra -pedantic -Wno-deprecated-declarations -Wno-implicit-fallthrough -Ofast -g
LDFLAGS = 
CC = cc

REQ = bitboards evaluate movegen movesort perft position search uci util

all: options main

options:
	@echo Knur build options
	@echo "CFLAGS  = $(CFLAGS)"
	@echo "LDFLAGS = $(LDFLAGS)"
	@echo "CC      = $(CC)"

main.o: main.c $(REQ:=.h)

.c.o:
	$(CC) -o $@ -c $(CFLAGS) $<

main: main.o $(REQ:=.o)
	$(CC) -o $@ $(REQ:=.o) main.o

clean:
	rm -f main main.o $(REQ:=.o)

install: all
	cp -f main ~/.local/bin/chess_engines/knur
