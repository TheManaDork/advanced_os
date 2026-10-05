CC = gcc
CFLAGS = -O2 -Wall -Wextra -march=native

obj-m += meltdown_kernel.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)

USER_PROGRAMS = cache_time flush_reload page_cache_mincore sender receiver

CHANNEL_FILES = bit0.bin bit1.bin bit2.bin bit3.bin \
                bit4.bin bit5.bin bit6.bin bit7.bin

all: $(USER_PROGRAMS) channel_files kernel

cache_time: cache_time.c
	$(CC) $(CFLAGS) -o $@ $<

flush_reload: flush_reload.c
	$(CC) $(CFLAGS) -o $@ $<

page_cache_mincore: page_cache_mincore.c
	$(CC) $(CFLAGS) -o $@ $<

sender: sender.c
	$(CC) $(CFLAGS) -o $@ $<

receiver: receiver.c
	$(CC) $(CFLAGS) -o $@ $<

channel_files:
	for i in 0 1 2 3 4 5 6 7; do \
		dd if=/dev/zero of=bit$$i.bin bs=4096 count=1 status=none; \
	done

kernel:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	rm -f $(USER_PROGRAMS) $(CHANNEL_FILES)
	$(MAKE) -C $(KDIR) M=$(PWD) clean

.PHONY: all kernel clean channel_files
