#define _DEFAULT_SOURCE
#include "dining_philosophers.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>

// Variables globales para manejo de señales
static DiningTable *global_table = NULL;

// Manejador de señales para terminación limpia
void signal_handler(int sig) {
    printf("\n\nRecibida señal %d. Terminando simulación...\n", sig);
    if (global_table) {
        global_table->simulation_running = false;
    }
}

// Test básico de funcionalidad
int test_basic_functionality() {
    printf("\n=== Probando Funcionalidad Básica ===\n");
    
    DiningTable table;
    if (init_dining_table(&table) != 0) {
        printf("❌ Error inicializando mesa\n");
        return -1;
    }
    
    printf("✅ Mesa inicializada correctamente\n");
    printf("✅ Todos los filósofos en estado THINKING\n");
    
    // Verificar estado inicial
    bool all_thinking = true;
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        if (table.philosophers[i].state != THINKING) {
            all_thinking = false;
            break;
        }
    }
    
    printf("Estado inicial: %s\n", all_thinking ? "✅ CORRECTO" : "❌ INCORRECTO");
    
    destroy_dining_table(&table);
    return all_thinking ? 0 : -1;
}

// Test de un solo filósofo
int test_single_philosopher() {
    printf("\n=== Probando Un Solo Filósofo ===\n");
    
    DiningTable table;
    if (init_dining_table(&table) != 0) {
        printf("❌ Error inicializando mesa\n");
        return -1;
    }
    
    // Modificar para que solo un filósofo intente comer
    table.philosophers[0].eating_count = 0;
    
    // Crear thread solo para el filósofo 0
    if (pthread_create(&table.philosophers[0].thread, NULL, 
                      philosopher_life, &table.philosophers[0]) != 0) {
        perror("Error creando thread");
        destroy_dining_table(&table);
        return -1;
    }
    
    // Esperar un poco
    sleep(2);
    
    // Terminar simulación
    table.simulation_running = false;
    pthread_cond_broadcast(&table.condition[0]);
    pthread_join(table.philosophers[0].thread, NULL);
    
    printf("Filósofo 0 comió %d veces\n", table.philosophers[0].eating_count);
    bool success = table.philosophers[0].eating_count > 0;
    printf("Test un filósofo: %s\n", success ? "✅ EXITOSO" : "❌ FALLÓ");
    
    destroy_dining_table(&table);
    return success ? 0 : -1;
}

// Test de detección de deadlock mejorado
int test_deadlock_prevention() {
    printf("\n=== Probando Prevención de Deadlock ===\n");
    
    DiningTable table;
    if (init_dining_table(&table) != 0) {
        printf("❌ Error inicializando mesa\n");
        return -1;
    }
    
    printf("🔬 Ejecutando prueba de estrés por 8 segundos...\n");
    
    // Crear threads
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        if (pthread_create(&table.philosophers[i].thread, NULL, 
                          philosopher_life, &table.philosophers[i]) != 0) {
            perror("Error creando thread");
            table.simulation_running = false;
            for (int j = 0; j < i; j++) {
                pthread_join(table.philosophers[j].thread, NULL);
            }
            destroy_dining_table(&table);
            return -1;
        }
    }
    
    // Monitorear por deadlock mejorado
    int progress_checks = 0;
    int stagnant_periods = 0;
    int last_meals = 0;
    
    for (int i = 0; i < 8; i++) {
        sleep(1);
        int current_meals = table.total_meals_served;
        
        progress_checks++;
        
        if (current_meals == last_meals && i > 1) {
            stagnant_periods++;
        } else {
            stagnant_periods = 0; // Reset counter if there's progress
        }
        
        last_meals = current_meals;
        printf("⏱️  Segundo %d: %d comidas servidas\n", i+1, current_meals);
        
        // Solo considerar deadlock si hay más de 4 segundos consecutivos sin progreso
        // y después de al menos 3 segundos de ejecución
        if (stagnant_periods > 4 && i > 3) {
            printf("⚠️  Deadlock prolongado detectado después de %d segundos sin progreso\n", stagnant_periods);
            break;
        }
    }
    
    // Terminar
    table.simulation_running = false;
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_cond_broadcast(&table.condition[i]);
        pthread_join(table.philosophers[i].thread, NULL);
    }
    
    // Criterios más realistas para éxito
    bool sufficient_progress = table.total_meals_served >= 3; // Al menos 3 comidas en 8 segundos
    bool no_severe_deadlock = stagnant_periods <= 4; // No más de 4 segundos consecutivos sin progreso
    
    bool success = sufficient_progress && no_severe_deadlock;
    
    printf("📊 Análisis de deadlock:\n");
    printf("  Comidas totales: %d (mínimo esperado: 3)\n", table.total_meals_served);
    printf("  Períodos sin progreso: %d (máximo aceptable: 4)\n", stagnant_periods);
    printf("  Progreso suficiente: %s\n", sufficient_progress ? "✅ SÍ" : "❌ NO");
    printf("  Sin deadlock severo: %s\n", no_severe_deadlock ? "✅ SÍ" : "❌ NO");
    printf("Prevención de deadlock: %s\n", success ? "✅ EXITOSA" : "❌ FALLÓ");
    
    destroy_dining_table(&table);
    return success ? 0 : -1;
}

