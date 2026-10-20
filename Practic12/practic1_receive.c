#define _POSIX_C_SOURCE 199309L
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
#define TIME_WAITING 10.0


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

int main()
{
    struct shm_seg *shm_data;
    int shm_id;

    if (signal(SIGINT, int_handler) == SIG_ERR)
    {
        perror("signal");
        return -1;
    }
    siginterrupt(SIGINT, 1);

    shm_id = shmget(SHM_KEY, sizeof(struct shm_seg), PERMS);
    if (shm_id < 0)
    {
        perror("shmget");
        return -1;
    }

    shm_data = shmat(shm_id, NULL, 0);
    if (shm_data == (void*)-1)
    {
        perror("shmat");
        return -1;
    }

    struct timespec seconds_wait;
    int time_sleep;
    int time_sleep_us;
    while (is_end != 1)
    {
        int rc;
        pthread_mutex_lock(&shm_data->mutex);
        clock_gettime(CLOCK_REALTIME, &seconds_wait);
        seconds_wait.tv_sec += 10;
        rc = pthread_cond_timedwait(&shm_data->cond, &shm_data->mutex, &seconds_wait);
        if (rc != 0)
        {
            if (rc != ETIMEDOUT) perror("pthread_cont_timedwait");
            break;
        }

        
        while ((is_end != 1) && (shm_data->current_messages > 0))
        {
            printf("RECEIVE %5d: get message.\n", getpid());
            time_sleep = rand() % 5;
            time_sleep_us = (time_sleep / 10.0 + 0.5) * 1000000;
            usleep(time_sleep_us);
            if (rand() % 200 == 1) printf("RECEIVE %5d: error in messaging, retrying.\n", getpid());
            else
            {
                shm_data->current_messages -= 1;
                printf("RECEIVE %5d: message_done. %2d retain.\n", getpid(), shm_data->current_messages);
            }
        }
        printf("RECEIVE %5d: all messages handled. waiting others\n", getpid());
        pthread_mutex_unlock(&shm_data->mutex);
    }
    pthread_mutex_unlock(&shm_data->mutex);

    printf("RECEIVE %5d: receiving stop.\n", getpid());

    int ret_val;

    pthread_cond_destroy(&shm_data->cond);
    pthread_mutex_destroy(&shm_data->mutex);

    ret_val = shmdt(shm_data);
    if (ret_val == -1)
    {
        perror("shmdt");
        return -1;
    }

    ret_val = shmctl(shm_id, IPC_RMID, 0);
    if (ret_val == -1)
    {
        perror("shmctl: rmid");
        return -1;
    }

    printf("RECEIVE %5d: stop program, all clean.\n", getpid());

    return 0;
}