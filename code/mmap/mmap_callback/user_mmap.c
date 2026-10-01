#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#define PAGE_SIZE 4096

int main() {
    int fd = open("/dev/mmap_example", O_RDWR);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    char *ptr = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    // Write something to the kernel buffer
    snprintf(ptr, PAGE_SIZE, "Hello from user-space!\n");

    // Read it back
    printf("%s", ptr);

    munmap(ptr, PAGE_SIZE);
    close(fd);
    return 0;
}
