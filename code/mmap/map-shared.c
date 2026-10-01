#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

int main()
{
    // Open data.bin for reading and writing.
    // Create the file if it does not already exist.
    int fd = open("data.bin", O_RDWR | O_CREAT, 0600);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    // Set the file size to 4096 bytes.
    // The file must be large enough to back our 4096-byte mapping.
    if (ftruncate(fd, 4096) < 0) {
        perror("ftruncate");
        return 1;
    }

    // Map the first 4096 bytes of the file into our address space.
    // MAP_SHARED allows changes to the mapping to be visible
    // to other processes that share the same mapping.
    char *map = mmap(NULL, 4096,
                    PROT_READ | PROT_WRITE,
                    MAP_SHARED,
                    fd, 0);

    if (map == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    // Write some initial data through the memory mapping.
    strcpy(map, "Hello from the parent!");
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        printf("Child sees: %s\n", map);
        strcpy(map, "Hello from the child!");
    } else {
        wait(NULL);
        printf("Parent sees: %s\n", map);
    }

    munmap(map, 4096);
    close(fd);

    return 0;
}
