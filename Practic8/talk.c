#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define READ 0
#define WRITE 1


char *msg = "Hello from child";


int main() {
    int fd[2];
    ssize_t bytes_read;
    char buffer[128];
    pipe(fd);
    if (fork() == 0) {
        printf("Child: work.\n");
        close(fd[READ]);
        write(fd[WRITE], msg, strlen(msg) + 1);
        close(fd[WRITE]);
        printf("Child: none.\n");
    } else {
        printf("Parent: work.\n");
        close(fd[WRITE]);
        bytes_read = read(fd[READ], buffer, 128);
        close(fd[READ]);
        printf("Bytes read: %ld\nBuffer: %s\n", bytes_read, buffer);
        printf("Parent: none.\n");
    }
    return 0;
}