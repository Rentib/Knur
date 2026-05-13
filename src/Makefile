# Knur, a UCI chess engine.
# Copyright (C) 2024-2026 Stanisław Bitner
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

VERSION = 1.14
EXE     = knur-v$(VERSION)

CFLAGS   = -std=c2x -Wall -Wextra -Werror -pedantic -pthread \
		   -DVERSION=\"$(VERSION)\"
LDFLAGS  = -pthread -lm
CC       = clang
EVALFILE = net.nnue

ifdef EVALFILE
	CFLAGS += -DUSE_NNUE=1 -DEVALFILE=\"$(EVALFILE)\"
else
	CFLAGS += -DUSE_NNUE=0
endif

REQ = bitboards evaluate history movegen movepicker nnue perft position search \
      transposition uci util

all: CFLAGS += -O3 -ffast-math
all: CFLAGS += -march=native -m64 -mpopcnt
all: CFLAGS += -mbmi2 -DUSE_PEXT
all: knur

debug: CFLAGS += -O0 -ggdb
debug: knur

debug-memory: CFLAGS  += -fsanitize=address
debug-memory: LDFLAGS += -fsanitize=address
debug-memory: debug

debug-thread: CFLAGS  += -fsanitize=thread
debug-thread: LDFLAGS += -fsanitize=thread
debug-thread: debug

tune: CFLAGS += -DTUNE
tune: CFLAGS += -O3 -ffast-math
tune: CFLAGS += -march=native -m64 -mpopcnt
tune: CFLAGS += -mbmi2 -DUSE_PEXT
tune: CFLAGS += -fopenmp
tune: LDFLAGS += -lm -lomp
tune: tune.o $(REQ:=.o)
	$(CC) -o $@ $(REQ:=.o) tune.o $(LDFLAGS)

knur.o: knur.c knur.h $(REQ:=.h)

.c.o:
	$(CC) -o $@ -c $(CFLAGS) $<

knur: knur.o $(REQ:=.o)
	$(CC) -o $(EXE) $(REQ:=.o) knur.o $(LDFLAGS)

install: all
	cp -f $(EXE) ~/.local/bin/chess_engines/

clean:
	rm -f $(EXE) knur.o tune tune.o $(REQ:=.o)
