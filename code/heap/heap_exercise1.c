#include <malloc.h>
#include <stdio.h>

typedef struct my_struct {
	void (*func)();
} my_struct;

void win(){
	printf("You won!\n");
}

void lose(){
	printf("You lost!\n");
}

// add one line to this main function such that this program prints "You won!\n" and nothing else.
// hard requirement: your one line should contain no more than 10 characters.
// set p1->func to win? That's already more than 10 characters.
int main(void){
	my_struct *p1 = (my_struct *)malloc(sizeof(my_struct));
	p1->func = lose;
	long *p2 = (long *)malloc(sizeof(my_struct));
	*p2 = (long)win;
	p1->func();
        return 0;
}
