CC = gcc
CFLAGS = -Wall -Wextra -O2

PROGRAMS = open_rd open_rw mmap_rd mmap_shared_rw mmap_private_rw mmap_private_madvise

all: $(PROGRAMS)

open_rd: open_rd.c
	$(CC) $(CFLAGS) -o $@ $<

open_rw: open_rw.c
	$(CC) $(CFLAGS) -o $@ $<

mmap_rd: mmap_rd.c
	$(CC) $(CFLAGS) -o $@ $<

mmap_shared_rw: mmap_shared_rw.c
	$(CC) $(CFLAGS) -o $@ $<

mmap_private_rw: mmap_private_rw.c
	$(CC) $(CFLAGS) -o $@ $<

mmap_private_madvise: mmap_private_madvise.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(PROGRAMS)
