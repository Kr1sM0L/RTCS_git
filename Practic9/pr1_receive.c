#include <stdio.h>
#include <sys/shm.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>


#define BUF_SIZE 256
#define SHM_KEY 1000


struct shmseg {
    int cnt;
    int complete;
    int is_update;
    char buf[BUF_SIZE];
};


int main() {
    int shmid;
    int prev_cnt;
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

    prev_cnt = 0;
    printf("Reading SHM: Start reading.\n");
    while (shm_data->complete != 1) {
        if (shm_data->is_update == 1) {
            printf("Segment update:\n%s", &shm_data->buf[prev_cnt]);
            if (shm_data->cnt == -1) {
                perror("read");
                return -1;
            }

            if (prev_cnt > shm_data->cnt) {
                prev_cnt = 0;
            }
            printf("Reading SHM: Read %d bytes\n", shm_data->cnt - prev_cnt);
            shm_data->is_update = 0;
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

    printf("Reading SHM: Process complete.\n");
    return 0;
}