/*
 * check_pfn.c
 *
 * Purpose:
 *   This program determines which physical page frame (PFN) a particular
 *   virtual address maps to for a given process.
 *
 * Usage:
 *   ./check_pfn <PID> <virtual-address>
 *
 * Example:
 *   ./check_pfn 3291 0x7f1234567000
 *
 * Inputs:
 *   PID              - The process ID of the process being examined.
 *   virtual-address  - The virtual address printed by the mapfile program.
 *
 * What the program does:
 *   1. Opens /proc/<PID>/pagemap for the specified process.
 *   2. Calculates which pagemap entry corresponds to the virtual address.
 *   3. Reads the 64-bit pagemap entry.
 *   4. Determines whether the page is currently present in physical memory.
 *   5. Extracts the Page Frame Number (PFN), when Linux makes it available.
 *   6. Prints the PFN and the corresponding physical address.
 *
 * Note:
 *   Modern Linux kernels restrict PFN information for unprivileged users.
 *   If the PFN is reported as unavailable, run this program with sudo.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#define PAGE_SIZE 4096ULL

static int get_pfn(pid_t pid, unsigned long long vaddr,
                   unsigned long long *pfn)
{
    char path[64];
    int fd;
    uint64_t entry;
    off_t offset;
    ssize_t n;

    snprintf(path, sizeof(path), "/proc/%d/pagemap", pid);
    fd = open(path, O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "Cannot open %s: %s\n", path, strerror(errno));
        return -1;
    }

    /*
     * Each virtual page has one 64-bit pagemap entry.
     */
    offset = (vaddr / PAGE_SIZE) * sizeof(uint64_t);

    if (lseek(fd, offset, SEEK_SET) == (off_t)-1) {
        fprintf(stderr, "lseek failed: %s\n", strerror(errno));
        close(fd);
        return -1;
    }

    n = read(fd, &entry, sizeof(entry));
    if (n != sizeof(entry)) {
        fprintf(stderr, "Failed to read pagemap entry: %s\n",
                strerror(errno));
        close(fd);
        return -1;
    }
    close(fd);

    /*
     * Bit 63: page present in RAM
     * Bits 0-54: PFN on systems that expose PFNs
     */
    if (!(entry & (1ULL << 63))) {
        printf("Page is not currently present in RAM.\n");
        return 1;
    }

    *pfn = entry & ((1ULL << 55) - 1);

    /*
     * Modern Linux may hide the PFN from unprivileged users.
     */
    if (*pfn == 0) {
        printf("Page is present, but the PFN is unavailable.\n");
        printf("Try running this program with sudo.\n");
        return 2;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    pid_t pid;
    unsigned long long vaddr;
    unsigned long long pfn;
    int result;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <pid> <virtual-address>\n", argv[0]);
        fprintf(stderr, "Example: %s 3291 0x7f1234567000\n", argv[0]);
        return 1;
    }

    pid = (pid_t)strtol(argv[1], NULL, 10);
    vaddr = strtoull(argv[2], NULL, 0);
    result = get_pfn(pid, vaddr, &pfn);

    if (result < 0) {
        return 1;
    }

    if (result == 1 || result == 2) {
        return 1;
    }

    printf("PID:             %d\n", pid);
    printf("Virtual address: 0x%llx\n", vaddr);
    printf("PFN:             %llu\n", pfn);
    printf("Physical address: 0x%llx\n",
           pfn * PAGE_SIZE + (vaddr % PAGE_SIZE));

    return 0;
}
