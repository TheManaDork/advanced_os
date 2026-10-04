#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main(void)
{
    // Try to open /etc/passwd for reading only
    int f = open("/etc/passwd", O_RDONLY);

    if (f == -1) {
        perror("open");
        return 1;
    }

    printf("File opened successfully.\n");

    close(f);
    return 0;
}
