#define _DEFAULT_SOURCE
#include "producer_consumer.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>


// Inicializar el buffer y semáforos
int init_buffer(ProducerConsumerBuffer *buffer) {
    if (!buffer) {
        fprintf(stderr, "Error: Buffer es NULL\n");
        return -1;
    }

    // Inicializar índices
    buffer->in = 0;
    buffer->out = 0;
    buffer->items_produced = 0;
    buffer->items_consumed = 0;
    buffer->shutdown = false;

    // Inicializar semáforos
    if (sem_init(&buffer->empty, 0, BUFFER_SIZE) != 0) {
        perror("Error inicializando semáforo empty");
        return -1;
    }

    if (sem_init(&buffer->full, 0, 0) != 0) {
        perror("Error inicializando semáforo full");
        sem_destroy(&buffer->empty);
        return -1;
    }

    // Inicializar mutex
    if (pthread_mutex_init(&buffer->mutex, NULL) != 0) {
        perror("Error inicializando mutex");
        sem_destroy(&buffer->empty);
        sem_destroy(&buffer->full);
        return -1;
    }

    // Inicializar buffer con valores -1 (vacío)
    for (int i = 0; i < BUFFER_SIZE; i++) {
        buffer->buffer[i] = -1;
    }

    printf("Buffer inicializado correctamente (tamaño: %d)\n", BUFFER_SIZE);
    return 0;
}

// Destruir el buffer y liberar recursos
void destroy_buffer(ProducerConsumerBuffer *buffer) {
    if (!buffer) return;

    buffer->shutdown = true;
    
    // Destruir semáforos
    sem_destroy(&buffer->empty);
    sem_destroy(&buffer->full);
    
    // Destruir mutex
    pthread_mutex_destroy(&buffer->mutex);
    
    printf("Buffer destruido correctamente\n");
}

// Función del productor
void *producer(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    ProducerConsumerBuffer *buffer = data->buffer;
    int thread_id = data->thread_id;
    int items_to_produce = data->items_to_produce;

    printf("Productor %d iniciado (producirá %d items)\n", thread_id, items_to_produce);

    for (int i = 0; i < items_to_produce && !buffer->shutdown; i++) {
        // Producir item
        int item = produce_item(thread_id, i);
        
        // Esperar por slot vacío
        if (sem_wait(&buffer->empty) != 0) {
            perror("Error en sem_wait(empty)");
            break;
        }

        // Verificar si debemos terminar
        if (buffer->shutdown) {
            sem_post(&buffer->empty);
            break;
        }

        // Entrar en sección crítica
        pthread_mutex_lock(&buffer->mutex);
        
        // Agregar item al buffer
        buffer->buffer[buffer->in] = item;
        printf("Productor %d: item %d agregado en posición %d\n", 
               thread_id, item, buffer->in);
        
        buffer->in = (buffer->in + 1) % BUFFER_SIZE;
        buffer->items_produced++;
        
        // Salir de sección crítica
        pthread_mutex_unlock(&buffer->mutex);
        
        // Señalar que hay un item disponible
        sem_post(&buffer->full);
        
        // Simular tiempo de producción
        usleep(100000 + (rand() % 200000)); // 0.1-0.3 segundos
    }

    printf("Productor %d terminado\n", thread_id);
    return NULL;
}

// Función del consumidor
void *consumer(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    ProducerConsumerBuffer *buffer = data->buffer;
    int thread_id = data->thread_id;

    printf("Consumidor %d iniciado\n", thread_id);

    while (!buffer->shutdown) {
        // Esperar por item disponible
        if (sem_wait(&buffer->full) != 0) {
            if (errno == EINTR) continue;
            perror("Error en sem_wait(full)");
            break;
        }

        // Verificar si debemos terminar
        if (buffer->shutdown) {
            sem_post(&buffer->full);
            break;
        }

        // Entrar en sección crítica
        pthread_mutex_lock(&buffer->mutex);
        
        // Verificar si realmente hay items (double-check)
        if (buffer->items_produced == buffer->items_consumed) {
            pthread_mutex_unlock(&buffer->mutex);
            sem_post(&buffer->full);
            continue;
        }
        
        // Extraer item del buffer
        int item = buffer->buffer[buffer->out];
        buffer->buffer[buffer->out] = -1; // Marcar como vacío
        printf("Consumidor %d: item %d extraído de posición %d\n", 
               thread_id, item, buffer->out);
        
        buffer->out = (buffer->out + 1) % BUFFER_SIZE;
        buffer->items_consumed++;
        
        // Salir de sección crítica
        pthread_mutex_unlock(&buffer->mutex);
        
        // Señalar que hay un slot libre
        sem_post(&buffer->empty);
        
        // Consumir item
        consume_item(item, thread_id);
        
        // Simular tiempo de consumo
        usleep(150000 + (rand() % 250000)); // 0.15-0.4 segundos
    }

    printf("Consumidor %d terminado\n", thread_id);
    return NULL;
}

// Función para producir un item
int produce_item(int thread_id, int item_number) {
    // Generar un item único basado en el thread_id y número de item
    int item = (thread_id * 1000) + item_number;
    return item;
}

// Función para consumir un item
void consume_item(int item, int thread_id) {
    printf("Consumidor %d procesando item %d\n", thread_id, item);
    // Simular procesamiento del item
    usleep(50000); // 0.05 segundos
}

// Funciones auxiliares
void print_buffer_status(ProducerConsumerBuffer *buffer) {
    pthread_mutex_lock(&buffer->mutex);
    
    printf("\n=== Estado del Buffer ===\n");
    printf("Buffer: [");
    for (int i = 0; i < BUFFER_SIZE; i++) {
        if (buffer->buffer[i] == -1) {
            printf(" _ ");
        } else {
            printf("%3d", buffer->buffer[i]);
        }
        if (i < BUFFER_SIZE - 1) printf(",");
    }
    printf("]\n");
    printf("In: %d, Out: %d\n", buffer->in, buffer->out);
    printf("Producidos: %d, Consumidos: %d\n", 
           buffer->items_produced, buffer->items_consumed);
    
    pthread_mutex_unlock(&buffer->mutex);
}

void print_statistics(ProducerConsumerBuffer *buffer) {
    pthread_mutex_lock(&buffer->mutex);
    
    printf("\n=== Estadísticas Finales ===\n");
    printf("Total items producidos: %d\n", buffer->items_produced);
    printf("Total items consumidos: %d\n", buffer->items_consumed);
    printf("Items pendientes: %d\n", buffer->items_produced - buffer->items_consumed);
    
    pthread_mutex_unlock(&buffer->mutex);
}

bool is_buffer_full(ProducerConsumerBuffer *buffer) {
    return ((buffer->in + 1) % BUFFER_SIZE) == buffer->out;
}

bool is_buffer_empty(ProducerConsumerBuffer *buffer) {
    return buffer->in == buffer->out;
}