/**
 * @file thread_safe_queue.c
 * @brief Implementation of thread-safe queue using mutex and condition variables
 */

#include "thread_safe_queue.h"
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

int queue_init(ThreadSafeQueue *q, int capacity) {
    if (q == NULL || capacity <= 0) {
        return -1;
    }

    // Allocate memory for items
    q->items = malloc(capacity * sizeof(int));
    if (q->items == NULL) {
        return -1;
    }

    // Initialize queue properties
    q->front = 0;
    q->rear = 0;
    q->size = 0;
    q->capacity = capacity;

    // Initialize mutex
    if (pthread_mutex_init(&q->lock, NULL) != 0) {
        free(q->items);
        return -1;
    }

    // Initialize condition variables
    if (pthread_cond_init(&q->not_empty, NULL) != 0) {
        pthread_mutex_destroy(&q->lock);
        free(q->items);
        return -1;
    }

    if (pthread_cond_init(&q->not_full, NULL) != 0) {
        pthread_cond_destroy(&q->not_empty);
        pthread_mutex_destroy(&q->lock);
        free(q->items);
        return -1;
    }

    return 0;
}

void queue_destroy(ThreadSafeQueue *q) {
    if (q == NULL) {
        return;
    }

    // Destroy condition variables
    pthread_cond_destroy(&q->not_empty);
    pthread_cond_destroy(&q->not_full);
    
    // Destroy mutex
    pthread_mutex_destroy(&q->lock);
    
    // Free memory
    free(q->items);
    q->items = NULL;
}

int enqueue(ThreadSafeQueue *q, int item) {
    if (q == NULL) {
        return -1;
    }

    pthread_mutex_lock(&q->lock);

    // Wait while queue is full
    while (q->size == q->capacity) {
        pthread_cond_wait(&q->not_full, &q->lock);
    }

    // Add item to queue
    q->items[q->rear] = item;
    q->rear = (q->rear + 1) % q->capacity;
    q->size++;

    // Signal that queue is not empty
    pthread_cond_signal(&q->not_empty);
    
    pthread_mutex_unlock(&q->lock);
    return 0;
}

int dequeue(ThreadSafeQueue *q, int *item) {
    if (q == NULL || item == NULL) {
        return -1;
    }

    pthread_mutex_lock(&q->lock);

    // Wait while queue is empty
    while (q->size == 0) {
        pthread_cond_wait(&q->not_empty, &q->lock);
    }

    // Remove item from queue
    *item = q->items[q->front];
    q->front = (q->front + 1) % q->capacity;
    q->size--;

    // Signal that queue is not full
    pthread_cond_signal(&q->not_full);
    
    pthread_mutex_unlock(&q->lock);
    return 0;
}

int enqueue_nonblocking(ThreadSafeQueue *q, int item) {
    if (q == NULL) {
        return -1;
    }

    pthread_mutex_lock(&q->lock);

    // Check if queue is full
    if (q->size == q->capacity) {
        pthread_mutex_unlock(&q->lock);
        return -1;
    }

    // Add item to queue
    q->items[q->rear] = item;
    q->rear = (q->rear + 1) % q->capacity;
    q->size++;

    // Signal that queue is not empty
    pthread_cond_signal(&q->not_empty);
    
    pthread_mutex_unlock(&q->lock);
    return 0;
}

int dequeue_nonblocking(ThreadSafeQueue *q, int *item) {
    if (q == NULL || item == NULL) {
        return -1;
    }

    pthread_mutex_lock(&q->lock);

    // Check if queue is empty
    if (q->size == 0) {
        pthread_mutex_unlock(&q->lock);
        return -1;
    }

    // Remove item from queue
    *item = q->items[q->front];
    q->front = (q->front + 1) % q->capacity;
    q->size--;

    // Signal that queue is not full
    pthread_cond_signal(&q->not_full);
    
    pthread_mutex_unlock(&q->lock);
    return 0;
}

int queue_size(ThreadSafeQueue *q) {
    if (q == NULL) {
        return -1;
    }

    pthread_mutex_lock(&q->lock);
    int size = q->size;
    pthread_mutex_unlock(&q->lock);
    
    return size;
}

bool queue_is_empty(ThreadSafeQueue *q) {
    if (q == NULL) {
        return true;
    }

    pthread_mutex_lock(&q->lock);
    bool empty = (q->size == 0);
    pthread_mutex_unlock(&q->lock);
    
    return empty;
}

bool queue_is_full(ThreadSafeQueue *q) {
    if (q == NULL) {
        return false;
    }

    pthread_mutex_lock(&q->lock);
    bool full = (q->size == q->capacity);
    pthread_mutex_unlock(&q->lock);
    
    return full;
}