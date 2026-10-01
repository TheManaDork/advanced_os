CC = gcc
CFLAGS = -Wall -Wextra -O2

TARGETS = mmap read

all: $(TARGETS)

mmap: mmap.c
	$(CC) $(CFLAGS) -o $@ $<

read: read.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS)
