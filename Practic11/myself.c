#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <sys/shm.h>


#define SHM_KEY 1000
#define PERMS 0600
#define BUF_SIZE 512
#define COUNT_WORKS_1 10
#define COUNT_WORKS_2 3     
#define TIME_WAITING 10.0


struct shmseg
{
    int cnt;
    int is_update;
    char buf[BUF_SIZE];
};


pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_to_send = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_to_receive = PTHREAD_COND_INITIALIZER;
struct shmseg *shm_buf;

void* send()
{
    printf("SEND: start working\n");
    for (int i = 0; i < COUNT_WORKS_1; i++)
    {

        pthread_mutex_lock(&mutex);
        while (shm_buf->is_update == 1)
        {
            pthread_cond_wait(&cond_to_send, &mutex);
        }
        int cur_index;
        int remain = BUF_SIZE;
        char *buf_ptr = shm_buf->buf;
        shm_buf->cnt = 0;
        shm_buf->is_update = 0;

        printf("SEND: start transmitting\n");

        for (int j = 0; j < COUNT_WORKS_2; j++)
        {
            if (j != COUNT_WORKS_2 - 1)
            {
                cur_index = snprintf(buf_ptr, remain, "process %4d\n", j);
            } else
            {
                cur_index = snprintf(buf_ptr, remain, "process %4d", j);
            }
            if ((cur_index) == -1)
            {
                perror("strncat");
                pthread_mutex_unlock(&mutex);
                break;
            }
            remain -= cur_index;
            buf_ptr += cur_index;
        }
        shm_buf->cnt = buf_ptr - shm_buf->buf;
        shm_buf->is_update = 1;
        pthread_cond_broadcast(&cond_to_receive);
        pthread_mutex_unlock(&mutex);

        printf("SEND: stop transmitting\n");
    }
    printf("SEND: stop working\n");
}

void* receive()
{
    int count = 0;
    struct timespec ts;
    time_t start_complete = time(NULL);
    printf("RECEIVE: start working\n");
    while (1)
    {
        pthread_mutex_lock(&mutex);
        if (shm_buf->is_update == 0)
        {
            clock_gettime(CLOCK_REALTIME, &ts);
            ts.tv_sec += 1;
            int rc = pthread_cond_timedwait(&cond_to_receive, &mutex, &ts);
            
            if (rc == ETIMEDOUT)
            {
                printf("READ: No writing processes in %.1ld seconds. Stopping.\n", ts.tv_sec);
                pthread_mutex_unlock(&mutex);
                break;
            }  
        } else
        {
            printf("READ: Segment update: \n%s\n", shm_buf->buf);
            if (shm_buf->cnt == -1)
            {
                perror("read");
                pthread_mutex_unlock(&mutex);
                break;
            }
            printf("READ: Read %3d bytes\n", shm_buf->cnt);
            count++;
            shm_buf->is_update = 0;
            start_complete = time(NULL);
        }
        pthread_cond_broadcast(&cond_to_send);
        pthread_mutex_unlock(&mutex);
    }
    if (count != (COUNT_WORKS_1 - 1) * COUNT_WORKS_1)
    {
        printf("Error in messaging");
    }
}

int main()
{   
    int shmid;

    shmid = shmget(SHM_KEY, sizeof(struct shmseg), 0600 | IPC_CREAT);
    if (shmid == -1) {
        perror("shmid");
        return -1;
    }

    shm_buf = shmat(shmid, NULL, 0);
    if (shm_buf == (void*)-1) {
        perror("shmat");
        return -1;
    }
    memset(shm_buf, 0, sizeof(struct shmseg));
    char *buf_ptr;
    
    int rc[COUNT_WORKS_1];
    pthread_t thread[COUNT_WORKS_1];

    for (int l = 0; l < COUNT_WORKS_1 - 1; l++)
    {
        rc[l] = pthread_create(&thread[l], NULL, &send, NULL);
    }

    rc[COUNT_WORKS_1 - 1] = pthread_create(&thread[COUNT_WORKS_1 - 1], NULL, &receive, NULL);

    for (int l = 0; l < COUNT_WORKS_1; l++)
    {
        pthread_join(thread[l], NULL);
    }

    int retval = shmdt(shm_buf);
    if (retval == -1) {
        perror("shmdt");
        return -1;
    }

    retval = shmctl(shmid, IPC_RMID, 0);
    if (retval == -1) {
        perror("shmctl: rmid");
        return -1;
    }

    printf("End\n");
    return 0;
}