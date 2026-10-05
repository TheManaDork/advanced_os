/*
 * sender.c
 *
 * Page-Cache Covert Channel
 *
 * Two completely independent processes communicate through
 * Linux page-cache state.
 *
 * Each bit has its own one-page file:
 *
 *     bit0.bin
 *     bit1.bin
 *     bit2.bin
 *     ...
 *     bit7.bin
 *
 * To transmit:
 *
 *     1 -> access the corresponding file/page
 *     0 -> do not access it
 *
 * The receiver later uses mincore() to determine which
 * pages are resident in the page cache.
 *
 * Clear the page cache before we run the attack.
 * $ sudo sh -c 'echo 1 > /proc/sys/vm/drop_caches'
 * $ gcc -Wall -Wextra -O2 -o sender sender.c
 * $ gcc -Wall -Wextra -O2 -o receiver receiver.c
 * $ ./sender 10110100
 * $ ./receiver
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define PAGE_SIZE 4096
#define NUM_BITS  8

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr,
                "Usage: %s <8-bit-message>\n",
                argv[0]);
        return 1;
    }

    const char *message = argv[1];

    if (strlen(message) != NUM_BITS) {
        fprintf(stderr,
                "Error: message must contain exactly 8 bits.\n");
        return 1;
    }

    printf("Sender transmitting: %s\n\n", message);

    for (int i = 0; i < NUM_BITS; i++) {

        char filename[64];

        snprintf(filename, sizeof(filename),
                 "bit%d.bin", i);

        /*
         * Open the one-page file corresponding to this bit.
         */
        int fd = open(filename, O_RDONLY);

        if (fd < 0) {
            perror(filename);
            return 1;
        }

        /*
         * Map exactly one page.
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

        if (message[i] == '1') {

            /*
             * Access the page.
             *
             * This makes the file-backed page resident
             * in the page cache.
             */
            volatile unsigned char value = mapping[0];

            (void)value;

            printf("Bit %d: 1 -> accessed %s\n",
                   i, filename);

        } else {

            /*
             * Do not access the page.
             *
             * If the page started nonresident, it should
             * remain nonresident.
             */
            printf("Bit %d: 0 -> did NOT access %s\n",
                   i, filename);
        }

        munmap(mapping, PAGE_SIZE);
        close(fd);
    }

    printf("\nTransmission complete.\n");

    return 0;
}
