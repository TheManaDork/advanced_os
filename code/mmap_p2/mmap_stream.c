/*
 * mmap_stream.c
 *
 * Process a large file one chunk at a time and observe the
 * process's resident memory (RSS).
 *
 * To create a large file:
 *
 * dd if=/dev/zero of=large_file bs=1M count=500
 *
 * Usage:
 *
 *     ./mmap_stream <file> <use_madvise>
 *
 * use_madvise = 0:
 *     Do not call MADV_DONTNEED.
 *
 * use_madvise = 1:
 *     Call MADV_DONTNEED after processing each chunk.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define CHUNK_SIZE (16 * 1024 * 1024)

/*
 * Print only the process's current resident memory (RSS: resident set size).
 *
 * VmRSS is the amount of the process's virtual memory that is
 * currently resident in physical memory.
 */
void print_rss(void)
{
    FILE *f = fopen("/proc/self/status", "r");

    if (f == NULL) {
        perror("fopen");
        return;
    }

    char line[256];

    while (fgets(line, sizeof(line), f) != NULL) {

        if (strncmp(line, "VmRSS:", 6) == 0) {
            printf("%s", line);
            break;
        }
    }

    fclose(f);
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <file> <use_madvise>\n", argv[0]);
        return 1;
    }

    int use_madvise = atoi(argv[2]);

    int f = open(argv[1], O_RDONLY);

    if (f == -1) {
        perror("open");
        return 1;
    }

    struct stat st;

    if (fstat(f, &st) == -1) {
        perror("fstat");
        close(f);
        return 1;
    }

    if (st.st_size == 0) {
        printf("File is empty.\n");
        close(f);
        return 0;
    }

    /*
     * Map the entire file.
     *
     * mmap() itself does not load the entire file into physical
     * memory. Pages are brought into memory when they are accessed.
     */
    char *map = mmap(NULL, st.st_size,
                     PROT_READ,
                     MAP_PRIVATE,
                     f, 0);

    if (map == MAP_FAILED) {
        perror("mmap");
        close(f);
        return 1;
    }

    printf("Initial RSS: ");
    print_rss();

    /*
     * Process the file one chunk at a time.
     */
    for (off_t offset = 0; offset < st.st_size; offset += CHUNK_SIZE) {

        off_t remaining = st.st_size - offset;

        size_t length = remaining < CHUNK_SIZE
                      ? (size_t) remaining
                      : CHUNK_SIZE;

        /*
         * Touch every page in this chunk.
         *
         * This causes the file-backed pages to become resident
         * in physical memory.
         */
        for (size_t i = 0; i < length; i += 4096) {
            volatile char value = map[offset + i];
            (void) value;
        }

        printf("After reading chunk: ");
        print_rss();

        if (use_madvise) {

            /*
             * We are finished processing this chunk.
             *
             * Tell the kernel that we do not expect to access
             * these pages again soon.
             *
             * The kernel can reclaim the physical pages associated
             * with this range.
             */
            madvise(map + offset, length, MADV_DONTNEED);

            printf("After MADV_DONTNEED: ");
            print_rss();
        }
    }

    munmap(map, st.st_size);
    close(f);

    return 0;
}
