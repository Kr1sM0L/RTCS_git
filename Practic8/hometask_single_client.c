#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>


#define FILE "SIGNLE_TRANSMITING"
#define LEN 256

int end;

void int_hand(int signum) {
    printf("Signal accepted: %d\nEnd cession.\n", signum);
    end = 1;
}


int main() {
    int fd;
    int in_bytes;
    end = 0;

    char buffer[LEN];
    printf("Single wave transmiting are ready!\n(Enter Ctrl+C to end cession)\n");
    if (signal(SIGINT, int_hand) == SIG_ERR) {
        printf("Error in signal initialisation!\n");
        return -1;
    }

    fd = open(FILE, O_CREAT|O_WRONLY);
    
    while (1) {
        memset(buffer, 0, LEN);
        printf("> ");
        if (fgets(buffer, LEN, stdin) != NULL) {
            in_bytes = strcspn(buffer, "\n");
            buffer[in_bytes] = '\0';
            write(fd, buffer, in_bytes);
        }
        if (end == 1) {
            char *end_string = "END CESSION password 1505";
            write(fd, end_string, strlen(end_string));
            close(fd);
            exit(1);
        }
    }
}