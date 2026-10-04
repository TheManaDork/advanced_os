CC = gcc
CFLAGS = -Wall -Wextra -O2

PROGRAMS = \
	open_rd \
	open_rw \
	mmap_rd \
	mmap_shared_rw \
	mmap_private_rw \
	mmap_private_madvise \
	mmap_stream \
	mmap_example \
	mmap_lazy \
	read_into_buffer \
	dirty_cow

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

mmap_stream: mmap_stream.c
	$(CC) $(CFLAGS) -o $@ $<

mmap_example: mmap_example.c
	$(CC) $(CFLAGS) -o $@ $<

mmap_lazy: mmap_lazy.c
	$(CC) $(CFLAGS) -o $@ $<

read_into_buffer: read_into_buffer.c
	$(CC) $(CFLAGS) -o $@ $<

dirty_cow: dirty_cow.c
	$(CC) $(CFLAGS) -pthread -o $@ $<

clean:
	rm -f $(PROGRAMS)
