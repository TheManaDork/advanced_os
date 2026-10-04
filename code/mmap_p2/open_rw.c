#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main(void)
{
    // Try to open /etc/passwd for both reading and writing.
    int f = open("/etc/passwd", O_RDWR);

    if (f == -1) {
        perror("open");
        return 1;
    }

    printf("File opened successfully.\n");

    close(f);
    return 0;
}
