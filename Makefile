CC=gcc
CFLAGS=-g -pthread
BINS=server
OBJS = server.o queue.o

all: $(BINS)

server: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

clean:
	rm -rvf *.dSYM $(BINS)
