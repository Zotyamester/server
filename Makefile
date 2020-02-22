CC=gcc
CFLAGS=-g
BINS=server

all: $(BINS)

%: %.CC
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -rvf *.dSYM $(BINS)
