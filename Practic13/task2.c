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


#define SHM_KEY 1302
#define PERMS 0600
#define NUMBER_CATS 5
#define NUMBER_DAYS 10


struct shm_seg
{
    int eat_value;
    int tryies_eat;
    int live_cats;
    int current_day;
    int power_cats[NUMBER_CATS];
    int hungry_cats[NUMBER_CATS];
    pthread_mutex_t mutex;
    pthread_cond_t cond_to_cats;
    pthread_cond_t cond_to_human;
    pthread_t cats[NUMBER_CATS];
    pthread_t human;
};


volatile sig_atomic_t is_end = 0;
int lazy_value;
struct shm_seg *bowl;

void int_handler(int signum)
{
    printf("\nSignal %d accepted. Interrupting program.\n", signum);
    is_end = 1;
}

void *func_human()
{
    struct timespec time_wait;
    int rc = 0;
    while ((is_end != 1) && (bowl->current_day < NUMBER_DAYS))
    {
        pthread_mutex_lock(&bowl->mutex);
        bowl->current_day++;
        if (rand() % 100 < lazy_value)
        {
            printf("HUMAN: Mmm, i love sleep, dont refill the bowl [current eat_value: %d].\n", bowl->eat_value);
        } else
        {   
            bowl->eat_value += rand() % 250 + 500;
            if (bowl->eat_value > 1000) bowl->eat_value = 1000;
            printf("HUMAN: Oh no, my lovely cats, i refill the bowl [current eat_value: %d].\n", bowl->eat_value);
        }
        bowl->tryies_eat = 0;
        pthread_cond_broadcast(&bowl->cond_to_cats);
        while ((is_end != 1) && (bowl->tryies_eat < bowl->live_cats))
        {
            clock_gettime(CLOCK_REALTIME, &time_wait);
            time_wait.tv_sec += 10;
            pthread_cond_timedwait(&bowl->cond_to_human, &bowl->mutex, &time_wait);
            if (rc != 0) break;
        }
        pthread_mutex_unlock(&bowl->mutex);
    }
    is_end = 1;
    printf("HUMAN: Sending end signal.\n");
    pthread_cond_broadcast(&bowl->cond_to_cats);
    printf("HUMAN: Work done.\n");
}

void *func_cat(void *arg)
{
    int rc = 0;
    int current_day = 0;
    int id = (intptr_t)arg;
    while (is_end != 1)
    {
        pthread_mutex_lock(&bowl->mutex);
        while ((is_end != 1) && (bowl->current_day == current_day))
        {
            rc = pthread_cond_wait(&bowl->cond_to_cats, &bowl->mutex);
            if (rc != 0) break;
        }
        
        if ((bowl->power_cats[id] != -1) && (current_day < NUMBER_DAYS)) 
        {
            if (bowl->eat_value < 200)
            {
                bowl->hungry_cats[id] += 1;
                if (bowl->hungry_cats[id] > 5)
                {
                    printf("CAT %3d: Im very hungry. Im dying.\n", id);
                    if (bowl->tryies_eat >= bowl->live_cats) pthread_cond_signal(&bowl->cond_to_human);
                    pthread_mutex_unlock(&bowl->mutex);
                    return;
                } else if (bowl->hungry_cats[id] > 2)
                {
                    if (bowl->live_cats > 1)
                    {
                        int id_another_cat;
                        do
                        {
                            id_another_cat = rand() % NUMBER_CATS;
                        } while ((id_another_cat == id) || (bowl->power_cats[id_another_cat] == -1));
                        if (bowl->power_cats[id_another_cat] < bowl->power_cats[id])
                        {
                            printf("CAT %3d: Sorry another cat [%3d], im very hungry...\n", id, id_another_cat);
                            bowl->power_cats[id_another_cat] = -1;
                            bowl->hungry_cats[id] = 0;
                        } else printf("CAT %3d: Oh no, another cat [%3d] is more powerful than me.\n", id, id_another_cat);
                    } else printf("CAT %3d: Im hungry.\n", id);
                } else printf("CAT %3d: Im hungry.\n", id);
            } else
            {
                bowl->eat_value -= 100 + rand() % 20;
                if (bowl->eat_value < 100) bowl->eat_value = 100;
                printf("CAT %3d: Im eat [least eat_value: %d].\n", id, bowl->eat_value);
                bowl->hungry_cats[id] = 0;
            }

            current_day = bowl->current_day;
        }
        bowl->tryies_eat++;

        if (bowl->tryies_eat >= bowl->live_cats) pthread_cond_signal(&bowl->cond_to_human);
        if (bowl->power_cats[id] == -1)
        {
            printf("CAT %3d: I have been eaten.\n", id);
            bowl->live_cats--;
            pthread_mutex_unlock(&bowl->mutex);
            return;
        }
        pthread_mutex_unlock(&bowl->mutex);
    }
    printf("CAT %3d: Im alive. Ending.\n", id);
}

