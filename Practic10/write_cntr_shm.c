#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


#define SHM_KEY 0x12345


struct shmseg {
    int cntr;
    int write_complete;
    int read_complete;
};


void shm_cntr_increment(int pid, struct shmseg *shmp, int total_count);


int main(int argc, char *argv[]) {
    int shmid;
    struct shmseg *shmp;
    char *bufptr;
    int total_count;
    int sleep_time;
    pid_t pid;


    if (argc != 2) {
        total_count = 10000;
    } else {
        total_count = atoi(argv[1]);
        if (total_count < 10000) {
            total_count = 10000;
        }
    }

    printf("Total count: %d\n", total_count);
    shmid = shmget(SHM_KEY, sizeof(struct shmseg), 0644 | IPC_CREAT);

    if (shmid == -1) {
        perror("shm");
        return -1;
    }

    shmp = shmat(shmid, NULL, 0);

    if (shmp == (void*) -1) {
        perror("Access shmp");
        return -1;
    }

    shmp->cntr = 0;

    pid = fork();

    if (pid == 0) {
        shm_cntr_increment(pid, shmp, total_count);
    } else if (pid > 0) {
        shm_cntr_increment(shmid, shmp, total_count);
        return 0;
    } else {
        perror("fork");
        return -1;
    }

    while (shmp->read_complete != 1) {
        sleep(1);
    }

    if (shmdt(shmp) == -1) {
        perror("shmdt");
        return -1;
    }

    if (shmctl(shmid, IPC_RMID, 0) == -1) {
        perror("shmctl: RMID");
        return -1;
    }
    printf("WRITE_SHM: finish.\n");
    return 0;
}


void shm_cntr_increment(int pid, struct shmseg *shmp, int total_count) {
    int cntr;
    int numtimes;
    int sleep_time;
    cntr = shmp->cntr;
    shmp->write_complete = 0;

    if (pid == 0) {
        printf("WRITE_SHM: CHILD: now writing.\n");
    } else if (pid > 0) {
        printf("WRITE_SHM: PARENT: now writing.\n");
    }
    printf("SHM_CNTR is %d\n", shmp->cntr);

    for (numtimes = 0; numtimes < total_count; numtimes++) {
        cntr += 1;
        shmp->cntr = cntr;
        sleep_time = cntr % 1000;
        if (sleep_time == 0) {
            usleep(500);
        }
    }

    shmp->write_complete = 1;
    if (pid == 0) {
        printf("SHM_WRITE: CHILD: Write done.\n");
    } else if (pid > 0) {
        printf("WRITE_SHM: PARENT: Write done.\n");
        return;
    }
}