// 4 task
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>


int main() {
    const char *fifo_path = "/tmp/my_fifo";
    mkfifo(fifo_path, 0666);
    if (fork() == 0) {
        int fd = open(fifo_path, O_RDONLY);
        char buffer[128];
        read(fd, buffer, sizeof(buffer));
        printf("Child received: %s\n", buffer);
        close(fd);
    } else {
        int fd = open(fifo_path, O_WRONLY);
        const char *msg = "Hello from parent.";
        write(fd, msg, strlen(msg)+1);
        close(fd);
        wait(NULL);
    }

    unlink(fifo_path);
    return 0;
}