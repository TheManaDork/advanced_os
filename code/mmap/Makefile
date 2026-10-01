CC = gcc
CFLAGS = -Wall -Wextra -O2

TARGETS = mmap read mmap-large read-large

all: $(TARGETS)

mmap: mmap.c
	$(CC) $(CFLAGS) -o $@ $<

read: read.c
	$(CC) $(CFLAGS) -o $@ $<

mmap-large: mmap-large.c
	$(CC) $(CFLAGS) -o $@ $<

read-large: read-large.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS)
