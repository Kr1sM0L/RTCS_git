#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>


#define READ 0
#define WRITE 1


void dif_pipe(char *cmd1, char *opt1, char *cmd2, char *opt2) {
    int fd[2];

    pipe(fd);

    if (fork() == 0) {
        close(fd[READ]);
        dup2(fd[WRITE], 1);
        close(fd[WRITE]);
        execlp(cmd1, cmd1, opt1, NULL);
    }
    
    if (fork() == 0) {
        close(fd[WRITE]);
        dup2(fd[READ], 0);
        close(fd[WRITE]);
        execlp(cmd2, cmd2, opt2, NULL);
    }
    
    close(fd[READ]);
    close(fd[WRITE]);
    wait(NULL);
    wait(NULL);
}

int main() {
    printf("Start working.\n");
    dif_pipe("ls", NULL, "wc", "-l");
    dif_pipe("who", NULL, "wc", "-l");
}