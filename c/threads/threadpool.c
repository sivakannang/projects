#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define MAX_QUEUE 100
#define MAX_THREADS 4

typedef void (*task_fn)(void*);

typedef struct {
    task_fn fn;
    void* arg;
} Task;

typedef struct {
    Task queue[MAX_QUEUE];
    int front;
    int rear;
    int count;

    pthread_t threads[MAX_THREADS];
    int stop;

    pthread_mutex_t lock;
    pthread_cond_t cond;
} ThreadPool;

// Task queue operations
void enqueue(ThreadPool* pool, task_fn fn, void* arg) {
    pthread_mutex_lock(&pool->lock);

    while (pool->count == MAX_QUEUE) {
        pthread_cond_wait(&pool->cond, &pool->lock); // wait if full
    }

    pool->queue[pool->rear].fn = fn;
    pool->queue[pool->rear].arg = arg;
    pool->rear = (pool->rear + 1) % MAX_QUEUE;
    pool->count++;

    pthread_cond_signal(&pool->cond);
    pthread_mutex_unlock(&pool->lock);
}

Task dequeue(ThreadPool* pool) {
    Task task = { NULL, NULL };
    pthread_mutex_lock(&pool->lock);

    while (pool->count == 0 && !pool->stop) {
        pthread_cond_wait(&pool->cond, &pool->lock); // wait if empty
    }

    if (pool->count > 0) {
        task = pool->queue[pool->front];
        pool->front = (pool->front + 1) % MAX_QUEUE;
        pool->count--;
        pthread_cond_signal(&pool->cond);
    }

    pthread_mutex_unlock(&pool->lock);
    return task;
}

void* worker(void* arg) {
    ThreadPool* pool = (ThreadPool*)arg;
    while (1) {
        Task task = dequeue(pool);
        if (task.fn)
            task.fn(task.arg);
        else if (pool->stop)
            break;
    }
    return NULL;
}

// Initialize pool
void thread_pool_init(ThreadPool* pool) {
    pool->front = 0;
    pool->rear = 0;
    pool->count = 0;
    pool->stop = 0;

    pthread_mutex_init(&pool->lock, NULL);
    pthread_cond_init(&pool->cond, NULL);

    for (int i = 0; i < MAX_THREADS; i++) {
        pthread_create(&pool->threads[i], NULL, worker, pool);
    }
}

// Add task
void thread_pool_add(ThreadPool* pool, task_fn fn, void* arg) {
    enqueue(pool, fn, arg);
}

// Shutdown pool
void thread_pool_destroy(ThreadPool* pool) {
    pthread_mutex_lock(&pool->lock);
    pool->stop = 1;
    pthread_cond_broadcast(&pool->cond);
    pthread_mutex_unlock(&pool->lock);

    for (int i = 0; i < MAX_THREADS; i++) {
        pthread_join(pool->threads[i], NULL);
    }

    pthread_mutex_destroy(&pool->lock);
    pthread_cond_destroy(&pool->cond);
}


void print_task(void* arg) {
    int id = *(int*)arg;
    printf("Thread %ld: Task %d\n", pthread_self(), id);
    usleep(100000); // simulate work
}

int main() {
    ThreadPool pool;
    thread_pool_init(&pool);

    int ids[10];
    for (int i = 0; i < 10; i++) {
        ids[i] = i + 1;
        thread_pool_add(&pool, print_task, &ids[i]);
    }

    sleep(2); // Let tasks finish
    thread_pool_destroy(&pool);
    return 0;
}

