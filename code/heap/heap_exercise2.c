#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *data;

// fn_t is a type that represents “pointer to a function returning void and taking no arguments.”
typedef void (*fn_t)(void);

void win() {
    printf("You won!\n");
}

void lose() {
    printf("You lost!\n");
}

// complete your function such that this program prints "You won!\n" and nothing else.
void your_fcn(){
}

int main(void) {
    char *x;
    data = malloc(5);
    strncpy(data, "test", 5);
    x = malloc(5);
    *(fn_t *)x = lose;
    your_fcn();
    // this cast tells C to treat the bytes at x as a pointer to a function.
    fn_t f = *(fn_t *)x;
    f();
    return 0;
}
