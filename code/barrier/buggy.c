/*
 * try these 3 and see the different behaviors:
 * gcc buggy.c -lpthread
 * gcc -O2 buggy.c -lpthread
 * gcc -O3 buggy.c -lpthread
 * */

#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h> // For usleep

// Shared data needs to be accessible globally or passed via a struct
bool flag = false;

// Function for the first thread (the "signaler")
void* thread1_func(void* arg) {
    usleep(100 * 1000); // Sleep for 100 milliseconds
    printf("t1 started\n");
    flag = true;
    printf("t1 signals and exits\n");
    return NULL;
}

// Function for the second thread (the "waiter")
void* thread2_func(void* arg) {
    printf("t2 started\n");
    // This is a "spin-wait" or "busy-wait", which is inefficient and buggy
    while (flag == false) {
        // Wastes CPU cycles doing nothing
    }
    printf("t2 got signaled and exits\n");
    return NULL;
}

int main() {
    pthread_t t1, t2;

    // Create the threads
    pthread_create(&t2, NULL, thread2_func, NULL);
    pthread_create(&t1, NULL, thread1_func, NULL);

    // Wait for the threads to finish
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    return 0; // Returning 0 is standard for success
}
