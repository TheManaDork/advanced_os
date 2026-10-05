#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <emmintrin.h>
#include <x86intrin.h>

uint8_t array[256 * 4096];
int temp;
char secret = 94;

/*
 * Based on cachetime.c on this machine:
 *
 *   Cache hit  : ~20-22 cycles
 *   Cache miss : ~116-180 cycles
 *
 * Therefore, 25 cycles is a reasonable threshold for this machine.
 */
#define CACHE_HIT_THRESHOLD 25
#define DELTA 1024
#define NUM_ENTRIES 256

/*
 * Flush all candidate addresses from the CPU cache.
 */
void flushSideChannel()
{
    int i;
    /*
     * Touch every page first.
     *
     * This ensures that the pages are actually mapped and that
     * we are not measuring effects caused by copy-on-write/page
     * allocation.
     */
    for (i = 0; i < NUM_ENTRIES; i++)
        array[i * 4096 + DELTA] = 1;

    /*
     * Remove every candidate address from the CPU cache.
     */
    for (i = 0; i < NUM_ENTRIES; i++)
        _mm_clflush(&array[i * 4096 + DELTA]);
}

/*
 * The victim accesses one secret-dependent location.
 *
 * If secret == 94:
 *
 *     array[94 * 4096 + DELTA]
 *
 * should become cached.
 */
void victim()
{
    temp = array[secret * 4096 + DELTA];
}

/*
 * Randomize the order in which we reload the candidate addresses.
 *
 * This is important because reloading:
 *
 *     0, 1, 2, 3, 4, ...
 *
 * creates a predictable sequential memory-access pattern.
 *
 * Modern CPUs have hardware prefetchers that can recognize such
 * patterns and fetch future addresses into the cache. That can
 * create many false "cache hits."
 *
 * By using a random order, we make it much harder for the hardware
 * prefetcher to predict which candidate we will access next.
 */
void shuffle(int *order)
{
    int i;
    for (i = NUM_ENTRIES - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = order[i];
        order[i] = order[j];
        order[j] = temp;
    }
}


/*
 * Reload each candidate address and measure its access time.
 *
 * A cache hit should be around 20 cycles on the test machine,
 * while a cache miss is roughly 100+ cycles.
 *
 * The address accessed by the victim should therefore be
 * noticeably faster than most of the other addresses.
 */
void reloadSideChannel()
{
    int junk = 0;
    uint64_t time1;
    uint64_t time2;
    volatile uint8_t *addr;
    int order[NUM_ENTRIES];
    int i;

    /*
     * Create the list:
     *
     *     0, 1, 2, ..., 255
     */
    for (i = 0; i < NUM_ENTRIES; i++)
        order[i] = i;

    /*
     * Randomize the reload order to reduce the effect of
     * hardware prefetching.
     */
    shuffle(order);
    printf("Reload order:\n");

    for (i = 0; i < NUM_ENTRIES; i++) {
        int index = order[i];
        addr = &array[index * 4096 + DELTA];

        /*
         * Measure the time required to read this address.
         */
        time1 = __rdtscp(&junk);
        junk = *addr;
        time2 = __rdtscp(&junk) - time1;

        /*
         * A very fast access indicates that the address was
         * already in the cache before this read.
         */
        if (time2 <= CACHE_HIT_THRESHOLD) {
            printf("array[%d*4096 + %d] is in cache "
                   "(%lu cycles) <-- possible secret\n",
                   index, DELTA, time2);
        }
    }
}


int main(int argc, const char **argv)
{
    /*
     * Use a different random reload order each execution.
     */
    srand((unsigned int)time(NULL));

    /*
     * Step 1:
     * Remove all candidate addresses from the cache.
     */
    flushSideChannel();

    /*
     * Step 2:
     * Victim accesses the secret-dependent address.
     */
    victim();

    /*
     * Step 3:
     * Measure the access time of every candidate address.
     *
     * The secret-dependent address should be a cache hit.
     */
    reloadSideChannel();
    return 0;
}
