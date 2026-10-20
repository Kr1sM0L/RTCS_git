#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

void ctrl_c_handler(int signum) {
    printf("Ctrl+C were detected (signal %i)!\nStopping process %i!\n", signum, getpid());
    kill(getpid(), SIGSTOP);
}


int main() {
    if (signal(SIGINT, ctrl_c_handler) == SIG_ERR) {
        printf("Signal error!\n");
        return 0;
    }
    printf("Start working!\n");
    int number;
    while (1) {
        printf("Enter the number: ");
        if (scanf("%i", &number) == 1) {
            printf("Good number: %i!\n", number);
        } else {
            while(getchar() != '\n');
        }
        sleep(1);
        printf("Good number!\n");
    }
    return 1;
}