#ifndef DINING_PHILOSOPHERS_H
#define DINING_PHILOSOPHERS_H

#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>

#define NUM_PHILOSOPHERS 5
#define MAX_EATING_CYCLES 5
#define THINKING_TIME_MS 1000
#define EATING_TIME_MS 800

// Estados del filósofo
typedef enum {
    THINKING,
    HUNGRY, 
    EATING
} PhilosopherState;

// Estructura para cada filósofo
typedef struct {
    int id;
    PhilosopherState state;
    int eating_count;
    int total_thinking_time;
    int total_eating_time;
    pthread_t thread;
} Philosopher;

// Estructura principal del problema
typedef struct {
    Philosopher philosophers[NUM_PHILOSOPHERS];
    pthread_mutex_t forks[NUM_PHILOSOPHERS];    // Un mutex por tenedor
    pthread_mutex_t state_mutex;               // Mutex para cambiar estados
    pthread_cond_t condition[NUM_PHILOSOPHERS]; // Condition variable por filósofo
    sem_t dining_room;                         // Semáforo para limitar comensales
    bool simulation_running;
    int total_meals_served;
    pthread_mutex_t stats_mutex;
} DiningTable;

// Funciones principales
int init_dining_table(DiningTable *table);
void destroy_dining_table(DiningTable *table);
void *philosopher_life(void *arg);

// Funciones de control
void think(Philosopher *phil, DiningTable *table);
void pickup_forks(Philosopher *phil, DiningTable *table);
void eat(Philosopher *phil, DiningTable *table);
void putdown_forks(Philosopher *phil, DiningTable *table);

// Funciones auxiliares
void test_philosopher(int phil_id, DiningTable *table);
int left_fork(int phil_id);
int right_fork(int phil_id);
void print_table_state(DiningTable *table);
void print_statistics(DiningTable *table);
const char* state_to_string(PhilosopherState state);

// Soluciones anti-deadlock
void asymmetric_solution(Philosopher *phil, DiningTable *table);
void semaphore_solution(Philosopher *phil, DiningTable *table);

#endif // DINING_PHILOSOPHERS_H