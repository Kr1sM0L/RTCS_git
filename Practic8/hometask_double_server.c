#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>


#define FILE_S2C "dt_s2c.fifo"
#define FILE_C2S "dt_c2s.fifo"
#define LEN 256


const static char end_string[] = "END CESSION password 1505";


int main() {
    int read_bytes;
    char buffer[LEN];
    unlink(FILE_C2S);
    unlink(FILE_S2C);
    mkfifo(FILE_C2S, 0660);
    mkfifo(FILE_S2C, 0660);

    int fd_read = open(FILE_C2S, O_RDONLY);
    if (fd_read == -1) {
        perror("read fd error");
        return 1;
    }

    int fd_write = open(FILE_S2C, O_WRONLY);
    if (fd_write == -1) {
        perror("write fd error");
        return 1;
    }
    
    while (1) {
        memset(buffer, 0, LEN);
        read_bytes = read(fd_read, buffer, LEN-1);
        if (read_bytes == -1) {
            perror("read message error");
            return 1;
        }

        if (read_bytes == 0) {
            printf("Ok, stopping server.\n");
            close(fd_read);
            close(fd_write);
            return 0;
        }

        buffer[read_bytes] = '\0';
        printf("Message: %s\n", buffer);

        if (write(fd_write, buffer, read_bytes) == -1) {
            perror("send message error");
            return 1;
        }
        printf("Replay sent\n");
    }
}