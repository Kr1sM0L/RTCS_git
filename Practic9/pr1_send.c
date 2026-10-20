#include <stdio.h>
#include <sys/shm.h>
#include <string.h>
#include <signal.h>
#include <errno.h>


#define BUF_SIZE 256
#define SHM_KEY 1000


struct shmseg {
    int cnt;
    int complete;
    int is_update;
    char buf[BUF_SIZE];
};


volatile sig_atomic_t is_end = 0;

void int_handler(int signum) {
    printf("\nSignal accepted: %d. Stop program\n", signum);
    is_end = 1;
}

int write_string(char *buffer, int buf_size) {
    printf("> ");
    int write_bytes;
    if (fgets(buffer, buf_size, stdin) == NULL) {
        perror("fgets");
        return -1;
    }
    write_bytes = strlen(buffer);
    buffer[write_bytes] = '\0';
    return write_bytes;
}

char write_char() {
    printf("> ");
    char write_char;
    if (scanf(" %c", &write_char) != 1) {
        return -1;
    }
    return write_char;
}

int main() {
    int cnt;
    int complete;
    char *buff_ptr;
    struct shmseg *shm_data;
    int space_available;

    struct sigaction sa;
    sa.sa_handler = int_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        return -1;
    }
    
    int shmid = shmget(SHM_KEY, sizeof(struct shmseg), 0600 | IPC_CREAT);

    if (shmid == -1) {
        perror("shmid");
        return -1;
    }

    shm_data = shmat(shmid, NULL, 0);
    if (shm_data == (void *) -1) {
        perror("shmat");
        return -1;
    }

    shm_data->cnt = 0;
    shm_data->complete = 0;
    shm_data->is_update = 0;
    buff_ptr = shm_data->buf;
    space_available = BUF_SIZE;
    int counter = 0;
    int mem_counter = 0;

    while (!is_end) {
        int write_bytes = write_string(buff_ptr, BUF_SIZE);

        if (space_available - write_bytes < 0) {
            printf("Error: not enough memory. Enter C to clear memory (Current input data will safe) or stop program.\n");
            char clear_char = 'C';
            char clear_input;
            while (!is_end) {
                clear_input = write_char();
                if (clear_char != clear_input) {
                    printf("Please, repeat (need C to clear)\n");
                } else {
                    break;
                }
            }
            mem_counter++;
            shm_data->cnt = 0;
        } else {
            shm_data->cnt += write_bytes;
            buff_ptr = shm_data->buf + shm_data->cnt;
        }
        shm_data->complete = 0;
        space_available = BUF_SIZE - shm_data->cnt;
        printf("Write SHM: Wrote %4d bytes | %4d space available.\n", shm_data->cnt, space_available);
        shm_data->is_update = 1;
        counter++;
    }

    printf("Write SHM: Wrote counter: %3d | Cycles: %3d\n", counter, mem_counter);
    shm_data->complete = 1;

    if (shmdt(shm_data) == -1) {
        perror("shmd");
        return -1;
    }

    if (shmctl(shmid, IPC_RMID, 0) == -1) {
        perror("shmctl: rmid");
        return -1;
    }

    printf("Writing SHM: Complete\n");
    return 0;
}