int main()
{
    if (signal(SIGINT, int_handler) == SIG_ERR)
    {
        perror("signal");
        return -1;
    }
    siginterrupt(SIGINT, 1);

    printf("Print lazy value of human (0 [- not lazy] to 100 [- over lazy]): ");
    scanf("%d", &lazy_value);

    printf("Starting programm.\n");

    int shm_id = shmget(SHM_KEY, sizeof(struct shm_seg), PERMS | IPC_CREAT | IPC_EXCL);
    if (shm_id < 0)
    {
        perror("shmget");
        return -1;
    }

    bowl = shmat(shm_id, NULL, 0);
    if (bowl == (void*)-1) {
        perror("shmat");
        is_end = 1;
    }

    while (is_end != 1)
    {
        pthread_mutexattr_t m_at;
        pthread_mutexattr_init(&m_at);
        pthread_mutexattr_setpshared(&m_at, PTHREAD_PROCESS_SHARED);
        pthread_mutex_init(&bowl->mutex, &m_at);

        pthread_condattr_t c_at;
        pthread_condattr_init(&c_at);
        pthread_condattr_setpshared(&c_at, PTHREAD_PROCESS_SHARED);
        pthread_cond_init(&bowl->cond_to_cats, &c_at);
        pthread_cond_init(&bowl->cond_to_human, &c_at);

        pthread_condattr_destroy(&c_at);
        pthread_mutexattr_destroy(&m_at);

        int rc;

        bowl->eat_value = 100;
        bowl->live_cats = NUMBER_CATS;
        bowl->tryies_eat = 0;
        bowl->current_day = 0;
        for (int i = 0; i < NUMBER_CATS; i++) bowl->power_cats[i] = rand() % 20 + 1;
        for (int i = 0; i < NUMBER_CATS; i++) bowl->hungry_cats[i] = 0;

        rc = pthread_create(&bowl->human, NULL, func_human, NULL);
        if (rc != 0)
        {
            perror("pthread_create: human");
            is_end = 1;
            break;
        }
        
        if (is_end == 1) break;
        for (intptr_t i = 0; i < NUMBER_CATS; i++)
        {
            rc = pthread_create(&bowl->cats[i], NULL, func_cat, (void*)i);
            if (rc != 0)
            {
                perror("pthread_create: cats");
                is_end = 1;
                break;
            }
        }

        pthread_join(bowl->human, NULL);
        for (int i = 0; i < NUMBER_CATS; i++)
        {
            pthread_join(bowl->cats[i], NULL);
        }


        pthread_mutex_destroy(&bowl->mutex);
        pthread_cond_destroy(&bowl->cond_to_cats);
        pthread_cond_destroy(&bowl->cond_to_human);
        
        printf("Working cycle is done! Closing all\n");
        if (shmdt(bowl) == -1)
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
}