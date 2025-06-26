/**
 * @file thread_safe_queue.h
 * @brief Thread-safe queue implementation using mutex and condition variables
 * @author Ricardo Contreras Garzón
 * @date 2025
 */

#ifndef THREAD_SAFE_QUEUE_H
#define THREAD_SAFE_QUEUE_H

#include <pthread.h>
#include <stdbool.h>

#define MAX_QUEUE_SIZE 100

/**
 * @brief Thread-safe queue structure using circular buffer
 */
typedef struct {
    int *items;           // Array to store queue items
    int front;            // Index of front element
    int rear;             // Index of rear element
    int size;             // Current number of elements
    int capacity;         // Maximum capacity
    pthread_mutex_t lock; // Mutex for thread safety
    pthread_cond_t not_empty; // Condition variable for non-empty queue
    pthread_cond_t not_full;  // Condition variable for non-full queue
} ThreadSafeQueue;

/**
 * @brief Initialize a thread-safe queue
 * @param q Pointer to the queue structure
 * @param capacity Maximum capacity of the queue
 * @return 0 on success, -1 on failure
 */
int queue_init(ThreadSafeQueue *q, int capacity);

/**
 * @brief Destroy a thread-safe queue and free resources
 * @param q Pointer to the queue structure
 */
void queue_destroy(ThreadSafeQueue *q);

/**
 * @brief Add an item to the queue (blocking if full)
 * @param q Pointer to the queue structure
 * @param item Item to add
 * @return 0 on success, -1 on failure
 */
int enqueue(ThreadSafeQueue *q, int item);

/**
 * @brief Remove an item from the queue (blocking if empty)
 * @param q Pointer to the queue structure
 * @param item Pointer to store the removed item
 * @return 0 on success, -1 on failure
 */
int dequeue(ThreadSafeQueue *q, int *item);

/**
 * @brief Try to add an item without blocking
 * @param q Pointer to the queue structure
 * @param item Item to add
 * @return 0 on success, -1 if queue is full or error
 */
int enqueue_nonblocking(ThreadSafeQueue *q, int item);

/**
 * @brief Try to remove an item without blocking
 * @param q Pointer to the queue structure
 * @param item Pointer to store the removed item
 * @return 0 on success, -1 if queue is empty or error
 */
int dequeue_nonblocking(ThreadSafeQueue *q, int *item);

/**
 * @brief Get current size of the queue (thread-safe)
 * @param q Pointer to the queue structure
 * @return Current size, -1 on error
 */
int queue_size(ThreadSafeQueue *q);

/**
 * @brief Check if queue is empty (thread-safe)
 * @param q Pointer to the queue structure
 * @return true if empty, false otherwise
 */
bool queue_is_empty(ThreadSafeQueue *q);

/**
 * @brief Check if queue is full (thread-safe)
 * @param q Pointer to the queue structure
 * @return true if full, false otherwise
 */
bool queue_is_full(ThreadSafeQueue *q);

#endif // THREAD_SAFE_QUEUE_H