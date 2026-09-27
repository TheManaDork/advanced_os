#include <unistd.h>
#include <stdio.h>

int main(void){
    int *currentBreak = sbrk(0);
    printf("%p\n", currentBreak);
}
