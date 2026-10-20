// 3 task
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>


#define READ 0
#define WRITE 1


int main(int argc, char *argv[]) {
    int fd[2];
    pipe(fd);
    if (fork() == 0) {
        close(fd[READ]);
        dup2(fd[WRITE], 1);
        close(fd[WRITE]);
        execlp(argv[1], argv[2], NULL);
        perror("connect");
    } else {
        close(fd[WRITE]);
        dup2(fd[READ], 0);
        close(fd[READ]);
        execl(argv[3], argv[4], NULL);
        perror("connect");
    }
}