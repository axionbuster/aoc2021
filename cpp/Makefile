CPP = g++
DBGOPT = -O0 -g --sanitize=undefined,address -lm -lasan -lubsan -Wall -Wpedantic --std=gnu++17
RELOPT = -DNDEBUG -Os -lm -flto -ffast-math --std=gnu++17

SOURCE = $(wildcard *.cpp)
HEADER = $(wildcard *.h)

DBGEXE = $(SOURCE:.cpp=.dbg)
RELEXE = $(SOURCE:.cpp=.rel)

all: $(DBGEXE) $(RELEXE)

%.dbg: %.cpp
	$(CPP) $(DBGOPT) $^ -o $@

%.rel: %.cpp
	$(CPP) $(RELOPT) $^ -o $@

format: $(SOURCE) $(HEADER)
	clang-format -i $^

clean:
	rm -rf *.dSYM
	rm -rf *.dbg
	rm -rf *.rel
	rm -rf *.o
