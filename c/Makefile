CC = gcc
DBGOPT = -Og -g --sanitize=undefined,address -lm -lasan -lubsan -Wall -Wpedantic -std=gnu17
RELOPT = -DNDEBUG -Os -lm -flto -ffast-math -std=gnu17
SOURCE = $(wildcard *.c)
LIBSRC = $(wildcard lib/*.c)
HEADER = $(wildcard *.h)
LIBHDR = $(wildcard lib/*.h)
DBGEXE = $(SOURCE:.c=.dbg)
RELEXE = $(SOURCE:.c=.rel)
DBGOBJ = $(LIBSRC:.c=.dbg.o)
RELOBJ = $(LIBSRC:.c=.rel.o)

all: $(DBGOBJ) $(RELOBJ) $(DBGEXE) $(RELEXE)

format: $(SOURCE) $(HEADER) $(LIBSRC) $(LIBHDR)
	clang-format -i $^

lib/%.dbg.o: lib/%.c
	$(CC) $(DBGOPT) -o $@ -c $^

lib/%.rel.o: lib/%.c
	$(CC) $(RELOPT) -o $@ -c $^

%.dbg: %.c
	$(CC) $(DBGOPT) -o $@ $(DBGOBJ) $^

%.rel: %.c
	$(CC) $(RELOPT) -o $@ $(RELOBJ) $^

.PHONY: clean

clean:
	rm -f *.dbg *.rel *.o
	rm -f lib/*.o
	rm -rf *.dSYM
