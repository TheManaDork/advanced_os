// To create the large file:
// base64 /dev/urandom | head -c 200M > large_file.txt

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s filename\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];

    // fopen() returns a FILE* stream used by fread(), fseek(), etc.
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        perror("fopen");
        return 1;
    }

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    // Move the file position to the end of the file.
    //
    // SEEK_END means "relative to the end of the file".
    // The second argument is 0, so we move exactly to the end.
    //
    // We do this so that ftell() can tell us the file size.
    fseek(fp, 0, SEEK_END);

    // ftell() returns the current file position.
    // Since we are at the end, this position is the file size
    // (number of bytes from the beginning of the file).
    size_t filesize = ftell(fp);

    // Move the file position back to the beginning.
    //
    // SEEK_SET means "relative to the beginning of the file".
    // Offset 0 therefore means the first byte.
    //
    // This is necessary because fread() will read starting from
    // the current file position.
    fseek(fp, 0, SEEK_SET);

    // Allocate a user-space buffer large enough to hold the
    // entire file.
    char *buffer = malloc(filesize);
    if (!buffer) {
        perror("malloc");
        fclose(fp);
        return 1;
    }

    // Read the entire file into the user-space buffer.
    //
    // fread() returns the number of bytes actually read.
    // We expect it to equal the file size.
    size_t nread = fread(buffer, 1, filesize, fp);
 
    if (nread != filesize) {
        if (ferror(fp)) {
            perror("fread");
        } else {
            fprintf(stderr, "Unexpected end of file\n");
        }

        free(buffer);
        fclose(fp);
        return 1;
    }

    // Write the data to /dev/null.
    //
    // We do this so that the program actually accesses all of
    // the data without printing 200 MB to the terminal.
    FILE *out = fopen("/dev/null", "wb");
    fwrite(buffer, 1, filesize, out);
    fclose(out);

    free(buffer);
    fclose(fp);

    clock_gettime(CLOCK_MONOTONIC, &end);

    double elapsed = (end.tv_sec - start.tv_sec) +
                     (end.tv_nsec - start.tv_nsec) / 1e9;

    printf("Elapsed time (fread): %.6f seconds\n", elapsed);

    return 0;
}
