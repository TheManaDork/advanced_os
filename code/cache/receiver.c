/*
 * receiver.c
 *
 * Page-Cache Covert Channel
 *
 * This is a completely independent process from sender.c.
 *
 * Usage:
 *
 *     ./receiver
 *
 * For each bit, the receiver opens the corresponding
 * one-page file and uses mincore() to determine whether
 * that page is resident in the page cache.
 *
 *     resident     -> 1
 *     nonresident  -> 0
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#define PAGE_SIZE 4096
#define NUM_BITS  8

int main(void)
{
    char received[NUM_BITS + 1];

    printf("Receiver checking page-cache state...\n\n");

    for (int i = 0; i < NUM_BITS; i++) {

        char filename[64];

        snprintf(filename, sizeof(filename),
                 "bit%d.bin", i);

        /*
         * Open the same file independently.
         *
         * This is NOT the sender's file descriptor.
         */
        int fd = open(filename, O_RDONLY);

        if (fd < 0) {
            perror(filename);
            return 1;
        }

        /*
         * Map one page.
         *
         * mmap() itself does not access the page contents.
         */
        unsigned char *mapping = mmap(
            NULL,
            PAGE_SIZE,
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

        unsigned char vec = 0;

        /*
         * mincore() asks:
         *
         *     "Is this page currently resident?"
         *
         * It does not read the page contents.
         */
        if (mincore(mapping, PAGE_SIZE, &vec) != 0) {
            perror("mincore");
            munmap(mapping, PAGE_SIZE);
            close(fd);
            return 1;
        }

        if (vec & 1) {

            received[i] = '1';

            printf("Bit %d: %s: RESIDENT     -> 1\n",
                   i, filename);

        } else {

            received[i] = '0';

            printf("Bit %d: %s: NONRESIDENT  -> 0\n",
                   i, filename);
        }

        munmap(mapping, PAGE_SIZE);
        close(fd);
    }

    received[NUM_BITS] = '\0';

    printf("\nReceived message: %s\n", received);

    return 0;
}
