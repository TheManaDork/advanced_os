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
     * Request a private, read/write mapping of a read-only file.
     *
     * MAP_PRIVATE means that writes to the mapping will not modify
     * the underlying file. Instead, the kernel creates a private
     * copy of the affected page using copy-on-write (COW).
     *
     * Therefore, PROT_WRITE is allowed even though the file was
     * opened with O_RDONLY.
     */
    char *map = mmap(NULL, st.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, f, 0);

    if (map == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    printf("First 20 bytes: %.20s\n", map);

    /*
     * Modify the mapping.
     *
     * Because this is a MAP_PRIVATE mapping, the write triggers
     * copy-on-write. The modification is made to a private copy
     * of the page, not to /etc/passwd itself.
     */
    map[5] = 'X';

    printf("After write:    %.20s\n", map);

    /*
     * Now examine the actual file:
     *
     *     cat /etc/passwd
     *
     * You will see that /etc/passwd has NOT been changed.
     * The write only modified this process's private copy.
     */

    munmap(map, st.st_size);
    close(f);

    return 0;
}
