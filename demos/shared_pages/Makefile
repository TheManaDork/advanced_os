CC = gcc
CFLAGS = -Wall -Wextra -O0

TARGET = mapfile
PFN_TARGET = check_pfn

SOURCE = mapfile.c
PFN_SOURCE = check_pfn.c
DATA = data.bin

all: $(TARGET) $(PFN_TARGET)

$(TARGET): $(SOURCE) $(DATA)
	$(CC) $(CFLAGS) -o $@ $(SOURCE)

$(PFN_TARGET): $(PFN_SOURCE)
	$(CC) $(CFLAGS) -o $@ $(PFN_SOURCE)

$(DATA):
	printf "Hello from the shared file!\n" > $@
	truncate -s 4096 $@

clean:
	rm -f $(TARGET) $(PFN_TARGET) $(DATA)

.PHONY: all clean
