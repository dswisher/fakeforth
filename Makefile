
# See https://stackoverflow.com/a/14777895/282725 for details on OS detection
ifeq ($(OS),Windows_NT)
    detected_OS := Windows
else
    detected_OS := $(shell uname -s)
endif

BINS=ffasm ffsim ffdbg
INCLUDES=common.h simulator.h opcodes.h

CFLAGS=-g -rdynamic -Wall -DUSE_READLINE
LDLIBS=-lreadline
ifeq ($(detected_OS),Darwin)  # Mac OS X
	CC=gcc-7
	CFLAGS=
	LDLIBS=
endif


all: $(BINS) ff.fo

ff.fo: ff.fa ffasm
	./ffasm ff

ffasm: ffasm.o opcodes.o

ffsim: ffsim.o simulator.o opcodes.o

ffdbg: ffdbg.o simulator.o opcodes.o

ffasm.c: $(INCLUDES)

ffsim.c: $(INCLUDES)

ffdbg.c: $(INCLUDES)

debug:
	gdb --args ffasm ff.fa ff.fo

clean:
	rm -f $(BINS) *.o ff.fo ff.sym

