#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int argc;
char *argv[2];
int main (int argc, char *argv[]) {
    int fd;
    fd = open(argv[1], O_CREAT | O_WRONLY | O_TRUNC);

    dup2(fd,1);

    close(fd);

    execvp(argv[2], &argv[2]);
    perror("error in exec");
    return 0;
}