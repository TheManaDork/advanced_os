// to create the large file:
// base64 /dev/urandom | head -c 200M > large_file.txt

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s filename\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    struct stat st;
    fstat(fd, &st);
    size_t filesize = st.st_size;

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    char *data = mmap(NULL, filesize, PROT_READ, MAP_PRIVATE, fd, 0);
    if (data == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }

    // Print to /dev/null to avoid terminal slowdown
    FILE *out = fopen("/dev/null", "wb");
    fwrite(data, 1, filesize, out);
    fclose(out);

    munmap(data, filesize);
    close(fd);

    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed = (end.tv_sec - start.tv_sec) +
                     (end.tv_nsec - start.tv_nsec)/1e9;
    printf("Elapsed time (mmap): %.6f seconds\n", elapsed);
    return 0;
}
