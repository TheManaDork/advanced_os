#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

int main(void)
{
    int fd;
    void *p;

    fd = open("data.bin", O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }
    p = mmap(NULL, 4096, PROT_READ, MAP_PRIVATE, fd, 0);
    if (p == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }
    printf("PID: %d\n", getpid());
    printf("Virtual address: %p\n", p);
    printf("Contents: %.30s\n", (char *)p);
    printf("Press ENTER to exit...\n");
    getchar();
    munmap(p, 4096);
    close(fd);
    return 0;
}
