#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


#define SHM_KEY 0x12345
#define SEM_KEY 0x54321


struct shmseg {
    int cntr;
    int write_complete;
    int read_complete;
};


void shm_cntr_increment(int pid, struct shmseg *shmp, int total_count);

void remove_semaphore();


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
        return 0;
    } else if (pid > 0) {
        shm_cntr_increment(shmid, shmp, total_count);
        wait(NULL);
    } else {
        perror("fork");
        return -1;
    }

    while (shmp->read_complete != 1) {
        printf("SHM_WRITE: wait reader.\n");
        sleep(3);
    }

    if (shmdt(shmp) == -1) {
        perror("shmdt");
        return -1;
    }

    if (shmctl(shmid, IPC_RMID, 0) == -1) {
        perror("shmctl: RMID");
        return -1;
    }

    remove_semaphore();
    printf("WRITE_SHM: finish.\n");
    return 0;
}


void shm_cntr_increment(int pid, struct shmseg *shmp, int total_count) {
    int cntr;
    int numtimes;
    int sleep_time;
    int semid;
    struct sembuf sem_buf;
    struct semid_ds buf;
    int tries;
    int retval;

    semid = semget(SEM_KEY, 1, IPC_CREAT | IPC_EXCL | 0666);

    if (semid >= 0) {
        printf("SHM_WRITE: first process.\n");
        sem_buf.sem_op = 1;
        sem_buf.sem_num = 0;
        sem_buf.sem_flg = 0;
        retval = semop(semid, &sem_buf, 1);
        if (retval == -1) {
            perror("semop");
            return;
        }
    } else if (errno == EEXIST) {
        int ready = 0;
        printf("SHM_WRITE: second process.");
        semid = semget(SEM_KEY, 1, 0);
        if (semid < 0) {
            perror("semget");
            return;
        }
        sem_buf.sem_op = 0;
        sem_buf.sem_flg = SEM_UNDO;
        sem_buf.sem_num = 0;

        retval = semop(semid, &sem_buf, 1);
        if (retval == -1) {
            perror("semop exist");
            return;
        }
    }

    sem_buf.sem_num = 0;
    sem_buf.sem_op = -1;
    sem_buf.sem_flg = SEM_UNDO;
    retval = semop(semid, &sem_buf, 1);

    if (retval == -1) {
        perror("sem close");
        return;
    }

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
            printf("SHM_CNTR is %d\n", shmp->cntr);
            sleep(1);
        }
    }

    shmp->write_complete = 1;
    sem_buf.sem_op = 1;
    retval = semop(semid, &sem_buf, 1);

    if (retval == -1) {
        perror("sem close");
        return;
    }

    if (pid == 0) {
        printf("SHM_WRITE: CHILD: Write done.\n");
    } else if (pid > 0) {
        printf("WRITE_SHM: PARENT: Write done.\n");
        return;
    }
}

void remove_semaphore() {
    int semid;
    int retval;
    semid = semget(SEM_KEY, 1, 0);
    if (semid == -1) {
        perror("semid");
        return;
    }

    retval = semctl(semid, 0, IPC_RMID);
    if (retval == -1) {
        perror("semop: RMID");
        return;
    }
}