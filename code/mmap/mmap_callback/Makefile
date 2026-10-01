# Build against the running kernel's build directory
KDIR ?= /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)

all: mmap_example user_mmap

obj-m += mmap_example.o

mmap_example:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

user-mmap: user-mmap.o
	$(CC) $(CFLAGS) -o $@ $^

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

