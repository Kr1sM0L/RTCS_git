#include <stdio.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <sys/sem.h>


#define BUF_SIZE 256
#define SEM_KEY 1111
#define SHM_KEY 1000


struct shmseg {
    int cnt;
    int complete;
    int is_update;
    int number_sleepers;
    char buf[BUF_SIZE];
};

union semnum {
    int val;
    struct semid_ds *buff;
    unsigned short *array;
};


volatile sig_atomic_t is_end = 0;


void int_handler(int signum) {
    printf("\nSignal accepted: %d. Stop program\n", signum);
    is_end = 1;
}

int write_string(char *buffer, int buf_size) {
    printf("> ");
    fflush(stdout);

    int write_bytes;
    if (fgets(buffer, buf_size, stdin) == NULL) {
        if (errno == EINTR) return -1;
        perror("fgets");
        return -1;
    }

    buffer[strcspn(buffer, "\n")] = '\0';
    write_bytes = strlen(buffer);
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
    union semnum arg;
    struct semid_ds sem_log; 
    struct sembuf sem_buf;
    int retval;
    char input[BUF_SIZE];

    int state = 0;

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

    int semid = semget(SEM_KEY, 1, 0600 | IPC_CREAT | IPC_EXCL);
    if (semid >= 0) {
        arg.val = 1;
        if (semctl(semid, 0, SETVAL, arg) < 0) {
            perror("semctl");
            return -1;
        }
        printf("SEND: creating semaphore.\n");
    } else {
        if (errno != EEXIST) {
            perror("semget");
            return -1;
        } else {
            printf("SEND: now semaphore busy, waiting.\n");
            semid = semget(SEM_KEY, 1, 0600);
            if (semid < 0) {
                perror("semid: start waiting");
                return -1;
            }
            struct semid_ds ds;
            arg.buff = &ds;
            while ((semctl(semid, 0, IPC_STAT, arg) != -1) && (ds.sem_otime == 0)) {
                usleep(10000);
            }
        }
    }

    sem_buf.sem_op = -1;
    sem_buf.sem_flg = SEM_UNDO;
    sem_buf.sem_num = 0;
    if ((retval = semop(semid, &sem_buf, 1)) == -1) {
        if (errno == EINTR && is_end) return 0;
        perror("semlock");
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
    shm_data->number_sleepers = 0;
    buff_ptr = shm_data->buf;
    space_available = BUF_SIZE;
    int counter = 0;
    int mem_counter = 0;

    printf("SEND: start writing.\n");
    while (!is_end) {
        int write_bytes = write_string(input, BUF_SIZE);
        if (write_bytes < 0) break;
        if (strcmp(input, "END") == 0) {
            printf("Ok, stopping program.\n");
            break;
        }
        if (state == 1) {
            if (strcmp(input, "START") == 0) {
                printf("Ok, start writing in process.\nIf you want stop again enter STOP\n");

                sem_buf.sem_num = 0;
                sem_buf.sem_op = -1;
                sem_buf.sem_flg = SEM_UNDO | IPC_NOWAIT;
                if (semop(semid, &sem_buf, 1) == -1) {
                    if (errno == EAGAIN) {
                        printf("Semaphore busy. Waiting.\n");
                        fflush(stdout);

                        sem_buf.sem_flg = SEM_UNDO;
                        if (semop(semid, &sem_buf, 1) == -1) {
                            perror("semop: waiting after start");
                            return -1;
                        }
                    } else {
                        perror("semop: grabbing after start");
                        return -1;
                    }
                }

                state = 0;

                shm_data->cnt = 0;
                shm_data->complete = 0;
                shm_data->is_update = 0;
                shm_data->number_sleepers -= 1;
                buff_ptr = shm_data->buf;
                space_available = BUF_SIZE;

                counter = 0;
                mem_counter = 0;
            } else {
                printf("Paused. Type START to continue.\n");
            }
            continue;
        }
        if (strcmp(input, "STOP") == 0) {
            printf("Ok, stop writing in process.\nIf you want transmit again enter START\n");

            sem_buf.sem_num = 0;
            sem_buf.sem_op = 1;
            sem_buf.sem_flg = SEM_UNDO;
            shm_data->number_sleepers += 1;
            if (semop(semid, &sem_buf, 1) == -1) {
                perror("semop: stop");
                return -1;
            }
            state = 1;
            continue;
        }
        if (write_bytes + 1 > space_available) {
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
            shm_data->buf[0] = '\0';
            buff_ptr = shm_data->buf;
            space_available = BUF_SIZE;
            continue;
        }
        memcpy(buff_ptr, input, (size_t)write_bytes + 1);
        shm_data->cnt += (write_bytes + 1);
        buff_ptr = shm_data->buf + shm_data->cnt;

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


    printf("Writing SHM: Complete\n");
    return 0;
}