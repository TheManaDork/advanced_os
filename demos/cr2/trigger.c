#include <stdio.h>
#include <stdlib.h>

int main(void) {
    volatile int *target_addr = (int *)0xdeadbeef4200;
    
    printf("Triggering page fault at address: %p\n", (void *)target_addr);
    
    // This read will trigger a page fault because the address is unmapped
    int val = *target_addr;
    
    printf("Value read: %d\n", val);
    return 0;
}
