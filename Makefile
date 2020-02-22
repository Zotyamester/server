CC=gcc
CFLAGS=-g -pthread
BINS=server

all: $(BINS)

%: %.CC
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -rvf *.dSYM $(BINS)
