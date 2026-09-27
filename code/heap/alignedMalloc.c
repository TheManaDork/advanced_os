#include <stdio.h>
#include <stdlib.h>

/*
 * alignedMalloc()
 *
 * Allocate 'size' bytes of memory whose starting address is aligned
 * to the specified 'alignment' boundary.
 *
 * Usage:
 *
 *     void *ptr = alignedMalloc(size, alignment);
 *
 * Example:
 *
 *     void *ptr = alignedMalloc(24, 4096);
 *
 * The returned address will be a multiple of 4096:
 *
 *     (size_t)ptr % 4096 == 0
 *
 * The implementation calls malloc() internally and may return an
 * address different from the address returned by malloc(). It stores
 * the original malloc() pointer immediately before the aligned
 * address so that alignedFree() can later recover and free it.
 *
 * Requirements:
 *   - 'alignment' should be a power of two (4, 8, 16, 4096, ...).
 *   - 'alignment' should be a positive value.
 *
 * The memory returned by alignedMalloc() must be released using
 * alignedFree(), not free():
 *
 *     alignedFree(ptr);
 *
 * This is a simplified implementation for demonstrating how
 * aligned memory allocation can be implemented. Production programs
 * should normally use standard interfaces such as posix_memalign()
 * or aligned_alloc().
 */
void *alignedMalloc(size_t size, size_t alignment){
        void *ptr1;
        void *ptr2;
        if(alignment >= sizeof(size_t *)){
                ptr1 = malloc(size + alignment + alignment);
                // %p prints a pointer address
                printf("ptr1 address is %p\n", ptr1);
                /* this formula will give us the next aligned address */
                ptr2 = (void *)((size_t)((char *)ptr1 + alignment - 1) & (~(alignment-1)));
                /* note (ptr2-ptr1), i.e., the distance between ptr2 and ptr1 is in between 0 and (alignment - 1) */
                printf("ptr2 address is (aligned) %p\n", ptr2);
                /* we must make sure the distance is at least the sizeof(size_t *), so that we can store a pointer. */
                if ((size_t)((char *)ptr2 - (char *)ptr1) < sizeof(size_t *)) {
                        ptr2 = ptr2 + alignment;
                }
        }else{
                ptr1 = malloc(size + alignment + sizeof(size_t *));
                printf("ptr1 address is %p\n", ptr1);
                /* this formula will give us the next aligned address */
                ptr2 = (void *)((size_t)((char *)ptr1 + alignment - 1) & (~(alignment-1)));
                /* note (ptr2-ptr1), i.e., the distance between ptr2 and ptr1 is in between 0 and (alignment - 1) */
                printf("ptr2 address is (aligned) %p\n", ptr2);
                /* we must make sure the distance is at least the sizeof(size_t *), so that we can store a pointer. */
                while ((size_t)((char *)ptr2 - (char *)ptr1) < sizeof(size_t *)) {
                        /* we increment ptr2 by alignment each time because we want to make sure ptr2 is always an aligned address. */
                        ptr2 = ptr2 + alignment;
                }
        }
        printf("eventually ptr2 address is (aligned) %p\n", ptr2);
        /* ptr2 going back one step (size_t) is a (size_t) pointer, which stores the address of ptr1. */
        size_t *ptr3;
        ptr3 = (size_t *)ptr2 - 1;
        *ptr3 = (size_t)ptr1;
        return ptr2;
}

void alignedFree(void *ptr){
	size_t *ptr3;
	/* this is where we store that size_t pointer */
	ptr3 = (size_t *)ptr - 1;
	printf("ptr3 address is %p\n", ptr3);
	void *ptr4;
	ptr4 = (void *)*ptr3;
	printf("ptr4 address is %p\n", ptr4);
	free(ptr4);
}

int main() {
    void *ptr;
    void *ptr1;
    int alignment;
    printf("sizeof(size_t) is %ld\n", sizeof(size_t));
    printf("sizeof(void *) is %ld\n", sizeof(void *));

    alignment=4;
    ptr = alignedMalloc(24, alignment);
    //memset(ptr, 0, 24);
    printf("ptr address (aligned) is %p\n", ptr);
    alignedFree(ptr);

    alignment=4096;
    ptr1 = alignedMalloc(24, alignment);
    printf("ptr1 address (aligned) is %p\n", ptr1);
    alignedFree(ptr1);

    alignment=4;
    //alignment=4096;
    int i = 0;
    char *buf = NULL;
    char *real_buf = NULL;
    char *real_buf2 = NULL;
	
    real_buf = alignedMalloc(10,alignment);
    real_buf2 = alignedMalloc(10,alignment);

    real_buf2[0] = 's';
    real_buf2[1] = 'u';
    real_buf2[2] = 'c';
    real_buf2[3] = 'c';
    real_buf2[4] = 'e';
    real_buf2[5] = 's';
    real_buf2[6] = 's';
    real_buf2[7] = '\0';
    real_buf[0] = 't';
    real_buf[1] = 'e';
    real_buf[2] = 's';
    real_buf[3] = 't';
    real_buf[4] = '\0';
    while(i < 30){

        buf = alignedMalloc(10,alignment);
        buf[0] ='a';
    	printf("buf is %s\n", buf);
        i++;
    }

    printf("%s\n", real_buf);
    printf("%s\n", real_buf2);
    alignedFree(buf);
    alignedFree(real_buf);
    alignedFree(real_buf2);

    return 0;
}
