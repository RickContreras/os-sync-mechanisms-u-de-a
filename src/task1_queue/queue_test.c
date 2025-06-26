/**
 * @file queue_test.c
 * @brief Test program for thread-safe queue implementation
 */

#define _GNU_SOURCE
#include "thread_safe_queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include <stdarg.h>

#define NUM_PRODUCERS 3
#define NUM_CONSUMERS 2
#define ITEMS_PER_PRODUCER 10
#define QUEUE_CAPACITY 5

// Global queue for testing
ThreadSafeQueue test_queue;

// Statistics
volatile int total_produced = 0;
volatile int total_consumed = 0;
volatile int producers_finished = 0;
pthread_mutex_t stats_lock = PTHREAD_MUTEX_INITIALIZER;

// Printf lock to prevent race conditions in output
pthread_mutex_t printf_lock = PTHREAD_MUTEX_INITIALIZER;

// Flag to control verbose output
volatile int verbose_mode = 0;

// Thread-safe printf function
void safe_printf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    
    pthread_mutex_lock(&printf_lock);
    vprintf(format, args);
    fflush(stdout);
    pthread_mutex_unlock(&printf_lock);
    
    va_end(args);
}

/**
 * @brief Producer thread function
 */
void *producer_thread(void *arg) {
    int producer_id = *(int *)arg;
    
    if (verbose_mode) safe_printf("Producer %d started\n", producer_id);
    
    for (int i = 0; i < ITEMS_PER_PRODUCER; i++) {
        int item = producer_id * 100 + i; // Unique item based on producer ID
        
        if (verbose_mode) safe_printf("Producer %d trying to enqueue item %d\n", producer_id, item);
        
        if (enqueue(&test_queue, item) == 0) {
            pthread_mutex_lock(&stats_lock);
            total_produced++;
            int current_produced = total_produced;
            pthread_mutex_unlock(&stats_lock);
            
            if (verbose_mode || current_produced % 5 == 0) {
                safe_printf("Producer %d enqueued item %d (total produced: %d)\n", 
                           producer_id, item, current_produced);
            }
        } else {
            safe_printf("Producer %d failed to enqueue item %d\n", producer_id, item);
        }
        
        // Small delay to simulate work
        usleep(rand() % 1000); // 0-1ms
    }
    
    // Signal that this producer is finished
    pthread_mutex_lock(&stats_lock);
    producers_finished++;
    pthread_mutex_unlock(&stats_lock);
    
    if (verbose_mode) safe_printf("Producer %d finished\n", producer_id);
    return NULL;
}

/**
 * @brief Consumer thread function
 */
void *consumer_thread(void *arg) {
    int consumer_id = *(int *)arg;
    
    if (verbose_mode) safe_printf("Consumer %d started\n", consumer_id);
    
    while (1) {
        int item;
        
        if (verbose_mode) safe_printf("Consumer %d trying to dequeue\n", consumer_id);
        
        // Try non-blocking dequeue first
        if (dequeue_nonblocking(&test_queue, &item) == 0) {
            pthread_mutex_lock(&stats_lock);
            total_consumed++;
            int current_consumed = total_consumed;
            pthread_mutex_unlock(&stats_lock);
            
            if (verbose_mode || current_consumed % 5 == 0) {
                safe_printf("Consumer %d dequeued item %d (total consumed: %d)\n", 
                           consumer_id, item, current_consumed);
            }
            
            // Check if we've consumed all expected items
            if (current_consumed >= NUM_PRODUCERS * ITEMS_PER_PRODUCER) {
                if (verbose_mode) safe_printf("Consumer %d: All items consumed, exiting\n", consumer_id);
                break;
            }
        } else {
            // Queue is empty, check if all producers are done
            pthread_mutex_lock(&stats_lock);
            int current_producers_finished = producers_finished;
            int current_consumed = total_consumed;
            int current_produced = total_produced;
            pthread_mutex_unlock(&stats_lock);
            
            if (current_producers_finished == NUM_PRODUCERS && 
                current_consumed >= current_produced) {
                if (verbose_mode) safe_printf("Consumer %d: All producers finished and queue empty, exiting\n", consumer_id);
                break;
            }
            
            // Small delay before trying again
            usleep(1000); // 1ms
        }
    }
    
    if (verbose_mode) safe_printf("Consumer %d finished\n", consumer_id);
    return NULL;
}

/**
 * @brief Test basic queue operations
 */
