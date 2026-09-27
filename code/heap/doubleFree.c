#include <stdlib.h>
#include <stdio.h>

int main() {
    int *p1 = malloc(sizeof(int));
    *p1 = 42;

    free(p1);
    free(p1); // double-free

    int *p2 = malloc(sizeof(int));
    int *p3 = malloc(sizeof(int)); // may return same address as p2

    *p2 = 100;
    printf("Read through p3: %d\n", *p3); // unexpected read/write overlap

    return 0;
}
