#include "producer_consumer.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>

#define NUM_PRODUCERS 3
#define NUM_CONSUMERS 2
#define ITEMS_PER_PRODUCER 10

// Variables globales para manejo de señales
static ProducerConsumerBuffer *global_buffer = NULL;
static pthread_t producer_threads[NUM_PRODUCERS];
static pthread_t consumer_threads[NUM_CONSUMERS];
static ThreadData producer_data[NUM_PRODUCERS];
static ThreadData consumer_data[NUM_CONSUMERS];

// Manejador de señales para terminación limpia
void signal_handler(int sig) {
    printf("\n\nRecibida señal %d. Terminando programa...\n", sig);
    if (global_buffer) {
        global_buffer->shutdown = true;
    }
}

// Test básico de funcionalidad
int test_basic_functionality() {
    printf("\n=== Probando Funcionalidad Básica ===\n");
    
    ProducerConsumerBuffer buffer;
    if (init_buffer(&buffer) != 0) {
        printf("❌ Error inicializando buffer\n");
        return -1;
    }
    
    // Test simple: un productor, un consumidor
    ThreadData prod_data = {&buffer, 0, 3};
    ThreadData cons_data = {&buffer, 0, 0};
    
    pthread_t prod_thread, cons_thread;
    
    // Crear threads
    if (pthread_create(&cons_thread, NULL, consumer, &cons_data) != 0) {
        perror("Error creando consumidor");
        destroy_buffer(&buffer);
        return -1;
    }
    
    if (pthread_create(&prod_thread, NULL, producer, &prod_data) != 0) {
        perror("Error creando productor");
        buffer.shutdown = true;
        pthread_join(cons_thread, NULL);
        destroy_buffer(&buffer);
        return -1;
    }
    
    // Esperar a que termine el productor
    pthread_join(prod_thread, NULL);
    
    // Dar tiempo para que el consumidor procese
    sleep(1);
    
    // Terminar consumidor
    buffer.shutdown = true;
    sem_post(&buffer.full); // Despertar al consumidor
    pthread_join(cons_thread, NULL);
    
    print_statistics(&buffer);
    
    bool success = (buffer.items_produced == 3 && buffer.items_consumed == 3);
    printf("Prueba básica: %s\n", success ? "✅ EXITOSA" : "❌ FALLÓ");
    
    destroy_buffer(&buffer);
    return success ? 0 : -1;
}

// Test multi-threaded completo
int test_multi_threaded() {
    printf("\n=== Probando Operaciones Multi-threaded ===\n");
    
    ProducerConsumerBuffer buffer;
    if (init_buffer(&buffer) != 0) {
        printf("❌ Error inicializando buffer\n");
        return -1;
    }
    
    global_buffer = &buffer;
    
    printf("Iniciando %d productores y %d consumidores\n", NUM_PRODUCERS, NUM_CONSUMERS);
    printf("Cada productor producirá %d elementos\n", ITEMS_PER_PRODUCER);
    
    // Configurar datos para threads
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        producer_data[i].buffer = &buffer;
        producer_data[i].thread_id = i;
        producer_data[i].items_to_produce = ITEMS_PER_PRODUCER;
    }
    
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        consumer_data[i].buffer = &buffer;
        consumer_data[i].thread_id = i;
        consumer_data[i].items_to_produce = 0; // No usado por consumidores
    }
    
    // Crear threads consumidores primero
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        if (pthread_create(&consumer_threads[i], NULL, consumer, &consumer_data[i]) != 0) {
            perror("Error creando thread consumidor");
            buffer.shutdown = true;
            return -1;
        }
        printf("Consumidor %d iniciado\n", i);
    }
    
    // Crear threads productores
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        if (pthread_create(&producer_threads[i], NULL, producer, &producer_data[i]) != 0) {
            perror("Error creando thread productor");
            buffer.shutdown = true;
            return -1;
        }
        printf("Productor %d iniciado\n", i);
    }
    
    // Esperar a que terminen todos los productores
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        pthread_join(producer_threads[i], NULL);
    }
    
    printf("\nTodos los productores han terminado. Esperando a consumidores...\n");
    
    // Esperar un tiempo para que los consumidores procesen todo
    int timeout = 10; // 10 segundos máximo
    while (timeout > 0 && buffer.items_consumed < buffer.items_produced) {
        sleep(1);
        timeout--;
        printf("Esperando... Producidos: %d, Consumidos: %d\n", 
               buffer.items_produced, buffer.items_consumed);
    }
    
    // Terminar consumidores
    buffer.shutdown = true;
    
    // Despertar a todos los consumidores
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        sem_post(&buffer.full);
    }
    
    // Esperar a que terminen los consumidores
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        pthread_join(consumer_threads[i], NULL);
    }
    
    print_statistics(&buffer);
    
    int expected = NUM_PRODUCERS * ITEMS_PER_PRODUCER;
    bool success = (buffer.items_produced == expected && 
                   buffer.items_consumed == expected);
    
    printf("\nTotal esperado: %d\n", expected);
    printf("Prueba multi-threaded: %s\n", success ? "✅ EXITOSA" : "❌ FALLÓ");
    
    destroy_buffer(&buffer);
    global_buffer = NULL;
    
    return success ? 0 : -1;
}

int main() {
    printf("Programa de Prueba Producer-Consumer\n");
    printf("===================================\n");
    
    // Configurar semilla para números aleatorios
    srand(time(NULL));
    
    // Configurar manejador de señales
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    int result = 0;
    
    // Ejecutar tests
    if (test_basic_functionality() != 0) {
        result = -1;
    }
    
    if (test_multi_threaded() != 0) {
        result = -1;
    }
    
    if (result == 0) {
        printf("\n🎉 ¡Todas las pruebas completadas exitosamente!\n");
    } else {
        printf("\n❌ Algunas pruebas fallaron\n");
    }
    
    return result;
}