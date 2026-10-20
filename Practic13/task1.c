#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
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
#define TIME_WAIT 10.0


struct shm_seg
{
    int trans_data;
    int count_writes;
    int version;
    pthread_mutex_t mutex;
    pthread_cond_t cond_to_writer;
    pthread_cond_t cond_to_readers;
};


volatile sig_atomic_t is_end;
struct shm_seg *shm_data;


void int_handler(int signum)
{
    printf("\nSignal %d accepted. Interrupting program.\n", signum);
    is_end = 1;
}

void *thread1_work();
void *thread2_work();
void *thread3_work();

int main()
{
    is_end = 0;

    if (signal(SIGINT, int_handler) == SIG_ERR)
    {
        perror("signal");
        return -1;
    }
    siginterrupt(SIGINT, 1);

    int shm_id = shmget(SHM_KEY, sizeof(struct shm_seg), PERMS | IPC_CREAT | IPC_EXCL);
    if (shm_id < 0)
    {
        perror("shmget: shmdata is exist");
        return -1;
    } else
    {
        printf("SHM_DATA is created.\n");
    }

    shm_data = shmat(shm_id, NULL, 0);
    if (shm_data == (void*)-1)
    {
        perror("shmat");
        is_end = 1;
    }

    pthread_mutexattr_t m_attr;
    pthread_mutexattr_init(&m_attr);
    pthread_mutexattr_setpshared(&m_attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&shm_data->mutex, &m_attr);

    pthread_condattr_t c_attr;
    pthread_condattr_init(&c_attr);
    pthread_condattr_setpshared(&c_attr, PTHREAD_PROCESS_SHARED);
    pthread_cond_init(&shm_data->cond_to_writer, &c_attr);
    pthread_cond_init(&shm_data->cond_to_readers, &c_attr);

    pthread_mutexattr_destroy(&m_attr);
    pthread_condattr_destroy(&c_attr);

    __sync_synchronize();

    shm_data->version = 0;
    shm_data->trans_data = 0;
    shm_data->count_writes = 0;

    int rc1, rc2, rc3;
    pthread_t thread1, thread2, thread3;

    if (is_end != 1)
    {
        rc1 = pthread_create(&thread1, NULL, thread1_work, NULL);
        if (rc1 == -1)
        {
            perror("pthread: thread1");
            is_end = 1;
        }
    }

    if (is_end != 1)
    {
        rc2 = pthread_create(&thread2, NULL, thread2_work, NULL);
        if (rc2 == -1)
        {
            perror("pthread: thread2");
            is_end = 1;
        }
    }

    if (is_end != 1)
    {
        rc3 = pthread_create(&thread3, NULL, thread3_work, NULL);
        if (rc3 == -1)
        {
            perror("pthread: thread3");
            is_end = 1;
        }
    }
    
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    pthread_join(thread3, NULL);

    pthread_mutex_destroy(&shm_data->mutex);
    pthread_cond_destroy(&shm_data->cond_to_writer);
    pthread_cond_destroy(&shm_data->cond_to_readers);
    
    printf("Working cycle is done! Closing all\n");
    if (shmdt(shm_data) == -1)
    {
        perror("shmdt");
        return -1;
    }

    if (shmctl(shm_id, IPC_RMID, 0) == -1)
    {
        perror("shmctl: RMID");
        return -1;
    }

    printf("All cleared! End.\n");
    return 0;
}

void *thread1_work()
{
    printf("THREAD 1: start working\n");
    int rc = 0;
    struct timespec time_wait;
    while ((rand() % 20 != 1) && (is_end != 1))
    {
        pthread_mutex_lock(&shm_data->mutex);
        int generate_value = rand() % 100;
        shm_data->trans_data = generate_value;
        printf("THREAD 1: generate value: %d\n", generate_value);
        shm_data->count_writes = 0;
        shm_data->version++;
        pthread_cond_broadcast(&shm_data->cond_to_readers);
        while ((is_end != 1) && (shm_data->count_writes < 2))
        {
            clock_gettime(CLOCK_REALTIME, &time_wait);
            time_wait.tv_sec += 10;
            rc = pthread_cond_timedwait(&shm_data->cond_to_writer, &shm_data->mutex, &time_wait);
        }
    
        if (rc != 0)
        {
            if (rc != ETIMEDOUT) perror("pthread_cond_timedwait: thread 1");
            else printf("THREAD 1: Ok, no reply, ending\n");

            is_end = 1;
            pthread_cond_broadcast(&shm_data->cond_to_readers);
        }
        pthread_mutex_unlock(&shm_data->mutex);
    }
    printf("THREAD 1: End working\n");
}

void *thread2_work()
{
    printf("THREAD 2: start working\n");
    int rc = 0;
    int local_version = -1;
    struct timespec time_wait;
    while ((is_end != 1))
    {
        pthread_mutex_lock(&shm_data->mutex);
        while ((shm_data->version == local_version) && (is_end != 1))
        {
            clock_gettime(CLOCK_REALTIME, &time_wait);
            time_wait.tv_sec += 10;
            rc = pthread_cond_timedwait(&shm_data->cond_to_readers, &shm_data->mutex, &time_wait);
            if (rc != 0) break;
        }

        if (rc != 0)
        {
            if (rc != ETIMEDOUT) perror("pthread_cond_timedwait: thread 2");
            else printf("THREAD 2: Ok, no working writing process, ending\n");

            is_end = 1;
            pthread_cond_broadcast(&shm_data->cond_to_writer);
        } else
        {
            if (shm_data->trans_data % 2 == 0) printf("THREAD 2: square value: %d\n", 
                shm_data->trans_data * shm_data->trans_data);
            shm_data->count_writes++;
            local_version = shm_data->version;
            if (shm_data->count_writes == 2) pthread_cond_broadcast(&shm_data->cond_to_writer);
        }
        pthread_mutex_unlock(&shm_data->mutex);
    }
    printf("THREAD 2: End working\n");
}

void *thread3_work()
{
    printf("THREAD 3: start working\n");
    int rc = 0;
    int local_version = -1;
    struct timespec time_wait;
    while ((is_end != 1))
    {
        pthread_mutex_lock(&shm_data->mutex);
        while ((shm_data->version == local_version) && (is_end != 1))
        {
            clock_gettime(CLOCK_REALTIME, &time_wait);
            time_wait.tv_sec += 10;
            rc = pthread_cond_timedwait(&shm_data->cond_to_readers, &shm_data->mutex, &time_wait);
            if (rc != 0) break;
        }

        if (rc != 0)
        {
            if (rc != ETIMEDOUT) perror("pthread_cond_timedwait: thread 2");
            else printf("THREAD 3: Ok, no working writing process, ending\n");

            is_end = 1;
            pthread_cond_broadcast(&shm_data->cond_to_writer);
        } else
        {
            if (shm_data->trans_data % 2 == 1) printf("THREAD 2: cubed value: %d\n", 
                shm_data->trans_data * shm_data->trans_data * shm_data->trans_data);
            shm_data->count_writes++;
            local_version = shm_data->version;
            if (shm_data->count_writes == 2) pthread_cond_broadcast(&shm_data->cond_to_writer);
        }
        pthread_mutex_unlock(&shm_data->mutex);
    }
    printf("THREAD 3: End working\n");
}
