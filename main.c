#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include "functions.h"

int main() {
    srand(time(NULL)); // Inicializa o gerador de números aleatórios

    pthread_t sensor_threads[SIZE][SIZE];
    pthread_t fire_thread, control_thread, printer_thread_id;

    initialize_forest();

    // Cria a thread da central de controle
    pthread_create(&control_thread, NULL, control_center, NULL);

    // Cria as threads dos nós sensores
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            int* coords = malloc(2 * sizeof(int));
            coords[0] = i;
            coords[1] = j;
            pthread_create(&sensor_threads[i][j], NULL, sensor_node, (void*)coords);
        }
    }

    // Cria a thread geradora de incêndios
    pthread_create(&fire_thread, NULL, fire_generator, NULL);

    // Cria a thread responsável por imprimir o estado atual e os logs
    pthread_create(&printer_thread_id, NULL, printer_thread, NULL);

    // Aguarda as threads principais (no exemplo, elas rodam indefinidamente)
    pthread_join(fire_thread, NULL);
    pthread_join(control_thread, NULL);
    pthread_join(printer_thread_id, NULL);

    // Finaliza as threads dos nós sensores
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            pthread_cancel(sensor_threads[i][j]);
        }
    }

    return 0;
}
