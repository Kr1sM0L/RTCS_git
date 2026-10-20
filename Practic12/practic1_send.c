#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/random.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>


#define SHM_KEY 1111
#define PERMS 0600


volatile sig_atomic_t is_end = 0;


struct shm_seg
{
    int current_messages;
    int initialised;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
};


void int_handler(int signum)
{
    printf("\nSignal %d accepted. Break program.\n", signum);
    is_end = 1;
}

void init_shm_seg(struct shm_seg *shm_data)
{
    shm_data->current_messages = 0;

    pthread_mutexattr_t ma;
    pthread_mutexattr_init(&ma);
    pthread_mutexattr_setpshared(&ma, PTHREAD_PROCESS_SHARED);

    pthread_condattr_t cd;
    pthread_condattr_init(&cd);
    pthread_condattr_setpshared(&cd, PTHREAD_PROCESS_SHARED);

    pthread_mutex_init(&shm_data->mutex, &ma);
    pthread_cond_init(&shm_data->cond, &cd);

    pthread_mutexattr_destroy(&ma);
    pthread_condattr_destroy(&cd);

    __sync_synchronize();
    shm_data->initialised = 1;
}

int main()
{
    struct shm_seg *shm_data;
    int flag_is_create;
    int retval;


    if (signal(SIGINT, int_handler) == SIG_ERR)
    {
        printf("signal");
        return -1;
    }
    siginterrupt(SIGINT, 1);

    int shm_id = shmget(SHM_KEY, sizeof(struct shm_seg), PERMS | IPC_CREAT | IPC_EXCL);
    if (shm_id >= 0)
    {
        flag_is_create = 1;
    } else if(errno == EEXIST)
    {
        shm_id = shmget(SHM_KEY, sizeof(struct shm_seg), PERMS | IPC_CREAT);
        if (shm_id < 0)
        {
            perror("shmget: shmid after EEXIST");
            return -1;
        }
        flag_is_create = 0;
    } else
    {
        perror("shmget: shmid");
        return -1;
    }

    shm_data = shmat(shm_id, NULL, 0);
    if (shm_data == (void*)-1)
    {
        perror("shmat");
        return -1;
    }

    if (flag_is_create == 1)
    {
        memset(shm_data, 0, sizeof(struct shm_seg));
        init_shm_seg(shm_data);
    } else
    {
        while (shm_data->initialised != 1)
        {
            usleep(500);
        }
    }

    int time_sleep;
    int time_sleep_us;
    int delay_ms;
    struct timeval start, end;
    printf("SEND %5d: start sending.\n", getpid());
    while ((rand() % 20 != 1) && (is_end != 1))
    {
        time_sleep = rand() % 20;
        time_sleep_us = (time_sleep / 10.0 + 0.1) * 1000000;
        usleep(time_sleep_us);
        gettimeofday(&start, NULL);
        printf("SEND %5d: sending message.\n", getpid());
        pthread_mutex_lock(&shm_data->mutex);
        shm_data->current_messages += 1;
        pthread_cond_broadcast(&shm_data->cond);
        pthread_mutex_unlock(&shm_data->mutex);
        gettimeofday(&end, NULL);
        delay_ms = (end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000;
        printf("SEND %5d: sending successful (%d ms).\n", getpid(), delay_ms);
    }

    printf("SEND %5d: sending done.\n", getpid());

    retval = shmdt(shm_data);
    if (retval == -1)
    {
        perror("shmdt");
        return -1;
    }

    printf("SEND %5d: program done.\n", getpid());
    return 0;
}