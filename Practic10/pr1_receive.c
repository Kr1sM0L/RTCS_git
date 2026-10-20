#include <stdio.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>


#define BUF_SIZE 256
#define SHM_KEY 1000
#define SEM_KEY 1111
#define TIME_WAITING 10.0


struct shmseg {
    int cnt;
    int complete;
    int is_update;
    int number_sleepers;
    char buf[BUF_SIZE];
};


int main() {
    int shmid;
    int prev_cnt;
    time_t start_complete;
    struct shmseg *shm_data;
    int message_counter = 0;

    shmid = shmget(SHM_KEY, sizeof(struct shmseg), 0600 | IPC_CREAT);
    if (shmid == -1) {
        perror("shmid");
        return -1;
    }

    shm_data = shmat(shmid, NULL, 0);
    if (shm_data == (void*)-1) {
        perror("shmat");
        return -1;
    }

    prev_cnt = 0;
    printf("Reading SHM: Start reading.\n");
    start_complete = time(NULL);
    while (1) {
        if ((shm_data->complete == 1) || (shm_data->number_sleepers != 0)) {
            if (difftime(time(NULL), start_complete) > TIME_WAITING) {
                printf("No writing processes in %.1f seconds. Stopping.\n", TIME_WAITING);
                break;
            }
            prev_cnt = 0;
            continue;
        }
        start_complete = time(NULL);
        if ((shm_data->is_update == 1) || (shm_data->cnt != prev_cnt)) {
            printf("READ: Segment update:\n");
            for (int i = prev_cnt; i < shm_data->cnt; i++) {
                if (shm_data->buf[i] != '\0') {
                    putc(shm_data->buf[i], stdout);
                    continue;
                }
                if (i != shm_data->cnt - 1) {
                    printf("\n");
                }
                message_counter++;
            }
            if (shm_data->cnt == -1) {
                perror("read");
                return -1;
            }

            if (prev_cnt > shm_data->cnt) {
                prev_cnt = 0;
            }
            printf("\nReading SHM: Read %3d bytes, %3d messages\n", shm_data->cnt - prev_cnt, message_counter);
            shm_data->is_update = 0;
            message_counter = 0;
            prev_cnt = shm_data->cnt;
        } else {
            sleep(2);
        }
    }

    printf("Reading SHM: Reading Done.\n");

    if (shmdt(shm_data) == -1) {
        perror("shmdt");
        return -1;
    }

    if (shmctl(shmid, IPC_RMID, 0) == -1) {
        if (errno != EIDRM) {
            perror("shmctl: rmid");
            return -1;
        }
    }

    int semid;

    if ((semid = semget(SEM_KEY, 0, 0600)) == -1) {
        perror("semget");
        return -1;
    }

    if (semctl(semid, 0, IPC_RMID) == -1) {
        perror("semctl: rmid");
        return -1;
    }

    printf("Reading SHM: Process complete.\n");
    return 0;
}