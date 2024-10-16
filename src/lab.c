#include "lab.h"
#include <pthread.h>

struct queue {
    void** arr;
    int capacity;
    int size;
    int head;
    int tail;
    bool shutdown;
    pthread_mutex_t lock;
    pthread_cond_t empty, fill;
};

queue_t queue_init(int capacity) {
    queue_t queue = (queue_t) malloc(sizeof(struct queue));
    queue->arr = (void**) malloc((capacity) * sizeof(void*));
    queue->capacity = capacity;
    queue->size = 0;
    queue->head= 0;
    queue->tail= 0;
    queue->shutdown = false;

    pthread_mutex_init(&(queue->lock), NULL);
    pthread_cond_init(&(queue->empty), NULL);
    pthread_cond_init(&(queue->fill), NULL);

    return queue;

    return queue;
}

void queue_destroy(queue_t q) {
    pthread_mutex_destroy(&(q->lock));
    pthread_cond_destroy(&(q->empty));
    pthread_cond_destroy(&(q->fill));

    free(q->arr);
    free(q);
}

void enqueue(queue_t q, void *data) {
    pthread_mutex_lock(&(q->lock));
    while (q->size == q->capacity) {
        pthread_cond_wait(&(q->empty), &(q->lock));
    }

    q->arr[q->tail] = data;
    q->tail = (q->tail + 1) % q->capacity;
    q->size++;

    pthread_cond_broadcast(&(q->fill));
    pthread_mutex_unlock(&(q->lock));
}

void *dequeue(queue_t q) {
    pthread_mutex_lock(&(q->lock));
    while (q->size == 0 && !q->shutdown) {
        pthread_cond_wait(&(q->fill), &(q->lock));
    }

    void* data = q->arr[q->head];
    if (q->size == 0) {
        data = NULL;
    } else  {
        q->head = (q->head + 1) % q->capacity;
        q->size--;
    }

    pthread_cond_broadcast(&(q->empty));
    pthread_mutex_unlock(&(q->lock));
    return data;
}

void queue_shutdown(queue_t q) {
    pthread_mutex_lock(&(q->lock));

    q->shutdown = true; 
    pthread_cond_broadcast(&(q->fill));

    pthread_mutex_unlock(&(q->lock));
}

bool is_empty(queue_t q) {
    pthread_mutex_lock(&(q->lock));

    bool empty = q->size == 0;

    pthread_mutex_unlock(&(q->lock));

    return empty;
}

bool is_shutdown(queue_t q) {
    pthread_mutex_lock(&(q->lock));

    bool shutdown = q->shutdown;

    pthread_mutex_unlock(&(q->lock));

    return shutdown;
}
