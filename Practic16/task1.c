#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <time.h>

#define QUEUE_CAPACITY 10
#define MSG_MAX_LEN    80

typedef struct queue {
    char data[QUEUE_CAPACITY][MSG_MAX_LEN + 1];

    int head;
    int tail;
    int count;

    int dropped;

    int waiting_get;
    int waiting_put;

    sem_t mutex;
    sem_t items;
    sem_t slots;
} queue;

static int sem_wait_nointr(sem_t *s)
{
    while (sem_wait(s) == -1)
    {
        if (errno != EINTR)
        {
            return -1;
        }
    }
    return 0;
}

static size_t bounded_strlen(const char *s, size_t maxlen)
{
    size_t i = 0;

    while (i < maxlen && s[i] != '\0')
    {
        i++;
    }

    return i;
}

void mymsginit(queue *q) {
    q->head = 0;
    q->tail = 0;
    q->count = 0;
    q->dropped = 0;
    q->waiting_get = 0;
    q->waiting_put = 0;

    sem_init(&q->mutex, 0, 1);
    sem_init(&q->items, 0, 0);
    sem_init(&q->slots, 0, QUEUE_CAPACITY);
}

void mymsgdrop(queue *q) {
    sem_wait_nointr(&q->mutex);

    if (!q->dropped) {
        q->dropped = 1;

        for (int i = 0; i < q->waiting_get; i++) {
            sem_post(&q->items);
        }

        for (int i = 0; i < q->waiting_put; i++) {
            sem_post(&q->slots);
        }
    }

    sem_post(&q->mutex);
}

void mymsqdrop(queue *q) {
    mymsgdrop(q);
}

void mymsgdestroy(queue *q)
{
    sem_destroy(&q->mutex);
    sem_destroy(&q->items);
    sem_destroy(&q->slots);
}

int mymsgput(queue *q, char *msg)
{
    if (msg == NULL)
    {
        return 0;
    }

    size_t len = bounded_strlen(msg, MSG_MAX_LEN);
    sem_wait_nointr(&q->mutex);

    if (q->dropped)
    {
        sem_post(&q->mutex);
        return 0;
    }

    q->waiting_put++;
    sem_post(&q->mutex);
    sem_wait_nointr(&q->slots);
    sem_wait_nointr(&q->mutex);
    q->waiting_put--;

    if (q->dropped)
    {
        sem_post(&q->mutex);
        return 0;
    }

    memcpy(q->data[q->tail], msg, len);
    q->data[q->tail][len] = '\0';
    q->tail = (q->tail + 1) % QUEUE_CAPACITY;
    q->count++;

    sem_post(&q->mutex);

    sem_post(&q->items);

    return (int)len;
}

int mymsgget(queue *q, char *buf, size_t bufsize)
{
    char local[MSG_MAX_LEN + 1];

    sem_wait_nointr(&q->mutex);

    if (q->dropped)
    {
        sem_post(&q->mutex);
        return 0;
    }

    q->waiting_get++;
    sem_post(&q->mutex);
    sem_wait_nointr(&q->items);
    sem_wait_nointr(&q->mutex);
    q->waiting_get--;

    if (q->dropped)
    {
        sem_post(&q->mutex);
        return 0;
    }

    strcpy(local, q->data[q->head]);
    q->head = (q->head + 1) % QUEUE_CAPACITY;
    q->count--;
    sem_post(&q->mutex);
    sem_post(&q->slots);

    if (bufsize == 0)
    {
        return 0;
    }

    size_t msglen = strlen(local);
    size_t copylen = msglen;

    if (copylen >= bufsize)
    {
        copylen = bufsize - 1;
    }

    memcpy(buf, local, copylen);
    buf[copylen] = '\0';

    return (int)copylen;
}


typedef struct
{
    queue *q;
    int id;
    int messages_count;
} producer_arg;

typedef struct
{
    queue *q;
    int id;
    sem_t *consumed_sem;
} consumer_arg;

static void sleep_ms(int ms)
{
    struct timespec ts;

    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (long)(ms % 1000) * 1000000L;

    while (nanosleep(&ts, &ts) == -1 && errno == EINTR);
}

void *producer_thread(void *arg) {
    producer_arg *pa = (producer_arg *)arg;

    for (int i = 0; i < pa->messages_count; i++) {
        char msg[256];

        snprintf(
            msg,
            sizeof(msg),
            "PRODUCER %04d: Message %02d: long tail",
            pa->id,
            i + 1
        );

        int written = mymsgput(pa->q, msg);

        if (written == 0)
        {
            printf("PRODUCER %04d: Queue dropped, stop\n", pa->id);
            break;
        }

        printf("PRODUCER %04d: Put %d chars\n", pa->id, written);
        sleep_ms(30);
    }

    printf("PRODUCER %04d: Finished\n", pa->id);

    return NULL;
}

void *consumer_thread(void *arg)
{
    consumer_arg *ca = (consumer_arg *)arg;

    while (1)
    {
        char buf[64];

        int read = mymsgget(ca->q, buf, sizeof(buf));

        if (read == 0)
        {
            printf("Consumer %04d: Queue dropped, stop\n", ca->id);
            break;
        }

        printf("Consumer %04d: Got %d chars: \"%s\"\n", ca->id, read, buf);

        sem_post(ca->consumed_sem);

        sleep_ms(120);
    }

    return NULL;
}

int main(void)
{
    queue q;
    mymsginit(&q);

    enum
    {
        PRODUCERS = 1,
        CONSUMERS = 3,
        MESSAGES_PER_PRODUCER = 15
    };

    const int total_messages = PRODUCERS * MESSAGES_PER_PRODUCER;

    pthread_t producers[PRODUCERS];
    pthread_t consumers[CONSUMERS];

    producer_arg pargs[PRODUCERS];
    consumer_arg cargs[CONSUMERS];

    sem_t consumed_sem;
    sem_init(&consumed_sem, 0, 0);

    for (int i = 0; i < CONSUMERS; i++)
    {
        cargs[i].q = &q;
        cargs[i].id = i + 1;
        cargs[i].consumed_sem = &consumed_sem;

        pthread_create(&consumers[i], NULL, consumer_thread, &cargs[i]);
    }

    for (int i = 0; i < PRODUCERS; i++)
    {
        pargs[i].q = &q;
        pargs[i].id = i + 1;
        pargs[i].messages_count = MESSAGES_PER_PRODUCER;

        pthread_create(&producers[i], NULL, producer_thread, &pargs[i]);
    }

    for (int i = 0; i < PRODUCERS; i++)
    {
        pthread_join(producers[i], NULL);
    }

    for (int i = 0; i < total_messages; i++)
    {
        sem_wait_nointr(&consumed_sem);
    }

    printf("\nAll messages consumed, dropping queue\n\n");

    mymsgdrop(&q);

    for (int i = 0; i < CONSUMERS; i++)
    {
        pthread_join(consumers[i], NULL);
    }

    sem_destroy(&consumed_sem);
    mymsgdestroy(&q);

    printf("Done\n");

    return 0;
}