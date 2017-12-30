
# See https://stackoverflow.com/a/14777895/282725 for details on OS detection
ifeq ($(OS),Windows_NT)
    detected_OS := Windows
else
    detected_OS := $(shell uname -s)
endif

BINS=ffasm ffsim ffdbg
INCLUDES=common.h simulator.h

CFLAGS=-g -rdynamic
ifeq ($(detected_OS),Darwin)  # Mac OS X
	CC=gcc-7
	CFLAGS=
endif


all: $(BINS) ff.fo

ff.fo: ff.fa ffasm
	./ffasm ff

ffasm: ffasm.c opcodes.c $(INCLUDES)

ffsim: ffsim.c simulator.c $(INCLUDES)

ffdbg: ffdbg.c simulator.c $(INCLUDES)

debug:
	gdb --args ffasm ff.fa ff.fo

clean:
	rm -f $(BINS) *.o ff.fo

