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
     * Request a shared, read/write mapping of a read-only file.
     *
     * MAP_SHARED + PROT_WRITE means that writes through the mapping
     * would modify the underlying file.
     *
     * Therefore, the kernel requires the file descriptor to have
     * write permission. Since we opened /etc/passwd with O_RDONLY,
     * mmap() fails.
     *
     * Notice that mmap() fails even though we never actually write
     * to 'map'. The requested PROT_WRITE permission is enough.
     */
    char *map = mmap(NULL, st.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, f, 0);

    if (map == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    /*
     * We will never reach this line with the current program,
     * because mmap() fails above.
     *
     * If the mmap() succeeded, this write would modify the
     * underlying /etc/passwd file because this is MAP_SHARED.
     */
    map[5] = 'X';

    printf("After write: %.20s\n", map);

    munmap(map, st.st_size);
    close(f);

    return 0;
}
