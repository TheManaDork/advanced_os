# Module name
obj-m += reboot_guard.o

# Target user-space application
USER_APP := reboot_auth

# Path to kernel build directory
KDIR ?= /lib/modules/$(shell uname -r)/build

# Default target: builds both kernel module and user-space app
all: $(USER_APP)
	make -C $(KDIR) M=$(PWD) modules

# Rule to compile the user-space application
$(USER_APP): reboot_auth.c
	$(CC) -Wall -O2 $< -o $@

# Clean target: cleans kernel module artifacts and user-space binary
clean:
	make -C $(KDIR) M=$(PWD) clean
	rm -f $(USER_APP)

.PHONY: all clean
