#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>

int main(void)
{
    /*
     * Open zzz for both reading and writing.
     *
     * O_CREAT creates the file if it does not already exist.
     * O_TRUNC clears the file if it already exists so that we
     * start with known content.
     *
     * The final argument (0644) specifies the permissions used
     * when the file is created.
     */
    int f = open("zzz", O_RDWR | O_CREAT | O_TRUNC, 0644);

    if (f == -1) {
        perror("open");
        return 1;
    }

    /*
     * Put some initial content into the file.
     */
    const char *content = "Hello from the zzz file!\n";

    if (write(f, content, strlen(content)) == -1) {
        perror("write");
        close(f);
        return 1;
    }

    /*
     * Get the size of the file.
     */
    struct stat st;

    if (fstat(f, &st) == -1) {
        perror("fstat");
        close(f);
        return 1;
    }

    /*
     * Request a shared, read/write mapping.
     *
     * Because this is MAP_SHARED, writes through the mapping
     * are reflected in the underlying file.
     *
     * Unlike the previous example with /etc/passwd, this mmap()
     * succeeds because we opened zzz with O_RDWR.
     */
    char *map = mmap(NULL, st.st_size,
                     PROT_READ | PROT_WRITE,
                     MAP_SHARED, f, 0);

    if (map == MAP_FAILED) {
        perror("mmap");
        close(f);
        return 1;
    }

    printf("Before write: %.24s\n", map); // print at most 24 characters from the string.

    /*
     * Modify the mapped memory.
     *
     * Because this is MAP_SHARED, this modification is also
     * reflected in the underlying file zzz.
     */
    map[0] = 'h';

    /*
     * msync() asks the kernel to synchronize the modified
     * memory with the underlying file.
     */
    if (msync(map, st.st_size, MS_SYNC) == -1) {
        perror("msync");
    }

    printf("After write:  %.24s\n", map);

    munmap(map, st.st_size);
    close(f);

    /*
     * Examine the actual file after running:
     *
     *     cat zzz
     *
     * You should see:
     *
     *     hello from the zzz file!
     *
     * The first character changed from 'H' to 'h'.
     */

    return 0;
}
