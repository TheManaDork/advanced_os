#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
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

    printf("File size: %ld MB\n",
           st.st_size / (1024 * 1024));

    printf("Before allocation:\n");
    print_rss();

    /*
     * Allocate enough memory to hold the entire file.
     */
    char *buffer = malloc(st.st_size);

    if (buffer == NULL) {
        perror("malloc");
        close(fd);
        return 1;
    }

    printf("\nAfter malloc:\n");
    print_rss();

    /*
     * read() copies the entire file into the buffer.
     */
    ssize_t total = 0;

    while (total < st.st_size) {
        ssize_t n = read(fd, buffer + total,
                         st.st_size - total);

        if (n <= 0) {
            perror("read");
            free(buffer);
            close(fd);
            return 1;
        }

        total += n;
    }

    printf("\nAfter reading the entire file:\n");
    print_rss();

    /*
     * Prevent the compiler from considering the buffer unused.
     */
    printf("First byte: %c\n", buffer[0]);

    free(buffer);
    close(fd);

    return 0;
}
