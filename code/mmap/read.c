#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

int main() {
    int fd = open("data.txt", O_RDONLY);
    char buffer[4096];
    ssize_t n = read(fd, buffer, sizeof(buffer));
    if (n < 0) {
        perror("read");
        close(fd);
        return 1;
    }

    if (n > 0) {
        printf("First char: %c\n", buffer[0]);
    }
    close(fd);
}
