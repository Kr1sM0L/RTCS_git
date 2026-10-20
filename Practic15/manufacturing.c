#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

sem_t sem_a;
sem_t sem_b;
sem_t sem_c;
sem_t sem_module;

void* produce_a(void* arg) {
    while (1) {
        sleep(1);
        printf("- Detail A created\n");
        sem_post(&sem_a);
    }
    return NULL;
}

void* produce_b(void* arg) {
    while (1) {
        sleep(2);
        printf("- Detail B created\n");
        sem_post(&sem_b);
    }
    return NULL;
}

void* produce_c(void* arg) {
    while (1) {
        sleep(3);
        printf("- Detail C created\n");
        sem_post(&sem_c);
    }
    return NULL;
}

void* assemble_module(void* arg) {
    while (1) {
        sem_wait(&sem_a);
        sem_wait(&sem_b);
        printf("--- Module (A + B) assembled\n");
        sem_post(&sem_module);
    }
    return NULL;
}

void* assemble_product(void* arg) {
    int count = 0;
    while (1) {
        sem_wait(&sem_module);
        sem_wait(&sem_c);
        count++;
        printf("----- PRODUCT #%d assembled (Module + C)\n\n", count);
    }
    return NULL;
}

int main() {
    pthread_t thread_a, thread_b, thread_c, thread_module, thread_product;

    sem_init(&sem_a, 0, 0);
    sem_init(&sem_b, 0, 0);
    sem_init(&sem_c, 0, 0);
    sem_init(&sem_module, 0, 0);

    pthread_create(&thread_a, NULL, produce_a, NULL);
    pthread_create(&thread_b, NULL, produce_b, NULL);
    pthread_create(&thread_c, NULL, produce_c, NULL);
    pthread_create(&thread_module, NULL, assemble_module, NULL);
    pthread_create(&thread_product, NULL, assemble_product, NULL);

    pthread_join(thread_product, NULL);

    sem_destroy(&sem_a);
    sem_destroy(&sem_b);
    sem_destroy(&sem_c);
    sem_destroy(&sem_module);

    return 0;
}