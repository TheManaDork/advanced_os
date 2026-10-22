// File: fence_demo.c
// gcc -O2 -o fence_demo fence_demo.c -lpthread
#include <stdio.h>
#include <pthread.h>

// Shared variables. 'volatile' prevents the compiler from optimizing
// away our memory accesses, but it does NOT prevent CPU reordering.
volatile int x = 0;
volatile int y = 0;

// Variables to store the results read by each thread.
int result1 = 0;
int result2 = 0;

// --- VERSION 1: BUGGY (No Memory Fence) ---

void* thread1_buggy(void* arg) {
    // Set x to 1. This write goes into this core's store buffer.
    x = 1;
    
    // The CPU is allowed to execute this read *before* the write to 'x'
    // is visible to the other core. This is a store-load reordering.
    result1 = y;
    
    return NULL;
}

void* thread2_buggy(void* arg) {
    // Set y to 1. This write goes into this core's store buffer.
    y = 1;
    
    // This read can also happen before the write to 'y' is globally visible.
    result2 = x;
    
    return NULL;
}

// --- VERSION 2: FIXED (With Memory Fence) ---

void* thread1_fixed(void* arg) {
    x = 1;
    
    // The memory fence. The CPU will not execute any instructions below this
    // line until all writes above it (i.e., x=1) are visible to all cores.
    // The "memory" clobber tells the compiler that memory has been changed.
    __asm__ volatile("mfence" ::: "memory");
    
    result1 = y;
    
    return NULL;
}

void* thread2_fixed(void* arg) {
    y = 1;
    
    // This fence prevents the read of 'x' from happening before
    // the write to 'y' is visible everywhere.
    __asm__ volatile("mfence" ::: "memory");
    
    result2 = x;
    
    return NULL;
}


int main() {
    pthread_t t1, t2;
    long long bad_result_count = 0;
    // WHY 10 MILLION ITERATIONS?
    // Hardware memory reordering (store-load) is a statistically rare event.
    // It depends on exact CPU pipeline timing, cache states, and store buffer loads.
    // It typically takes thousands or millions of runs for the architecture to hit
    // the precise microarchitectural window where the bug manifests.
    //
    // NOTE: Spawning and joining threads 10 million times creates massive kernel overhead,
    // which is why this program appears to hang/freeze for a few minutes with no output.
    long long iterations = 10000000; // 10000000 runs

    printf("### Running test WITHOUT memory fence...\n");
    printf("Looking for incorrect results where result1=0 and result2=0\n");

    for (long long i = 0; i < iterations; i++) {
        // Reset state for the next run
        x = 0;
        y = 0;

        pthread_create(&t1, NULL, thread1_buggy, NULL);
        pthread_create(&t2, NULL, thread2_buggy, NULL);

        pthread_join(t1, NULL);
        pthread_join(t2, NULL);

        // This result should be impossible in a sequentially consistent world
        if (result1 == 0 && result2 == 0) {
            bad_result_count++;
        }
    }

    printf("\nFinished! Out of %lld runs:\n", iterations);
    printf("Incorrect result occurred %lld times. \n", bad_result_count);
    
    printf("\n--------------------------------------------\n\n");
    
    bad_result_count = 0; // Reset counter for the next test
    printf("### Running test WITH memory fence...\n");

    for (long long i = 0; i < iterations; i++) {
        x = 0;
        y = 0;
        
        pthread_create(&t1, NULL, thread1_fixed, NULL);
        pthread_create(&t2, NULL, thread2_fixed, NULL);

        pthread_join(t1, NULL);
        pthread_join(t2, NULL);

        if (result1 == 0 && result2 == 0) {
            bad_result_count++;
        }
    }

    printf("\nFinished! Out of %lld runs:\n", iterations);
    printf("Incorrect result occurred %lld times. \n", bad_result_count);

    return 0;
}
