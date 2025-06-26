#ifndef PRODUCER_CONSUMER_H
#define PRODUCER_CONSUMER_H

#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>

#define BUFFER_SIZE 10
#define MAX_ITEMS 100

// Estructura del buffer compartido
typedef struct {
    int buffer[BUFFER_SIZE];
    int in;                     // Índice para insertar
    int out;                    // Índice para extraer
    sem_t empty;               // Semáforo para slots vacíos
    sem_t full;                // Semáforo para slots llenos
    pthread_mutex_t mutex;     // Mutex para acceso exclusivo al buffer
    int items_produced;        // Contador de items producidos
    int items_consumed;        // Contador de items consumidos
    bool shutdown;             // Flag para terminar la ejecución
} ProducerConsumerBuffer;

// Estructura para pasar datos a los threads
typedef struct {
    ProducerConsumerBuffer *buffer;
    int thread_id;
    int items_to_produce;
} ThreadData;

// Funciones principales
int init_buffer(ProducerConsumerBuffer *buffer);
void destroy_buffer(ProducerConsumerBuffer *buffer);
void *producer(void *arg);
void *consumer(void *arg);
int produce_item(int thread_id, int item_number);
void consume_item(int item, int thread_id);

// Funciones auxiliares
void print_buffer_status(ProducerConsumerBuffer *buffer);
void print_statistics(ProducerConsumerBuffer *buffer);
bool is_buffer_full(ProducerConsumerBuffer *buffer);
bool is_buffer_empty(ProducerConsumerBuffer *buffer);

#endif // PRODUCER_CONSUMER_H