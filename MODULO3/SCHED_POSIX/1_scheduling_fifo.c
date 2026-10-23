#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

void print_timestamp(const char* label, int tid, int prio) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    printf("[%ld.%06ld] %s - Thread %d (prio %d)\n",
           ts.tv_sec, ts.tv_nsec / 1000, label, tid, prio);
}

void *thread_func(void *arg) {
    int tid = ((int*)arg)[0];
    int prio = ((int*)arg)[1];

    print_timestamp("Start", tid, prio);
    for (int i = 0; i < 5; i++) {
        printf("Thread %d running (iteration %d)\n", tid, i+1);
        usleep(100000); // 100 ms
    }
    print_timestamp("End", tid, prio);
    pthread_exit(NULL);
}

int main() {
    pthread_t threads[3];
    pthread_attr_t attr[3];
    struct sched_param param[3];
    int thread_args[3][2] = {{1, 10}, {2, 20}, {3, 30}}; // {tid, prio}
    cpu_set_t cpuset;

    int priorities[3] = {10, 20, 30};  // SCHED_FIFO priorities

    for (int i = 0; i < 3; i++) {
        pthread_attr_init(&attr[i]);
        pthread_attr_setinheritsched(&attr[i], PTHREAD_EXPLICIT_SCHED);
        pthread_attr_setschedpolicy(&attr[i], SCHED_FIFO);

        param[i].sched_priority = priorities[i];
        pthread_attr_setschedparam(&attr[i], &param[i]);

        // Crear el hilo
        if (pthread_create(&threads[i], &attr[i], thread_func, thread_args[i]) != 0) {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }

        // Afinidad al CPU 0
        CPU_ZERO(&cpuset);
        CPU_SET(0, &cpuset);
        if (pthread_setaffinity_np(threads[i], sizeof(cpu_set_t), &cpuset) != 0) {
            perror("pthread_setaffinity_np");
            exit(EXIT_FAILURE);
        }

        pthread_attr_destroy(&attr[i]);
    }

    // Esperar a los hilos
    for (int i = 0; i < 3; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