int test_basic_operations() {
    printf("\n=== Testing Basic Operations ===\n");
    
    ThreadSafeQueue queue;
    if (init_queue(&queue) != 0) {
        printf("Failed to initialize queue\n");
        return -1;
    }
    
    // Test enqueue operations
    for (int i = 0; i < 5; i++) {
        if (enqueue(&queue, i * 10) != 0) {
            printf("Failed to enqueue item %d\n", i);
            destroy_queue(&queue);
            return -1;
        }
    }
    
    // Test size
    if (get_queue_size(&queue) != 5) {
        printf("Expected size 5, got %d\n", get_queue_size(&queue));
        destroy_queue(&queue);
        return -1;
    }
    
    // Test dequeue operations
    for (int i = 0; i < 5; i++) {
        int item;
        if (dequeue(&queue, &item) != 0) {
            printf("Failed to dequeue item %d\n", i);
            destroy_queue(&queue);
            return -1;
        }
        
        int expected = i * 10;
        if (item != expected) {
            printf("Expected %d, got %d\n", expected, item);
            destroy_queue(&queue);
            return -1;
        }
    }
    
    // Test empty queue
    if (get_queue_size(&queue) != 0) {
        printf("Expected empty queue, size is %d\n", get_queue_size(&queue));
        destroy_queue(&queue);
        return -1;
    }
    
    // Test dequeue from empty queue
    int item;
    if (dequeue(&queue, &item) == 0) {
        printf("Dequeue from empty queue should fail\n");
        destroy_queue(&queue);
        return -1;
    }
    
    destroy_queue(&queue);
    printf("Basic operations test: PASSED\n");
    return 0;
}

/**
 * @brief Test multi-threaded operations
 */
void test_multithreaded() {
    safe_printf("\n=== Testing Multi-threaded Operations ===\n");
    
    // Initialize queue
    if (queue_init(&test_queue, QUEUE_CAPACITY) != 0) {
        safe_printf("Failed to initialize queue\n");
        exit(1);
    }
    
    // Create thread arrays
    pthread_t producers[NUM_PRODUCERS];
    pthread_t consumers[NUM_CONSUMERS];
    int producer_ids[NUM_PRODUCERS];
    int consumer_ids[NUM_CONSUMERS];
    
    // Reset statistics
    total_produced = 0;
    total_consumed = 0;
    producers_finished = 0;
    
    safe_printf("Starting %d producers and %d consumers\n", NUM_PRODUCERS, NUM_CONSUMERS);
    safe_printf("Each producer will produce %d items\n", ITEMS_PER_PRODUCER);
    safe_printf("Expected total items: %d\n", NUM_PRODUCERS * ITEMS_PER_PRODUCER);
    
    // Create producer threads
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        producer_ids[i] = i;
        if (pthread_create(&producers[i], NULL, producer_thread, &producer_ids[i]) != 0) {
            safe_printf("Failed to create producer thread %d\n", i);
            exit(1);
        }
    }
    
    // Create consumer threads
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        consumer_ids[i] = i;
        if (pthread_create(&consumers[i], NULL, consumer_thread, &consumer_ids[i]) != 0) {
            safe_printf("Failed to create consumer thread %d\n", i);
            exit(1);
        }
    }
    
    // Wait for all producer threads to complete
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        pthread_join(producers[i], NULL);
    }
    
    safe_printf("All producers finished. Waiting for consumers to finish...\n");
    
    // Wait for all consumer threads to complete
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        pthread_join(consumers[i], NULL);
    }
    
    safe_printf("\nFinal Statistics:\n");
    safe_printf("Total produced: %d\n", total_produced);
    safe_printf("Total consumed: %d\n", total_consumed);
    safe_printf("Expected: %d\n", NUM_PRODUCERS * ITEMS_PER_PRODUCER);
    
    // Verify correctness
    if (total_produced == NUM_PRODUCERS * ITEMS_PER_PRODUCER &&
        total_consumed == NUM_PRODUCERS * ITEMS_PER_PRODUCER) {
        safe_printf("Multi-threaded test: PASSED\n");
    } else {
        safe_printf("Multi-threaded test: FAILED\n");
        safe_printf("  - Production %s\n", (total_produced == NUM_PRODUCERS * ITEMS_PER_PRODUCER) ? "OK" : "FAILED");
        safe_printf("  - Consumption %s\n", (total_consumed == NUM_PRODUCERS * ITEMS_PER_PRODUCER) ? "OK" : "FAILED");
    }
    
    // Clean up
    queue_destroy(&test_queue);
}

int main(int argc, char *argv[]) {
    safe_printf("Thread-Safe Queue Test Program\n");
    safe_printf("==============================\n");
    
    // Check for verbose flag
    if (argc > 1 && strcmp(argv[1], "-v") == 0) {
        verbose_mode = 1;
        safe_printf("Verbose mode enabled\n");
    }
    
    // Initialize random seed
    srand(time(NULL));
    
    // Run tests
    test_basic_operations();
    test_multithreaded();
    
    safe_printf("\nAll tests completed successfully!\n");
    
    // Destroy printf lock
    pthread_mutex_destroy(&printf_lock);
    
    return 0;
}