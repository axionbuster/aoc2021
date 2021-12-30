CC = gcc
DBGOPT = -g -O0 --sanitize=undefined,address -lm -lasan -lubsan -Wall -Wpedantic
RELOPT = -DNDEBUG -Os -lm -ffast-math
SOURCE = $(wildcard *.c)
HEADER = $(wildcard *.h)
DBGEXE = $(SOURCE:.c=.dbg)
RELEXE = $(SOURCE:.c=.rel)

all: $(DBGEXE) $(RELEXE)

format: $(SOURCE) $(HEADER)
	clang-format -i $^

%.dbg: %.c
	$(CC) $(DBGOPT) -o $@ $^

%.rel: %.c
	$(CC) $(RELOPT) -o $@ $^

.PHONY: clean

clean:
	rm -f *.dbg *.rel
	rm -rf *.dSYM
