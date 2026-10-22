#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>

// Must be page-aligned (lower 12 bits are 0)
#define BASE_ADDR ((void *)0x500000000000)

int main(void) {
    size_t page_size = 4096;
    
    // 1. mmap must use a page-aligned address
    char *addr = mmap(BASE_ADDR, page_size, PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0);
    if (addr == MAP_FAILED) {
        perror("mmap failed (address might be in use)");
        return 1;
    }

    printf("Successfully mapped exact address at: %p\n", (void *)addr);

    char *target_addr = addr + 0x41;
    printf("Triggering page fault at offset address: %p\n", (void *)target_addr);

    // 3. Reading this triggers the demand-paging fault at 0x500000000041
    volatile char val = *target_addr;

    printf("Successfully read value: %d (Page is now mapped)\n", val);
    
    munmap(addr, page_size);
    return 0;
}
