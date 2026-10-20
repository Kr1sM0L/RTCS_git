#include <stdio.h>
#include <unistd.h>
#define MAX_COUNT 200


int ParentProcess(void) {
    for (int i = 0; i < MAX_COUNT; i++) {
        if (i % 5 == 0) {
            printf("This line is from parent, value = %d\n", i);
        }
    }
    return 0;
}


int ChildProcess(void) {
    for (int i = 0; i < MAX_COUNT; i++) {
        if (i % 5 == 0) {
            printf("This line is from child, value = %d\n", i);
        }
    }
    return 0;
}

int main() {
    int pid;
    printf("Start working program.\n");
    pid = fork();

    if (pid!=0) {
        printf("Parent (PID : %d) start working!\n", getpid());
        ParentProcess();
    } else {
        printf("Child (PID : %d) start working!\n", getpid());
        ChildProcess();
    }
    printf("Process %d is over.\n", getpid());
}