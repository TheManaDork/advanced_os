CC = gcc
CFLAGS = -O2 -Wall -Wextra -march=native

obj-m += meltdown_kernel.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)

USER_PROGRAMS = cache_time flush_reload

all: $(USER_PROGRAMS) kernel

cache_time: cache_time.c
	$(CC) $(CFLAGS) -o $@ $<

flush_reload: flush_reload.c
	$(CC) $(CFLAGS) -o $@ $<

kernel:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	rm -f $(USER_PROGRAMS)
	$(MAKE) -C $(KDIR) M=$(PWD) clean

.PHONY: all kernel clean
