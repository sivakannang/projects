// gcc threadpool.c -pthread -o threadpool
//
// A fixed number of worker threads take tasks from a bounded queue.
// Adding a task blocks while the queue is full, and a worker blocks while it is empty.

#include <pthread.h>
#include <stdio.h>
#include <string.h>
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
    int threadCount;
    int stop;

    pthread_mutex_t lock;
    pthread_cond_t notEmpty;   // Workers wait here for a task
    pthread_cond_t notFull;    // Callers of thread_pool_add wait here for space
} ThreadPool;

// There are two condition variables because each event has its own kind of waiter. With a
// single one, a signal meant for a worker could wake a caller, who just goes back to sleep,
// and the worker never wakes up. The wait loops recheck the condition, because a thread
// can wake without its condition being true.

// Returns 0, or -1 if the pool is shutting down and the task wasn't added
static int enqueue(ThreadPool* pool, task_fn fn, void* arg) {
    pthread_mutex_lock(&pool->lock);

    while (pool->count == MAX_QUEUE && !pool->stop) {
        pthread_cond_wait(&pool->notFull, &pool->lock);
    }

    if (pool->stop) {
        pthread_mutex_unlock(&pool->lock);
        return -1;
    }

    pool->queue[pool->rear].fn = fn;
    pool->queue[pool->rear].arg = arg;
    pool->rear = (pool->rear + 1) % MAX_QUEUE;
    pool->count++;

    pthread_cond_signal(&pool->notEmpty);
    pthread_mutex_unlock(&pool->lock);

    return 0;
}

// Returns a task with fn == NULL only when the pool is stopping and the queue is empty
static Task dequeue(ThreadPool* pool) {
    Task task = { NULL, NULL };
    pthread_mutex_lock(&pool->lock);

    while (pool->count == 0 && !pool->stop) {
        pthread_cond_wait(&pool->notEmpty, &pool->lock);
    }

    if (pool->count > 0) {
        task = pool->queue[pool->front];
        pool->front = (pool->front + 1) % MAX_QUEUE;
        pool->count--;
        pthread_cond_signal(&pool->notFull);
    }

    pthread_mutex_unlock(&pool->lock);
    return task;
}

static void* worker(void* arg) {
    ThreadPool* pool = (ThreadPool*)arg;

    while (1) {
        Task task = dequeue(pool);

        if (task.fn == NULL)
            break;

        // Run it outside the lock, so that workers can run tasks at the same time
        task.fn(task.arg);
    }

    return NULL;
}

// Returns 0, or -1 if no thread could be started. Fewer than MAX_THREADS still works.
int thread_pool_init(ThreadPool* pool) {
    pool->front = 0;
    pool->rear = 0;
    pool->count = 0;
    pool->stop = 0;
    pool->threadCount = 0;

    pthread_mutex_init(&pool->lock, NULL);
    pthread_cond_init(&pool->notEmpty, NULL);
    pthread_cond_init(&pool->notFull, NULL);

    for (int i = 0; i < MAX_THREADS; i++) {
        int ret = pthread_create(&pool->threads[pool->threadCount], NULL, worker, pool);

        if (ret != 0)
            fprintf(stderr, "pthread_create : %s\n", strerror(ret));
        else
            pool->threadCount++;
    }

    if (pool->threadCount == 0) {
        pthread_mutex_destroy(&pool->lock);
        pthread_cond_destroy(&pool->notEmpty);
        pthread_cond_destroy(&pool->notFull);
        return -1;
    }

    return 0;
}

// Returns 0, or -1 if fn is NULL or the pool is shutting down
int thread_pool_add(ThreadPool* pool, task_fn fn, void* arg) {
    if (fn == NULL)    // NULL is how a worker recognizes shutdown
        return -1;

    return enqueue(pool, fn, arg);
}

// Waits for the tasks already in the queue to finish, and then stops the workers.
// All other calls using the pool must have returned before the caller reuses or frees it.
void thread_pool_destroy(ThreadPool* pool) {
    pthread_mutex_lock(&pool->lock);
    pool->stop = 1;
    pthread_cond_broadcast(&pool->notEmpty);
    pthread_cond_broadcast(&pool->notFull);
    pthread_mutex_unlock(&pool->lock);

    for (int i = 0; i < pool->threadCount; i++) {
        pthread_join(pool->threads[i], NULL);
    }

    pthread_mutex_destroy(&pool->lock);
    pthread_cond_destroy(&pool->notEmpty);
    pthread_cond_destroy(&pool->notFull);
}


static void print_task(void* arg) {
    int id = *(int*)arg;
    printf("Thread %lu: Task %d\n", (unsigned long)pthread_self(), id);   // pthread_t's type varies, so cast it to print
    usleep(100000); // simulate work
}

int main(void) {
    ThreadPool pool;
    int ids[10];

    if (thread_pool_init(&pool) != 0)
        return 1;

    for (int i = 0; i < 10; i++) {
        ids[i] = i + 1;
        thread_pool_add(&pool, print_task, &ids[i]);
    }

    // ids must stay valid until the tasks finish. This call waits for them.
    thread_pool_destroy(&pool);

    return 0;
}
