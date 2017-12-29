
# See https://stackoverflow.com/a/14777895/282725 for details on OS detection
ifeq ($(OS),Windows_NT)
    detected_OS := Windows
else
    detected_OS := $(shell uname -s)
endif

BINS=ffasm ffdasm ffsim ffdbg

CFLAGS=-g -rdynamic
ifeq ($(detected_OS),Darwin)  # Mac OS X
	CC=gcc-7
endif


all: $(BINS) ff.fo

ff.fo: ff.fa ffasm
	./ffasm ff.fa ff.fo

ffasm: ffasm.c

ffdasm: ffdasm.c

ffsim: ffsim.c

ffdbg: ffdbg.c

debug:
	gdb --args ffasm ff.fa ff.fo

clean:
	rm -f $(BINS) *.o ff.fo

