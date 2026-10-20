#include <stdio.h>
#include <unistd.h>
#include <signal.h>


#define LEN 256

void alarm_handler(int sig) {
    printf("Program is end [%d]\nOutput > ", sig);
}


int main() {
    char buffer[LEN];
    signal(SIGALRM, alarm_handler);
    siginterrupt(SIGALRM, 1);

    alarm(10);

    char *out_fgets;
    
    if ((out_fgets = fgets(buffer, LEN, stdin)) != NULL) {
        printf("Thank you for your input!\n");
        alarm(0);
    } else {
        printf("No input :(\n");
    }

    printf("Out fgets: %s\n", out_fgets);
    return 0;
}