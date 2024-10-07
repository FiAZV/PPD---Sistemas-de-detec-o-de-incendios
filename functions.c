#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdarg.h>
#include "functions.h"

char forest[SIZE][SIZE]; // Matriz que representa a floresta
pthread_mutex_t forest_mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex para proteger o acesso à floresta

// Mutexes e variáveis de condição para comunicação entre threads
pthread_mutex_t message_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t message_cond = PTHREAD_COND_INITIALIZER;
int message_available = 0;
int message_x, message_y;

// Buffer circular para armazenar logs recentes
#define LOG_BUFFER_SIZE 5
char log_buffer[LOG_BUFFER_SIZE][256];
int log_start = 0;
int log_count = 0;
pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t print_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;
int can_print = 0;

void initialize_forest() {
    pthread_mutex_lock(&forest_mutex);
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            forest[i][j] = 'T'; // Inicializa todas as células com sensores ativos
        }
    }
    pthread_mutex_unlock(&forest_mutex);
}

void clear_console() {
    // Detecta o sistema operacional e executa o comando apropriado
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void print_forest_and_logs() {
    pthread_mutex_lock(&forest_mutex);
    pthread_mutex_lock(&log_mutex);

    clear_console(); // Limpa o console antes de imprimir

    // Exibe o estado atual da floresta
    printf("Estado Atual da Floresta:\n");
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            printf("%c ", forest[i][j]);
        }
        printf("\n");
    }

    // Exibe os logs mais recentes
    printf("\n---- Logs Recentes ----\n");
    for (int i = 0; i < log_count; i++) {
        int index = (log_start + i) % LOG_BUFFER_SIZE;
        printf("%s\n", log_buffer[index]);
    }
    printf("-----------------------\n");

    pthread_mutex_unlock(&log_mutex);
    pthread_mutex_unlock(&forest_mutex);
}

void add_log(const char* format, ...) {
    pthread_mutex_lock(&log_mutex);

    va_list args;
    va_start(args, format);
    vsnprintf(log_buffer[(log_start + log_count) % LOG_BUFFER_SIZE], 256, format, args);
    va_end(args);

    if (log_count < LOG_BUFFER_SIZE) {
        log_count++;
    } else {
        log_start = (log_start + 1) % LOG_BUFFER_SIZE;
    }

    pthread_mutex_unlock(&log_mutex);
}

void* sensor_node(void* arg) {
    int* coords = (int*)arg;
    int x = coords[0];
    int y = coords[1];
    free(coords);

    while (1) {
        pthread_mutex_lock(&forest_mutex);
        char state = forest[x][y];

        if (state == '@') {
            // Sensor detecta incêndio
            // Se for borda, envia para a central
            if (x == 0 || x == SIZE - 1 || y == 0 || y == SIZE - 1) {
                send_message_to_control_center(x, y);
            }
        }
        pthread_mutex_unlock(&forest_mutex);
        sleep(1);
    }

    return NULL;
}

void* fire_generator(void* arg) {
    while (1) {
        sleep(FIRE_INTERVAL);

        int x = rand() % SIZE;
        int y = rand() % SIZE;

        pthread_mutex_lock(&forest_mutex);
        if (forest[x][y] == 'T' || forest[x][y] == '-') {
            forest[x][y] = '@'; // Inicia um incêndio
            add_log("Incendio iniciado em [%d, %d].", x, y);
        }
        pthread_mutex_unlock(&forest_mutex);

        // Sinaliza que pode imprimir
        pthread_mutex_lock(&print_mutex);
        can_print = 1;
        pthread_cond_signal(&print_cond);
        pthread_mutex_unlock(&print_mutex);
    }

    return NULL;
}

void* control_center(void* arg) {
    while (1) {
        int x, y;
        // Aguarda mensagens dos sensores de borda
        receive_message_from_sensors(&x, &y);
        add_log("Incendio detectado em [%d, %d]. Iniciando combate ao fogo.", x, y);
        fight_fire(x, y);

        // Sinaliza que pode imprimir
        pthread_mutex_lock(&print_mutex);
        can_print = 1;
        pthread_cond_signal(&print_cond);
        pthread_mutex_unlock(&print_mutex);
    }

    return NULL;
}

void* printer_thread(void* arg) {
    while (1) {
        // Aguarda sinal para imprimir
        pthread_mutex_lock(&print_mutex);
        while (!can_print) {
            pthread_cond_wait(&print_cond, &print_mutex);
        }
        can_print = 0;
        pthread_mutex_unlock(&print_mutex);

        print_forest_and_logs();

        sleep(DISPLAY_INTERVAL); // Pausa para o usuário ler as informações
    }
    return NULL;
}

void send_message_to_control_center(int x, int y) {
    pthread_mutex_lock(&message_mutex);
    message_x = x;
    message_y = y;
    message_available = 1;
    pthread_cond_signal(&message_cond);
    pthread_mutex_unlock(&message_mutex);
}

void receive_message_from_sensors(int* x, int* y) {
    pthread_mutex_lock(&message_mutex);
    while (!message_available) {
        pthread_cond_wait(&message_cond, &message_mutex);
    }
    *x = message_x;
    *y = message_y;
    message_available = 0;
    pthread_mutex_unlock(&message_mutex);
}

void fight_fire() {
    pthread_mutex_lock(&forest_mutex);
    int fires_combated = 0;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            if (forest[i][j] == '@') {
                forest[i][j] = '/';
                fires_combated++;
            }
        }
    }
    if (fires_combated > 0) {
        add_log("Central de Controle: Fogo combatido em toda a floresta. Total de celulas afetadas: %d.", fires_combated);
    } else {
        add_log("Central de Controle: Nenhum fogo encontrado para combater.");
    }
    pthread_mutex_unlock(&forest_mutex);
}
