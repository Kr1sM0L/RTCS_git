#include <stdio.h>
#include <sys/shm.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>


#define SHM_KEY 1000
#define PERMS 0600
#define BUF_SIZE 512
#define TIME_WAITING 10.0


struct shmseg
{
    int cnt;
    int is_update;
    char buf[BUF_SIZE];
};


pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


int main() {
    int shmid;
    time_t start_complete;
    struct shmseg *shm_data;

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

    printf("READ: Start reading.\n");
    start_complete = time(NULL);
    while (1) {
        if (shm_data->is_update == 1) {
            if (difftime(time(NULL), start_complete) > TIME_WAITING) {
                printf("READ: No writing processes in %.1f seconds. Stopping.\n", TIME_WAITING);
                break;
            }
            continue;
        }
        start_complete = time(NULL);
        if (shm_data->is_update == 1) {
            pthread_mutex_lock(&mutex);
            printf("READ: Segment update: \n%s\n", shm_data->buf);
            if (shm_data->cnt == -1) {
                perror("read");
                return -1;
            }
            printf("\nReading SHM: Read %3d bytes\n", shm_data->cnt);
            shm_data->is_update = 0;
            pthread_mutex_unlock(&mutex);
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

    if (pthread_mutex_destroy(&mutex) == -1) {
        perror("mutex: destroy");
        return -1;
    }

    printf("Reading SHM: Process complete.\n");
    return 0;
}