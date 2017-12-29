
BINS=ffasm ffdasm ffsim ffdbg

all: $(BINS)

ffasm: ffasm.c

ffdasm: ffdasm.c

ffsim: ffsim.c

ffdbg: ffdbg.c

clean:
	rm -f $(BINS) *.o

