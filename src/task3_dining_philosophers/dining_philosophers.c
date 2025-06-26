#define _DEFAULT_SOURCE
#include "dining_philosophers.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <string.h>

// Inicializar la mesa de comedor
int init_dining_table(DiningTable *table) {
    if (!table) {
        fprintf(stderr, "Error: Table es NULL\n");
        return -1;
    }

    // Inicializar filósofos
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        table->philosophers[i].id = i;
        table->philosophers[i].state = THINKING;
        table->philosophers[i].eating_count = 0;
        table->philosophers[i].total_thinking_time = 0;
        table->philosophers[i].total_eating_time = 0;
    }

    // Inicializar mutexes para tenedores
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        if (pthread_mutex_init(&table->forks[i], NULL) != 0) {
            perror("Error inicializando mutex de tenedor");
            // Cleanup mutexes ya inicializados
            for (int j = 0; j < i; j++) {
                pthread_mutex_destroy(&table->forks[j]);
            }
            return -1;
        }
    }

    // Inicializar mutex de estado
    if (pthread_mutex_init(&table->state_mutex, NULL) != 0) {
        perror("Error inicializando state_mutex");
        for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
            pthread_mutex_destroy(&table->forks[i]);
        }
        return -1;
    }

    // Inicializar condition variables
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        if (pthread_cond_init(&table->condition[i], NULL) != 0) {
            perror("Error inicializando condition variable");
            // Cleanup
            for (int j = 0; j < i; j++) {
                pthread_cond_destroy(&table->condition[j]);
            }
            pthread_mutex_destroy(&table->state_mutex);
            for (int j = 0; j < NUM_PHILOSOPHERS; j++) {
                pthread_mutex_destroy(&table->forks[j]);
            }
            return -1;
        }
    }

    // Inicializar semáforo del comedor (máximo 4 filósofos pueden intentar comer)
    if (sem_init(&table->dining_room, 0, NUM_PHILOSOPHERS - 1) != 0) {
        perror("Error inicializando semáforo dining_room");
        // Cleanup
        for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
            pthread_cond_destroy(&table->condition[i]);
            pthread_mutex_destroy(&table->forks[i]);
        }
        pthread_mutex_destroy(&table->state_mutex);
        return -1;
    }

    // Inicializar mutex de estadísticas
    if (pthread_mutex_init(&table->stats_mutex, NULL) != 0) {
        perror("Error inicializando stats_mutex");
        sem_destroy(&table->dining_room);
        for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
            pthread_cond_destroy(&table->condition[i]);
            pthread_mutex_destroy(&table->forks[i]);
        }
        pthread_mutex_destroy(&table->state_mutex);
        return -1;
    }

    table->simulation_running = true;
    table->total_meals_served = 0;

    printf("Mesa de comedor inicializada correctamente con %d filósofos\n", NUM_PHILOSOPHERS);
    return 0;
}

// Destruir recursos
void destroy_dining_table(DiningTable *table) {
    if (!table) return;

    table->simulation_running = false;

    // Despertar a todos los filósofos
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_cond_broadcast(&table->condition[i]);
    }

    // Destruir recursos
    sem_destroy(&table->dining_room);
    pthread_mutex_destroy(&table->stats_mutex);
    pthread_mutex_destroy(&table->state_mutex);
    
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_cond_destroy(&table->condition[i]);
        pthread_mutex_destroy(&table->forks[i]);
    }

    printf("Mesa de comedor destruida correctamente\n");
}

// Función principal del filósofo
void *philosopher_life(void *arg) {
    Philosopher *phil = (Philosopher *)arg;
    DiningTable *table = (DiningTable *)((char *)phil - 
        phil->id * sizeof(Philosopher));

    printf("🧠 Filósofo %d comenzó a pensar\n", phil->id);

    while (table->simulation_running && phil->eating_count < MAX_EATING_CYCLES) {
        // Pensar
        think(phil, table);
        
        if (!table->simulation_running) break;

        // Intentar comer usando solución con semáforo
        semaphore_solution(phil, table);
    }

    printf("🏁 Filósofo %d terminó (comió %d veces)\n", phil->id, phil->eating_count);
    return NULL;
}

// Función de pensar
void think(Philosopher *phil, DiningTable *table) {
    pthread_mutex_lock(&table->state_mutex);
    phil->state = THINKING;
    pthread_mutex_unlock(&table->state_mutex);

    printf("🤔 Filósofo %d está pensando\n", phil->id);
    
    int thinking_time = THINKING_TIME_MS + (rand() % THINKING_TIME_MS);
    usleep(thinking_time * 1000);
    
    phil->total_thinking_time += thinking_time;
}

