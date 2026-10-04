#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

int main(void)
{
    // /etc/passwd is a read-only file for this process.
    int f = open("/etc/passwd", O_RDONLY);

    if (f == -1) {
        perror("open");
        return 1;
    }

    struct stat st;
    if (fstat(f, &st) == -1) {
        perror("fstat");
        return 1;
    }

    /*
     * Create a private, read/write mapping of a read-only file.
     *
     * MAP_PRIVATE means that writes to the mapping do not modify
     * the underlying file. Instead, copy-on-write (COW) creates
     * a private copy of the affected page when we write to it.
     */
    char *map = mmap(NULL, st.st_size,
                     PROT_READ | PROT_WRITE,
                     MAP_PRIVATE, f, 0);

    if (map == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    printf("Before write:   %.20s\n", map);

    /*
     * Modify the mapping.
     *
     * This triggers COW. The modification is made to a private
     * copy of the page, not to /etc/passwd.
     */
    map[5] = 'X';

    printf("After write:    %.20s\n", map);

    /*
     * Tell the kernel that we do not need the pages in this
     * mapping anymore.
     *
     * For this private mapping, the modified COW page can be
     * discarded. The next access can use the original file-backed
     * contents again.
     */
    madvise(map, st.st_size, MADV_DONTNEED);

    printf("After madvise:  %.20s\n", map);

    /*
     * Also examine the actual file:
     *
     *     cat /etc/passwd
     *
     * The file was never modified. The 'X' only existed in the
     * private COW copy.
     */

    munmap(map, st.st_size);
    close(f);

    return 0;
}
