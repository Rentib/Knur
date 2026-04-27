VERSION = 2.0

CFLAGS   = -std=c2x -Wall -Wextra -Werror -pedantic -pthread \
		   -DVERSION=\"$(VERSION)\"
LDFLAGS  = -pthread
CC       = clang
EXE      = knur$(VERSION)
EVALFILE = net.nnue

ifdef EVALFILE
	CFLAGS += -DUSE_NNUE=1 -DEVALFILE=\"$(EVALFILE)\"
else
	CFLAGS += -DUSE_NNUE=0
endif

REQ = bitboards evaluate movegen movepicker perft position search transposition \
	  uci util nn/nnue

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