// Tomar tenedores
void pickup_forks(Philosopher *phil, DiningTable *table) {
    pthread_mutex_lock(&table->state_mutex);
    
    phil->state = HUNGRY;
    printf("😋 Filósofo %d tiene hambre\n", phil->id);
    
    test_philosopher(phil->id, table);
    
    while (phil->state != EATING && table->simulation_running) {
        pthread_cond_wait(&table->condition[phil->id], &table->state_mutex);
    }
    
    pthread_mutex_unlock(&table->state_mutex);
}

// Función de comer
void eat(Philosopher *phil, DiningTable *table) {
    printf("🍽️  Filósofo %d está comiendo (comida #%d)\n", 
           phil->id, phil->eating_count + 1);
    
    int eating_time = EATING_TIME_MS + (rand() % EATING_TIME_MS);
    usleep(eating_time * 1000);
    
    phil->eating_count++;
    phil->total_eating_time += eating_time;
    
    pthread_mutex_lock(&table->stats_mutex);
    table->total_meals_served++;
    pthread_mutex_unlock(&table->stats_mutex);
}

// Dejar tenedores
void putdown_forks(Philosopher *phil, DiningTable *table) {
    pthread_mutex_lock(&table->state_mutex);
    
    phil->state = THINKING;
    printf("✅ Filósofo %d dejó los tenedores\n", phil->id);
    
    // Permitir que los vecinos intenten comer
    test_philosopher(left_fork(phil->id), table);
    test_philosopher(right_fork(phil->id), table);
    
    pthread_mutex_unlock(&table->state_mutex);
}

// Probar si un filósofo puede comer
void test_philosopher(int phil_id, DiningTable *table) {
    int left = left_fork(phil_id);
    int right = right_fork(phil_id);
    
    if (table->philosophers[phil_id].state == HUNGRY &&
        table->philosophers[left].state != EATING &&
        table->philosophers[right].state != EATING) {
        
        table->philosophers[phil_id].state = EATING;
        printf("🎉 Filósofo %d puede comer ahora\n", phil_id);
        pthread_cond_signal(&table->condition[phil_id]);
    }
}

// Solución con semáforo (previene deadlock limitando comensales)
void semaphore_solution(Philosopher *phil, DiningTable *table) {
    // Entrar al comedor (máximo N-1 filósofos pueden intentar comer)
    sem_wait(&table->dining_room);
    
    pickup_forks(phil, table);
    eat(phil, table);
    putdown_forks(phil, table);
    
    // Salir del comedor
    sem_post(&table->dining_room);
}

// Solución asimétrica (filósofos impares toman tenedor izquierdo primero)
void asymmetric_solution(Philosopher *phil, DiningTable *table) {
    int left = left_fork(phil->id);
    int right = right_fork(phil->id);
    
    if (phil->id % 2 == 0) {
        // Filósofos pares: izquierda primero
        pthread_mutex_lock(&table->forks[left]);
        pthread_mutex_lock(&table->forks[right]);
    } else {
        // Filósofos impares: derecha primero
        pthread_mutex_lock(&table->forks[right]);
        pthread_mutex_lock(&table->forks[left]);
    }
    
    eat(phil, table);
    
    pthread_mutex_unlock(&table->forks[left]);
    pthread_mutex_unlock(&table->forks[right]);
}

// Funciones auxiliares
int left_fork(int phil_id) {
    return phil_id;
}

int right_fork(int phil_id) {
    return (phil_id + 1) % NUM_PHILOSOPHERS;
}

void print_table_state(DiningTable *table) {
    pthread_mutex_lock(&table->state_mutex);
    
    printf("\n=== Estado de la Mesa ===\n");
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        printf("Filósofo %d: %s (comidas: %d)\n", 
               i, state_to_string(table->philosophers[i].state),
               table->philosophers[i].eating_count);
    }
    printf("========================\n\n");
    
    pthread_mutex_unlock(&table->state_mutex);
}

void print_statistics(DiningTable *table) {
    pthread_mutex_lock(&table->stats_mutex);
    pthread_mutex_lock(&table->state_mutex);
    
    printf("\n=== Estadísticas Finales ===\n");
    printf("Total de comidas servidas: %d\n", table->total_meals_served);
    printf("Estadísticas por filósofo:\n");
    
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        Philosopher *phil = &table->philosophers[i];
        printf("  Filósofo %d:\n", i);
        printf("    - Comidas: %d\n", phil->eating_count);
        printf("    - Tiempo pensando: %.2f segundos\n", 
               phil->total_thinking_time / 1000.0);
        printf("    - Tiempo comiendo: %.2f segundos\n", 
               phil->total_eating_time / 1000.0);
    }
    
    pthread_mutex_unlock(&table->state_mutex);
    pthread_mutex_unlock(&table->stats_mutex);
}

const char* state_to_string(PhilosopherState state) {
    switch (state) {
        case THINKING: return "Pensando";
        case HUNGRY: return "Hambriento";
        case EATING: return "Comiendo";
        default: return "Desconocido";
    }
}