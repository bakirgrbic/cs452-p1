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
    int rc; 

    queue_t queue = (queue_t) malloc(sizeof(queue_t));
    queue->arr = (void**) malloc(capacity * sizeof(void*));
    queue->capacity = capacity;
    queue->size = 0;
    queue->head= 0;
    queue->tail= 0;
    queue->shutdown = false;

    rc = pthread_mutex_init(&(queue->lock), NULL);
    assert(rc == 0);
    rc = pthread_cond_init(&(queue->empty), NULL);
    assert(rc == 0);
    rc = pthread_cond_init(&(queue->fill), NULL);
    assert(rc == 0);
}

void queue_destroy(queue_t q) {
    int rc; 

    rc = pthread_mutex_destroy(&(q->lock));
    assert(rc == 0);
    rc = pthread_cond_destroy(&(q->empty));
    assert(rc == 0);
    rc = pthread_cond_destroy(&(q->fill));
    assert(rc == 0);

    free(q->arr);
    free(q);
}

void enqueue(queue_t q, void *data) {
    pthread_mutex_lock(&(q->lock));
    while (q->count == q->capacity) {
        pthread_cond_wait(&(q->empty), &(q->lock));
    }
    q->size++;

    q->arr[q->tail] = data;
    if (q->tail == q->capacity) {
        q->tail = 0;
    } else {
        q->tail += 1;
    }

    pthread_cond_broadcast(&(q->fill));
    pthread_mutex_unlock(&(q->lock));
}

void *dequeue(queue_t q) {
    pthread_mutex_lock(&(q->lock));
    while (q->count == 0 && !q->shutdown) {
        pthread_cond_wait(&(q->fill), &(q->lock));
    }
    q->size--;

    void* data = q->arr[q->head];
    if (q->head == q->capacity) {
        q->head = 0;
    } else {
        q->head += 1;
    }

    pthread_cond_broadcast(&(q->empty));
    pthread_mutex_unlock(&(q->lock));
    return data
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
