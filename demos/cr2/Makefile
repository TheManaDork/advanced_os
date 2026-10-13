# Object files and target names
obj-m += cr2_probe.o
KDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

# User-space application target
APP = trigger

all: module app

# Build the kernel module using Kbuild
module:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

# Build the user-space application
app: $(APP).c
	gcc -O2 -Wall -Wextra $(APP).c -o $(APP)

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -f $(APP)
