#include <stdio.h>
#include <unistd.h>
#define MAX_COUNT 200


int main() {
    int pid;
    printf("Start working program.\n");
    pid = fork();

    if (pid!=0) {
        usleep(2500);
        printf("Parent (PID : %d) start working!\n", getpid());
        for (int i = 0; i < MAX_COUNT; i++) {
            if (i % 5 == 0) {
                printf("This line is from parent, value = %d\n", i);
            }
        }
    } else {
        printf("Child (PID : %d) start working!\n", getpid());
        execlp("./task2_forchild", "task2_forchild", NULL);
        perror("Exec failed\n");
    }
    printf("Process %d is over.\n", getpid());
}