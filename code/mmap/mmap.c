#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

// create the data.txt first before running this program
// echo hello > data.txt

int main(){
    int length = 4096;
    int fd = open("data.txt", O_RDONLY);
    char *map = mmap(NULL, length, PROT_READ,
                 MAP_PRIVATE, fd, 0);
    printf("First char: %c\n", map[0]);
    munmap(map, length);
    close(fd);
}
