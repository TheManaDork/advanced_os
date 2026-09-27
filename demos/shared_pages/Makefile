CC = gcc
CFLAGS = -Wall -Wextra -O0

TARGET = mapfile
PFN_TARGET = check_pfn
KMOD = pagecache_probe.ko

SOURCE = mapfile.c
PFN_SOURCE = check_pfn.c
DATA = data.bin

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all: $(TARGET) $(PFN_TARGET) $(KMOD)

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -o $@ $<

$(PFN_TARGET): $(PFN_SOURCE)
	$(CC) $(CFLAGS) -o $@ $<

$(DATA):
	printf "Hello from the shared file!\n" > $@
	truncate -s 4096 $@

$(KMOD): pagecache_probe.c
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	rm -f $(TARGET) $(PFN_TARGET) $(DATA)
	$(MAKE) -C $(KDIR) M=$(PWD) clean

.PHONY: all clean
