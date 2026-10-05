/*
 * victim_secret.c
 *
 * Demo 3 -- Secret-Dependent Page-Cache Side Channel
 *
 * The victim contains a secret value from 0 to 7.
 *
 * The secret determines which page of ONE file is accessed.
 *
 * The attacker does not communicate with the victim.
 * The attacker does not read the victim's memory.
 *
 * The attacker can only observe page-cache residency.
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#define PAGE_SIZE       4096
#define NUM_VALUES      8

/*
 * Candidate pages are separated by 1 MB.
 *
 * This is intentional.
 *
 * If candidate pages were adjacent, Linux filesystem
 * readahead could bring neighboring pages into the
 * page cache as well.
 */
#define PAGE_STRIDE     (1024 * 1024)


/*
 * The victim's secret.
 *
 * The attacker does NOT know this value.
 *
 * Change this value between experiments.
 */
static int secret = 5;


int main(void)
{
    printf("Victim: processing secret...\n");

    if (secret < 0 || secret >= NUM_VALUES) {
        fprintf(stderr, "Invalid secret.\n");
        return 1;
    }

    int fd = open("lookup_table.bin", O_RDONLY);

    if (fd < 0) {
        perror("lookup_table.bin");
        return 1;
    }

    /*
     * The secret determines which part of the file
     * the victim accesses.
     */
    off_t offset = (off_t)secret * PAGE_STRIDE;

    /*
     * mmap() creates a virtual-memory mapping.
     *
     * It does not necessarily load the file page yet.
     *
     * The actual memory access below is what causes
     * the file-backed page to be brought into memory
     * and associated with the page cache.
     */
    unsigned char *mapping = mmap(
        NULL,
        PAGE_SIZE,
        PROT_READ,
        MAP_PRIVATE,
        fd,
        offset
    );

    if (mapping == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }

    printf("Victim: performing secret-dependent lookup.\n");

    /*
     * This is the important operation.
     *
     * The victim touches exactly one candidate page.
     *
     * The attacker does not know which page was touched.
     */
    volatile unsigned char value = mapping[0];

    (void)value;

    munmap(mapping, PAGE_SIZE);
    close(fd);

    printf("Victim: processing complete.\n");

    return 0;
}
