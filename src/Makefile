VERSION = 0.1

CFLAGS  = -std=c2x -Wall -Wextra -Werror -pedantic -pthread \
		  -DVERSION=\"$(VERSION)\"
LDFLAGS = -pthread
CC      = clang

REQ =

all: CFLAGS += -O3 -ffast-math
all: CFLAGS += -march=native -m64 -mpopcnt
all: knur

debug: CFLAGS += -O0 -ggdb
debug: knur

debug-memory: CFLAGS  += -fsanitize=address
debug-memory: LDFLAGS += -fsanitize=address
debug-memory: debug

debug-thread: CFLAGS  += -fsanitize=thread
debug-thread: LDFLAGS += -fsanitize=thread
debug-thread: debug

knur.o: knur.c knur.h $(REQ:=.h)

.c.o:
	$(CC) -o $@ -c $(CFLAGS) $<

knur: knur.o $(REQ:=.o)
	$(CC) -o $@ $(REQ:=.o) knur.o $(LDFLAGS)

clean:
	rm -f knur knur.o $(REQ:=.o)
