#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <signal.h>


void inter_handler(int sig) {
    printf("\nSignal accepted: %d\nYou stopped a sleep program.\n", sig);
}

void alarm_handler(int sig) {
    printf("Signal accepted: %d\n", sig);
}

unsigned int my_sleep(int seconds) {
    signal(SIGALRM, alarm_handler);
    signal(SIGINT, inter_handler);
    siginterrupt(SIGALRM, 1);
    alarm(seconds);
    pause();
    return alarm(0);
}

int main() {
    int seconds;
    while (1) {
        printf("Enter number of seconds to sleep > ");
        if ((scanf("%d", &seconds) != 1) || (seconds <= 0)) {
            printf("Error input value!\n");
        } else {
            printf("Seconds out > %d\n", my_sleep(seconds));
        }
    }
    return 0;
}