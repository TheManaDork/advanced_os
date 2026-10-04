#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

int main(void)
{
    int f = open("/etc/passwd", O_RDONLY);

    if (f == -1) {
        perror("open");
        return 1;
    }

    struct stat st;
    if (fstat(f, &st) == -1) {
        perror("fstat");
        return 1;
    }

    char *map = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, f, 0);

    if (map == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    printf("First 20 bytes: %.20s\n", map);

    munmap(map, st.st_size);
    close(f);

    return 0;
}
