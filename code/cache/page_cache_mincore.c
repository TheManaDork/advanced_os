/*
 * page_cache_mincore.c
 *
 * Demonstrate that file-backed pages can become resident
 * in the Linux page cache after they are accessed.
 *
 * Experiment:
 *
 *     mmap()
 *        |
 *        v
 *     mincore()       <-- Is the page resident?
 *        |
 *        v
 *     access page
 *        |
 *        v
 *     mincore()       <-- Is the page resident now?
 *
 * Compile:
 *
 *     gcc -Wall -Wextra -O2 -o page_cache_mincore page_cache_mincore.c
 *
 * Run:
 *
 *     ./page_cache_mincore large_file.bin
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>

#define PAGE_SIZE 4096


/*
 * Print whether a particular file page is currently resident
 * in the page cache.
 *
 * mincore() does NOT tell us whether the page is present in
 * this process's virtual memory.
 *
 * For a file-backed mapping, it tells us whether the
 * corresponding page is currently resident in physical memory
 * through the operating system's page cache.
 */
static void check_page_residency(void *mapping, size_t page_number)
{
    unsigned char vec = 0;

    void *page_address =
        (char *)mapping + page_number * PAGE_SIZE;

    /*
     * Ask the kernel about the residency of one page.
     *
     * The address must be page-aligned.
     */
    if (mincore(page_address, PAGE_SIZE, &vec) != 0) {
        perror("mincore");
        exit(EXIT_FAILURE);
    }

    if (vec & 1)
        printf("Page %zu: RESIDENT in page cache\n", page_number);
    else
        printf("Page %zu: NOT resident in page cache\n", page_number);
}


int main(int argc, char *argv[])
{
    int fd;
    struct stat st;
    void *mapping;

    /*
     * We need a filename supplied by the user.
     */
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    /*
     * Open the file read-only.
     */
    fd = open(argv[1], O_RDONLY);

    if (fd < 0) {
        perror("open");
        return EXIT_FAILURE;
    }

    /*
     * Determine the file size.
     */
    if (fstat(fd, &st) != 0) {
        perror("fstat");
        close(fd);
        return EXIT_FAILURE;
    }

    if (st.st_size < PAGE_SIZE) {
        fprintf(stderr, "File must be at least one page (4096 bytes).\n");
        close(fd);
        return EXIT_FAILURE;
    }

    /*
     * Map the file into our virtual address space.
     *
     * IMPORTANT:
     *
     * mmap() itself does not necessarily read the file into
     * physical memory.
     *
     * It primarily establishes the virtual-memory mapping.
     */
    mapping = mmap(NULL,
                   st.st_size,
                   PROT_READ,
                   MAP_PRIVATE,
                   fd,
                   0);

    if (mapping == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return EXIT_FAILURE;
    }

    close(fd);

    printf("File size: %lld bytes\n",
           (long long)st.st_size);

    printf("Mapped address: %p\n\n", mapping);


    /*
     * ---------------------------------------------------------
     * Step 1: Check page-cache residency BEFORE accessing it.
     * ---------------------------------------------------------
     */
    printf("=== BEFORE ACCESS ===\n");

    check_page_residency(mapping, 0);


    /*
     * ---------------------------------------------------------
     * Step 2: Access the first byte of the mapped file.
     * ---------------------------------------------------------
     *
     * This access may cause a page fault.
     *
     * The kernel will bring the corresponding file page into
     * memory, normally through the page cache.
     */
    printf("\nAccessing the first byte...\n");

    volatile unsigned char value =
        *((unsigned char *)mapping);

    printf("First byte = %u ('%c')\n",
           value,
           (value >= 32 && value <= 126) ? value : '.');


    /*
     * ---------------------------------------------------------
     * Step 3: Check residency AFTER the access.
     * ---------------------------------------------------------
     */
    printf("\n=== AFTER ACCESS ===\n");

    check_page_residency(mapping, 0);


    /*
     * Clean up the mapping.
     */
    if (munmap(mapping, st.st_size) != 0) {
        perror("munmap");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
