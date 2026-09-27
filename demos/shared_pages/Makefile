CC = gcc
CFLAGS = -Wall -Wextra -O0

obj-m += pagecache_probe.o

TARGET = mapfile
PFN_TARGET = check_pfn
KMOD = pagecache_probe.ko

SOURCE = mapfile.c
PFN_SOURCE = check_pfn.c
KMOD_SOURCE = pagecache_probe.c
DATA = data.bin

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all: $(TARGET) $(PFN_TARGET) $(KMOD) $(DATA)

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -o $@ $<

$(PFN_TARGET): $(PFN_SOURCE)
	$(CC) $(CFLAGS) -o $@ $<

$(KMOD): $(KMOD_SOURCE)
	$(MAKE) -C $(KDIR) M=$(PWD) modules

$(DATA):
	printf "Hello from the shared file!\n" > $@
	truncate -s 4096 $@

clean:
	rm -f $(TARGET) $(PFN_TARGET) $(DATA)
	$(MAKE) -C $(KDIR) M=$(PWD) clean

.PHONY: all clean
