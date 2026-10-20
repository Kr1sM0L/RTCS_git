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


static int num_in_query = 0;
static pthread_mutex_t mutex;
static pthread_cond_t cond;
static volatile sig_atomic_t is_end = 0;


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

void *query_func()
{
    printf("QUERY: Start working.\n");
    while (is_end != 1)
    {
        pthread_mutex_lock(&mutex);
        printf("QUERY: Plus new client.\n");
        num_in_query++;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
        usleep((500 + rand() % 200) * 1000);
    }
    printf("QUERY: End working.\n");

}

void *human_func()
{
    printf("HUMAN: Start working.\n");
    int ret_val;
    while (is_end != 1)
    {
        pthread_mutex_lock(&mutex);
        while ((is_end != 1) && (num_in_query < 1))
        {
            ret_val = pthread_cond_wait(&cond, &mutex);
            if (ret_val != 0) break;
        }

        printf("HUMAN: Start working [num clients: %1d].\n", num_in_query);
        usleep((300 + rand() % 100) * 1000);
        num_in_query--;
        if (num_in_query == 0) printf("HUMAN: End service clien. I go sleep.\n");
        else printf("HUMAN: End service client. Go next.\n");
        pthread_mutex_unlock(&mutex);
    }
    printf("HUMAN: End working.\n");
}

int main()
{
    printf("Start working programm.\n");

    pthread_t query;
    pthread_t human;
    set_handler();
    
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    pthread_create(&human, NULL, human_func, NULL);
    pthread_create(&query, NULL, query_func, NULL);

    pthread_join(human, NULL);
    pthread_cond_signal(&cond);
    pthread_join(query, NULL);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    printf("End working program.\n");
}