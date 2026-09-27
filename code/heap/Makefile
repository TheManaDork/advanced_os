CC = gcc
CFLAGS = -Wall -Wextra -O0

TARGETS = alignedMalloc doubleFree heap_exercise1 heap_exercise2 library sbrk1 sbrk2

all: $(TARGETS)

alignedMalloc: alignedMalloc.c
	$(CC) $(CFLAGS) -o $@ $<

doubleFree: doubleFree.c
	$(CC) $(CFLAGS) -o $@ $<

heap_exercise1: heap_exercise1.c
	$(CC) $(CFLAGS) -o $@ $<

heap_exercise2: heap_exercise2.c
	$(CC) $(CFLAGS) -o $@ $<

library: library.c
	$(CC) $(CFLAGS) -o $@ $<

sbrk1: sbrk1.c
	$(CC) $(CFLAGS) -o $@ $<

sbrk2: sbrk2.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS)

.PHONY: all clean
