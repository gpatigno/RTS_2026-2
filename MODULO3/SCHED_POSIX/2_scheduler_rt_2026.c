#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sched.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#define NUM_CYCLES 10
#define PERIOD_MS 100       // Período de la tarea: 100 ms
#define DEADLINE_MS 100     // Plazo de entrega: 100 ms

// Función auxiliar para calcular la diferencia en milisegundos
double timespec_to_ms(struct timespec t) {
    return (t.tv_sec * 1000.0) + (t.tv_nsec / 1000000.0);
}

void* rt_periodic_task() {
    struct timespec next_activation;
    struct timespec start_time, end_time;
    
    // Obtener el tiempo inicial
    clock_gettime(CLOCK_MONOTONIC, &next_activation);

    printf("\n");
    printf("\n--- Iniciando Tarea de Tiempo Real (SCHED_FIFO) ---\n");
    printf("\n");
    printf("%-10s %-15s %-15s %-20s %-10s\n", "Ciclo", "Inicio (ms)", "Fin (ms)", "Duración (ms)", "Estado");
    printf("-----------------------------------------------------------------------\n");

    for (int i = 0; i < NUM_CYCLES; i++) {
        // Esperar hasta el inicio del próximo período
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next_activation, NULL);
        
        // Registrar el tiempo de inicio real del ciclo
        clock_gettime(CLOCK_MONOTONIC, &start_time);

        // --- SIMULACIÓN DE CARGA DE TRABAJO ---
        // En los ciclos 4 y 8 simulamos una sobrecarga pesada (150ms) que causará un "Deadline Miss"
        if (i == 3 || i == 7) {
            usleep(150000); // 150 ms
        } else {
            usleep(40000);  // 40 ms (Carga normal, cumple el deadline)
        }
        // --------------------------------------

        // Registrar el tiempo de finalización
        clock_gettime(CLOCK_MONOTONIC, &end_time);

        // Calcular métricas
        double start_ms = timespec_to_ms(start_time);
        double end_ms = timespec_to_ms(end_time);
        double duration = end_ms - start_ms;

        // Verificar cumplimiento del Deadline
        const char* status = (duration <= DEADLINE_MS) ? "Cumple el Deadline" : "Falta (Miss) el Deadline";

        printf("%-10d %-15.2f %-17.2f %-13.2f %-10s\n", i + 1, start_ms, end_ms, duration, status);

        // Calcular la próxima activación absoluta de forma rígida para evitar la deriva temporal
        next_activation.tv_nsec += PERIOD_MS * 1000000;
        if (next_activation.tv_nsec >= 1000000000) {
            next_activation.tv_sec += 1;
            next_activation.tv_nsec -= 1000000000;
        }
    }

    return NULL;
}

int main() {
    pthread_t rt_thread;
    pthread_attr_t attr;
    struct sched_param param;
    int ret;

    // 1. Inicializar atributos del hilo
    ret = pthread_attr_init(&attr);
    if (ret != 0) {
        perror("Error inicializando atributos");
        exit(EXIT_FAILURE);
    }

    // 2. Establecer la herencia de la política de scheduling de forma explícita
    ret = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    if (ret != 0) {
        perror("Error en setinheritsched");
        exit(EXIT_FAILURE);
    }

    // 3. Definir la política de Tiempo Real: SCHED_FIFO
    ret = pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    if (ret != 0) {
        perror("Error configurando SCHED_FIFO");
        exit(EXIT_FAILURE);
    }

    // 4. Asignar la prioridad (SCHED_FIFO requiere al menos prioridad 1)
    param.sched_priority = 80; // Prioridad alta (rango típico 1-99)
    ret = pthread_attr_setschedparam(&attr, &param);
    if (ret != 0) {
        perror("Error configurando prioridad");
        exit(EXIT_FAILURE);
    }

    // 5. Crear el hilo. NOTA: Esto fallará si no se ejecuta como ROOT/SUDO
    ret = pthread_create(&rt_thread, &attr, rt_periodic_task, NULL);
    if (ret != 0) {
        fprintf(stderr, "Error al crear el hilo de RT: %s\n", strerror(ret));
        fprintf(stderr, "RECUERDE: Los cambios de política RT requieren privilegios de ROOT (ejecuta con sudo).\n");
        pthread_attr_destroy(&attr);
        exit(EXIT_FAILURE);
    }

    // Esperar a que el hilo termine
    pthread_join(rt_thread, NULL);
    pthread_attr_destroy(&attr);

    printf("\n--- Simulación finalizada ---\n");
    return 0;
}
