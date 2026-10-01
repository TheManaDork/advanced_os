#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#define PAGE_SIZE 4096

int main()
{
    // Open the device created by the kernel module.
    // The returned file descriptor is associated with the module's
    // file_operations structure, including its .mmap callback.
    int fd = open("/dev/mmap_example", O_RDWR);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    // mmap() on this device invokes the kernel module's
    // my_mmap() callback.
    //
    // The callback maps the kernel module's kernel_buffer
    // into this process's user-space address space.
    //
    // After mmap() returns, ptr refers to the same physical
    // memory page as kernel_buffer in the kernel module.
    char *ptr = mmap(NULL, PAGE_SIZE,
                     PROT_READ | PROT_WRITE,
                     MAP_SHARED,
                     fd, 0);

    if (ptr == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }

    // Write through the user-space mapping.
    //
    // This does not call the kernel module again. Instead, the
    // CPU writes directly to the physical page that the module
    // mapped here, thereby modifying kernel_buffer.
    snprintf(ptr, PAGE_SIZE, "Hello from user-space!\n");

    // Read the same data back through the user-space mapping.
    //
    // printf() is not communicating with the kernel module.
    // It simply reads the bytes from ptr and prints them.
    printf("%s", ptr);

    // Remove the mapping from this process's virtual address space.
    munmap(ptr, PAGE_SIZE);

    // Close the device file.
    close(fd);

    return 0;
}
