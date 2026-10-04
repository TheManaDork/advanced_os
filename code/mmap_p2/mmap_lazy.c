#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

static void print_rss(void)
{
    FILE *f = fopen("/proc/self/status", "r");
    char line[256];
    long rss;

    if (f == NULL) {
        perror("fopen");
        return;
    }

    while (fgets(line, sizeof(line), f) != NULL) {
        if (sscanf(line, "VmRSS: %ld kB", &rss) == 1) {
            printf("VmRSS: %ld kB\n", rss);
            break;
        }
    }

    fclose(f);
}

int main(void)
{
    int fd = open("large_file.bin", O_RDONLY);

    if (fd == -1) {
        perror("open");
        return 1;
    }

    struct stat st;

    if (fstat(fd, &st) == -1) {
        perror("fstat");
        close(fd);
        return 1;
    }

    printf("File size: %ld MB\n", st.st_size / (1024 * 1024));

    printf("Before mmap:\n");
    print_rss();

    char *map = mmap(NULL, st.st_size,
                     PROT_READ,
                     MAP_PRIVATE,
                     fd, 0);

    if (map == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }

    printf("\nAfter mmap:\n");
    print_rss();

    printf("\nAccessing the first byte...\n");
    volatile char x = map[0];
    (void)x;

    printf("After accessing one byte:\n");
    print_rss();

    munmap(map, st.st_size);
    close(fd);

    return 0;
}
