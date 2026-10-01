#include <stdio.h>
#include <unistd.h>

// Question: what is wrong with this program?
int main(){
    void *first = sbrk(0);
    void *second = sbrk(4096);
    void *third = sbrk(0);

    int *ptr = (int *)third+1;
    *ptr = 0xDEAD;

    printf("First: %p\n", first);
    printf("Second: %p\n", second);
    printf("Third: %p\n", third);
}
