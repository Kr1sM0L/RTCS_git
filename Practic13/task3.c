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


#define NUM_PHIL 5


static volatile sig_atomic_t is_end = 0;
static pthread_mutex_t mutex_fork[NUM_PHIL];
static pthread_mutex_t mutex_all;
static pthread_cond_t cond_fork;
static int varient = 1;


void int_handler(int signo)
{
    (void)signo;
    fprintf(stdout, "\nSignal %d accepted. End flag in up state.\n", signo);
    is_end = 1;
}

void set_handler()
{
    struct sigaction sa;
    sa.sa_handler = int_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
}

int to_mikroseconds()
{
    struct timeval curr_time;
    gettimeofday(&curr_time, NULL);
    long long mil_sec = curr_time.tv_sec * 1000000 + curr_time.tv_usec;
    return mil_sec;
}

void *phil_function(void *arg)
{
    int ret_val;
    int id = (intptr_t)arg;
    long long mil_sec_start;
    long long mil_sec_delay;
    struct timespec time_wait;
    int curr_varient = 0;
    while (is_end != 1)
    {
        mil_sec_start = to_mikroseconds();
        pthread_mutex_lock(&mutex_all);
        while ((is_end != 1) && (varient == curr_varient))
        {
            clock_gettime(CLOCK_REALTIME, &time_wait);
            time_wait.tv_sec += 1;
            ret_val = pthread_cond_timedwait(&cond_fork, &mutex_all, &time_wait);
            if (ret_val != 0) break;
        }

        if (is_end == 1)
        {
            pthread_mutex_unlock(&mutex_all);
            break;
        }

        curr_varient = varient;
        ret_val = pthread_mutex_trylock(&mutex_fork[id]);
        if ((ret_val != 0) || (is_end == 1))
        {
            if (is_end != 1) printf("PHIL %1d: LEFT fork is busy. Go down.\n", id);
            pthread_cond_broadcast(&cond_fork);
            pthread_mutex_unlock(&mutex_all);
        } else
        {
            ret_val = pthread_mutex_trylock(&mutex_fork[(id + 1) % NUM_PHIL]);
            if ((ret_val != 0) || (is_end == 1))
            {
                if (is_end != 1) printf("PHIL %1d: RIGHT fork is busy. Go down.\n", id);
                pthread_mutex_unlock(&mutex_fork[id]);
                pthread_cond_broadcast(&cond_fork);
                pthread_mutex_unlock(&mutex_all);
            } else
            {
                // Eating
                varient++;
                pthread_cond_broadcast(&cond_fork);
                pthread_mutex_unlock(&mutex_all);
                mil_sec_delay = to_mikroseconds() - mil_sec_start;
                printf("PHIL %1d: Yes, im eating [delay to start eat: %lld].\n", id, mil_sec_delay);
                usleep(500 * 1000);

                // Give out all
                pthread_mutex_unlock(&mutex_fork[id]);
                pthread_mutex_unlock(&mutex_fork[(id + 1) % NUM_PHIL]);

                // Reasoning
                usleep(350 * 1000);
            }
        }
    }
    printf("PHIL %1d: Done working.\n", id);
}

int main()
{
    printf("Program start working.\n");
    set_handler();
    pthread_t phil_thread[NUM_PHIL];

    if (is_end != 1)
    {
        printf("Start initializing threads, mutex and conds.\n");

        for (int i = 0; i < NUM_PHIL; i++)
            pthread_mutex_init(&mutex_fork[i], NULL);

        pthread_mutex_init(&mutex_all, NULL);
        pthread_cond_init(&cond_fork, NULL);

        for (intptr_t i = 0; i < NUM_PHIL; i++)
        {
            pthread_create(&phil_thread[i], NULL, phil_function, (void*)i);
        }
        pthread_cond_broadcast(&cond_fork);

        for (int i = 0; i < NUM_PHIL; i++)
        {
            pthread_join(phil_thread[i], NULL);
        }

        for (int i = 0; i < NUM_PHIL; i++)
        {
            pthread_mutex_destroy(&mutex_fork[i]);
        }
        pthread_mutex_destroy(&mutex_all);
        pthread_cond_destroy(&cond_fork);
    }
}