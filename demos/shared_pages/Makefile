CC = gcc
CFLAGS = -Wall -Wextra -O0

TARGET = mapfile
SOURCE = mapfile.c
DATA = data.bin

all: $(TARGET)

$(TARGET): $(SOURCE) $(DATA)
	$(CC) $(CFLAGS) -o $@ $(SOURCE)

$(DATA):
	truncate -s 4096 $@
	printf "Hello from the shared file!\n" > $@

clean:
	rm -f $(TARGET) $(DATA)

.PHONY: all clean
