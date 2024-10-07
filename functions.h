#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <stdarg.h>

#define SIZE 30
#define FIRE_INTERVAL 5 // Intervalo em segundos para gerar novos incêndios
#define DISPLAY_INTERVAL 5 // Intervalo em segundos para exibir as informações

void initialize_forest();
void* sensor_node(void* arg);
void* fire_generator(void* arg);
void* control_center(void* arg);
void* printer_thread(void* arg);

void send_message_to_control_center(int x, int y);
void receive_message_from_sensors(int* x, int* y);
void fight_fire();

void clear_console();
void add_log(const char* format, ...);
void print_forest_and_logs();

#endif
