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
