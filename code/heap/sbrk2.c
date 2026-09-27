#include <unistd.h>
#include <stdio.h>

int main(void){

    void *currentBreak = sbrk(0x5);
    printf("First increment of 0x5: %p\n",currentBreak);

    currentBreak = sbrk(0x5);
    printf("Second increment of 0x5: %p\n",currentBreak);

    currentBreak = sbrk(0x5);
    printf("Third increment of 0x5: %p\n",currentBreak);

    currentBreak = sbrk(0x5);
    printf("Fourth increment of 0x5: %p\n",currentBreak);

    currentBreak = sbrk(0x5);
    printf("Fifth increment of 0x5: %p\n",currentBreak);

}
