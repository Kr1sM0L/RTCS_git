#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>


#define shm_name "my_shared_memory"
#define SIZE 4096*2


int main() {
    int shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
    
    ftruncate(shm_fd, SIZE);

    pid_t pid = fork();

    if (pid == 0) {
        char *shared_mem = (char*)mmap(0, SIZE, PROT_READ, MAP_SHARED, shm_fd, 4096);
        printf("Child read: %s", shared_mem);
        munmap(shared_mem, SIZE);
    } else {
        char *shared_mem = (char *)mmap(0, SIZE, PROT_WRITE, MAP_SHARED, shm_fd, 4096);
        const char *message = "Hello from parent!\n";
        sprintf(shared_mem, "%s", message);
        munmap(shared_mem, SIZE);
        wait(NULL);
    }
    shm_unlink(shm_name);
    return 0;
}