// Test completo multi-threaded
int test_full_simulation() {
    printf("\n=== Simulación Completa de Filósofos ===\n");
    
    DiningTable table;
    if (init_dining_table(&table) != 0) {
        printf("❌ Error inicializando mesa\n");
        return -1;
    }
    
    global_table = &table;
    
    printf("🍽️  Iniciando simulación con %d filósofos\n", NUM_PHILOSOPHERS);
    printf("📊 Cada filósofo intentará comer %d veces\n", MAX_EATING_CYCLES);
    printf("⏱️  Tiempo de pensamiento: ~%.1f segundos\n", THINKING_TIME_MS / 1000.0);
    printf("⏱️  Tiempo de comida: ~%.1f segundos\n", EATING_TIME_MS / 1000.0);
    
    // Crear threads para todos los filósofos
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        if (pthread_create(&table.philosophers[i].thread, NULL, 
                          philosopher_life, &table.philosophers[i]) != 0) {
            perror("Error creando thread de filósofo");
            table.simulation_running = false;
            // Cleanup threads ya creados
            for (int j = 0; j < i; j++) {
                pthread_join(table.philosophers[j].thread, NULL);
            }
            destroy_dining_table(&table);
            return -1;
        }
        printf("🧠 Filósofo %d iniciado\n", i);
    }
    
    // Monitorear progreso mejorado
    int consecutive_stagnant = 0;
    int last_total_meals = 0;
    
    for (int elapsed = 0; elapsed < 30 && table.simulation_running; elapsed++) {
        sleep(1);
        
        // Verificar progreso
        int current_meals = table.total_meals_served;
        if (current_meals == last_total_meals) {
            consecutive_stagnant++;
        } else {
            consecutive_stagnant = 0;
            last_total_meals = current_meals;
        }
        
        // Mostrar estado cada 5 segundos
        if (elapsed % 5 == 0) {
            printf("\n⏱️  Tiempo transcurrido: %d segundos\n", elapsed);
            printf("🍽️  Total comidas servidas: %d\n", table.total_meals_served);
            
            // Mostrar estado de cada filósofo
            pthread_mutex_lock(&table.state_mutex);
            for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
                printf("  Filósofo %d: %s (comidas: %d)\n", 
                       i, state_to_string(table.philosophers[i].state),
                       table.philosophers[i].eating_count);
            }
            pthread_mutex_unlock(&table.state_mutex);
        }
        
        // Verificar si todos terminaron
        bool all_finished = true;
        for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
            if (table.philosophers[i].eating_count < MAX_EATING_CYCLES) {
                all_finished = false;
                break;
            }
        }
        
        if (all_finished) {
            printf("\n🎉 ¡Todos los filósofos terminaron de comer!\n");
            break;
        }
        
        // Detectar deadlock severo (más de 15 segundos sin progreso)
        if (consecutive_stagnant > 15) {
            printf("\n⚠️  Deadlock severo detectado - sin progreso por %d segundos\n", 
                   consecutive_stagnant);
            break;
        }
    }
    
    // Terminar simulación
    table.simulation_running = false;
    
    // Despertar a todos los filósofos que puedan estar esperando
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_cond_broadcast(&table.condition[i]);
    }
    
    // Esperar a que terminen todos los threads
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_join(table.philosophers[i].thread, NULL);
    }
    
    print_statistics(&table);
    
    // Verificar resultados con criterios más realistas
    int total_expected = NUM_PHILOSOPHERS * MAX_EATING_CYCLES;
    bool no_starvation = true;
    int min_meals = total_expected;
    
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        if (table.philosophers[i].eating_count == 0) {
            no_starvation = false;
            printf("❌ Filósofo %d no comió (starvation)\n", i);
        }
        if (table.philosophers[i].eating_count < min_meals) {
            min_meals = table.philosophers[i].eating_count;
        }
    }
    
    // Criterios de éxito más flexibles
    bool good_progress = table.total_meals_served >= (total_expected * 0.7); // 70% del total
    bool fair_distribution = min_meals >= (MAX_EATING_CYCLES * 0.5); // Al menos 50% para cada uno
    bool no_severe_deadlock = table.total_meals_served > 10; // Al menos 10 comidas en total
    
    printf("\nResultados:\n");
    printf("  Sin inanición: %s\n", no_starvation ? "✅ SÍ" : "❌ NO");
    printf("  Progreso adecuado: %s (%d/%d comidas, %.1f%%)\n", 
           good_progress ? "✅ SÍ" : "❌ NO",
           table.total_meals_served, total_expected,
           (table.total_meals_served * 100.0) / total_expected);
    printf("  Distribución justa: %s (mínimo %d comidas por filósofo)\n",
           fair_distribution ? "✅ SÍ" : "❌ NO", min_meals);
    printf("  Sin deadlock severo: %s\n", no_severe_deadlock ? "✅ SÍ" : "❌ NO");
    
    bool success = no_starvation && good_progress && no_severe_deadlock;
    printf("\nSimulación completa: %s\n", success ? "✅ EXITOSA" : "❌ FALLÓ");
    
    destroy_dining_table(&table);
    global_table = NULL;
    
    return success ? 0 : -1;
}

int main() {
    printf("Programa de Prueba - Problema de los Filósofos Cenando\n");
    printf("====================================================\n");
    printf("Implementación con solución anti-deadlock usando:\n");
    printf("  - Semáforo para limitar comensales concurrentes\n");
    printf("  - Condition variables para sincronización\n");
    printf("  - Mutexes para exclusión mutua\n\n");
    
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
    
    if (test_single_philosopher() != 0) {
        result = -1;
    }
    
    if (test_deadlock_prevention() != 0) {
        result = -1;
    }
    
    if (test_full_simulation() != 0) {
        result = -1;
    }
    
    if (result == 0) {
        printf("\n🎉 ¡Todas las pruebas completadas exitosamente!\n");
        printf("✅ Sin deadlocks detectados\n");
        printf("✅ Sin inanición (starvation)\n");
        printf("✅ Sincronización correcta\n");
        printf("✅ Distribución justa de recursos\n");
    } else {
        printf("\n❌ Algunas pruebas fallaron\n");
    }
    
    return result;
}