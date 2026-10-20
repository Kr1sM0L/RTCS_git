#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define LEN 10

int fd; 
volatile sig_atomic_t got_sigint = 0;

void onintr(int sig) {
    signal(sig, onintr);
    got_sigint = 1;
}

int main(void) {
    char buffer[LEN];
    char symbol;
    ssize_t bytes_read;

    printf("Opening file...\n");

    fd = open("text.txt", O_RDONLY);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    if (signal(SIGINT, onintr) == SIG_ERR) {
        perror("signal");
        close(fd);
        return 1;
    }

    printf("Start file > \n");
    while ((bytes_read = read(fd, buffer, LEN - 1)) > 0) {
        buffer[bytes_read] = '\0';
        
        char *find_newline = strchr(buffer, '\n');

        if (find_newline) {
            int new_length = (int)(buffer - find_newline) + 1;

            buffer[new_length] = '\0';

            lseek(fd, new_length - bytes_read, SEEK_SET);
        }

        printf("%s", buffer);

        sleep(1);

        if (got_sigint) {
            got_sigint = 0;

            printf("\nContinue? y - continue, n - end, r - restart: ");
            scanf(" %c", &symbol);

            if (symbol == 'n') {
                printf("End program.\n");
                break;
            } else if (symbol == 'r') {
                if (lseek(fd, 0L, SEEK_SET) == -1) {
                    perror("lseek");
                    break;
                }
                printf("Start file > \n");
            }
        }
    }

    close(fd);
    return 0;
}