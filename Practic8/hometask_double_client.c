#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>


#define FILE_S2C "dt_s2c.fifo"
#define FILE_C2S "dt_c2s.fifo"
#define LEN 256


static volatile sig_atomic_t is_end = 0;


void int_hand(int signum) {
    printf("\nSignal accepted: %d\nEnd cession.\n", signum);
    is_end = 1;
}

int main() {
    int read_bytes;
    int write_bytes;
    char read_buffer[LEN];
    char write_buffer[LEN];

    if (signal(SIGINT, int_hand) == SIG_ERR) {
        perror("signal init error");
        return 1;
    }
    siginterrupt(SIGINT, 1);

    int fd_write = open(FILE_C2S, O_WRONLY);
    if (fd_write == -1) {
        perror("write fd error");
        return 1;
    }

    int fd_read = open(FILE_S2C, O_RDONLY);
    if (fd_read == -1) {
        perror("read fd error");
        return 1;
    }

    printf("Starting 2 way transmiting.\n");
    while (1) {
        if (is_end != 1) {
            printf("> ");
            if (fgets(write_buffer, LEN, stdin) == NULL) {
                if (is_end != 1) {
                    continue;
                } else {
                    break;
                }
            }

            write_bytes = strcspn(write_buffer, "\n");
            write_buffer[write_bytes] = '\0';
            if (write(fd_write, write_buffer, write_bytes) == -1) {
                perror("send message error");
                return 1;
            }

            read_bytes = read(fd_read, read_buffer, LEN);
            if (read_bytes == -1) {
                perror("receive message error");
                return 1;
            }
            read_buffer[read_bytes] = '\0';
            if (strcmp(write_buffer, read_buffer) != 0) {
                printf("Message damaged! (repeat if you want)\n");
            } else {
                printf("Transmission succesful\n");
            }
        } else {
            printf("Ok, stopping client\n");
            close(fd_read);
            close(fd_write);
            return 0;
        }
    }
}