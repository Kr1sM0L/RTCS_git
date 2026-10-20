#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>


#define FILE "SIGNLE_TRANSMITING"
#define LEN 256

int main() {
    char end_string[] = "END CESSION password 1505";
    int fd;
    char buffer[LEN];
    ssize_t read_bytes;

    unlink(FILE);
    mkfifo(FILE, 0640);

    fd = open(FILE, O_RDONLY);

    while (1) {
        memset(buffer, 0, LEN);
        read_bytes = read(fd, buffer, LEN);
        if (read_bytes > 0) {
            buffer[read_bytes] = '\0';
            if (strcmp(buffer, end_string) != 0) {
                printf("Received string: %s.\n", buffer);
            } else {
                printf("End signal is accepted.\n");
                close(fd);
                return 0;
            }
        }
    }
}