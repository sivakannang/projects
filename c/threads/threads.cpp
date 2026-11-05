// g++ threads.cpp -pthread -o threads

/***************************** Threads *****************************************
 *
 * What is a thread?
 *   -> A separate flow of execution inside a process, with its own stack and registers.
 *      Threads share the process's memory, open files and signal handlers.
 *
 * Why use threads?
 *   -> To use several CPU cores, and to keep waiting for one thing from blocking everything else.
 *      Creating one and switching between them is cheaper than for processes, and sharing data is easy.
 *
 * Drawbacks
 *   -> Shared memory needs synchronization, or you get race conditions and deadlocks.
 *   -> A crash in one thread kills the whole process.
 *   -> Bugs depend on timing, so they are hard to reproduce.
 *
 * Process vs thread, or fork vs pthread_create
 *   -> fork creates a process with its own copy of memory. It is isolated, so a crash
 *      stays in that process, but sharing data needs IPC (pipes, shared memory...).
 *   -> pthread_create adds a thread in the same memory, so sharing is direct and unprotected.
 *
 * Order of thread execution
 *   -> Not guaranteed. The scheduler decides, and the order can change on every run.
 *      Only synchronization (join, mutex, condition variable...) can force an order.
 *
 * Race condition   -> the result depends on timing, because threads use shared data unsynchronized
 * Mutual exclusion -> only one thread at a time may run a critical section
 * Deadlock         -> threads wait on each other forever, for example by taking two locks in opposite orders
 *
*******************************************************************************/

/***************************** API Definition **********************************
 *
 * pthread_create()           -> create a new thread
 * pthread_cancel()           -> send a cancellation request to a thread
 *                            -> by default it takes effect at the next cancellation point, such as sleep or read
 *                            -> the thread then runs pthread_cleanup_push handlers, thread-specific data destructors, and ends
 * pthread_setcancelstate()   -> enable or disable cancellation for this thread ( PTHREAD_CANCEL_DISABLE, PTHREAD_CANCEL_ENABLE )
 *                            -> enabled by default. While disabled, requests wait until it is enabled again
 * pthread_exit()             -> terminates the calling thread, passing retval to pthread_join
 *                            -> retval must not point to the thread's own stack, which is gone by then
 * pthread_join()             -> suspends the calling thread until the target thread ends, and frees its resources
 * pthread_detach()           -> frees the thread's resources automatically when it ends
 *                            -> threads are joinable by default. Every thread must be either joined or detached, or its resources leak.
 * pthread_self()             -> obtain ID of the calling thread
 * pthread_equal()            -> compare thread IDs. pthread_t may not be a number, so don't compare with ==
 * pthread_kill()             -> send a signal to a thread. Despite the name, it doesn't have to kill it
 * pthread_attr_init()        -> initialize thread attributes, such as stack size and detached state
 * pthread_attr_destroy()     -> destroy thread attributes
 *
 * pthread_mutex_init()       -> initialize a mutex
 * pthread_mutex_lock()       -> lock, blocking if another thread holds it
 * pthread_mutex_trylock()    -> lock, failing with EBUSY instead of blocking
 * pthread_mutex_unlock()     -> unlock. Must be the thread that locked it
 * pthread_mutex_destroy()    -> destroy an unlocked mutex
 * pthread_mutexattr_init()   -> initialize mutex attributes, such as recursive or process-shared
 * pthread_mutexattr_destroy()-> destroy mutex attributes
 *
 * A read-write lock allows many readers at once, or one writer. It helps when reads greatly outnumber writes.
 *
 * pthread_rwlock_init()      -> initialize a read-write lock
 * pthread_rwlock_rdlock()    -> lock for reading, blocking if a writer holds it
 * pthread_rwlock_tryrdlock() -> lock for reading, failing with EBUSY instead of blocking
 * pthread_rwlock_wrlock()    -> lock for writing, blocking if anyone holds it
 * pthread_rwlock_trywrlock() -> lock for writing, failing with EBUSY instead of blocking
 * pthread_rwlock_unlock()    -> unlock, for readers and writers
 * pthread_rwlock_destroy()   -> destroy a read-write lock
 * pthread_rwlockattr_init()  -> initialize read-write lock attributes
 * pthread_rwlockattr_destroy()-> destroy read-write lock attributes
 *
 * All of these return 0, or the error number itself. They don't set errno, so
 * print strerror(ret) rather than strerror(errno). pthread_self and pthread_equal always succeed.
 *
 * int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg)
 * void pthread_exit(void *retval)
 * int pthread_cancel(pthread_t thread)
 * int pthread_join(pthread_t thread, void **retval)
 * int pthread_detach(pthread_t thread)
 * pthread_t pthread_self(void)
 * int pthread_equal(pthread_t t1, pthread_t t2)
 * int pthread_kill(pthread_t thread, int sig)
 * int pthread_attr_init(pthread_attr_t *attr)
 * int pthread_attr_destroy(pthread_attr_t *attr)
 *
 * int pthread_mutex_init(pthread_mutex_t *restrict mutex, const pthread_mutexattr_t *restrict attr)
 * int pthread_mutex_lock(pthread_mutex_t *mutex)
 * int pthread_mutex_trylock(pthread_mutex_t *mutex)
 * int pthread_mutex_unlock(pthread_mutex_t *mutex)
 * int pthread_mutex_destroy(pthread_mutex_t *mutex)
 * int pthread_mutexattr_init(pthread_mutexattr_t *attr)
 * int pthread_mutexattr_destroy(pthread_mutexattr_t *attr)
 *
 * int pthread_rwlock_init(pthread_rwlock_t *rwlock, const pthread_rwlockattr_t *attr)
 * int pthread_rwlock_rdlock(pthread_rwlock_t *rwlock)
 * int pthread_rwlock_tryrdlock(pthread_rwlock_t *rwlock)
 * int pthread_rwlock_wrlock(pthread_rwlock_t *rwlock)
 * int pthread_rwlock_trywrlock(pthread_rwlock_t *rwlock)
 * int pthread_rwlock_unlock(pthread_rwlock_t *rwlock)
 * int pthread_rwlock_destroy(pthread_rwlock_t *rwlock)
 * int pthread_rwlockattr_init(pthread_rwlockattr_t *attr)
 * int pthread_rwlockattr_destroy(pthread_rwlockattr_t *attr)
 *
*******************************************************************************/


#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>


#define THREAD_COUNT 2
#define WORK_SECONDS 1

typedef enum
{
	TEST_THREAD_CREATE = 1,
	TEST_THREAD_WAIT,
	TEST_THREAD_EXIT,
	TEST_THREAD_CANCEL
}TEST_TYPE;

int thread_create();
int thread_wait();
int thread_exit();
int thread_cancel();

void *doWork(void *arg);
void *doWorkAndReturnValue(void *arg);
void *doWorkWithCancelDisabled(void *arg);

void DBG_PRINT(const char* format, ...) __attribute__((format(printf, 1, 2)));

// Each thread gets a pointer to its own number. A thread can't work this out by comparing
// pthread_self() with a global pthread_t array: it may start running before pthread_create has stored its ID.
static int threadNumber[THREAD_COUNT] = {1, 2};

// Not on a thread's stack, so these are still valid after the thread ends
static int threadResult[THREAD_COUNT];


int main()
{
	int choice = 0;

	printf("\n ----------- Thread Test ------------");
	printf("\n%d. Thread create", TEST_THREAD_CREATE);
	printf("\n%d. Thread wait", TEST_THREAD_WAIT);
	printf("\n%d. Thread exit", TEST_THREAD_EXIT);
	printf("\n%d. Thread cancel", TEST_THREAD_CANCEL);
	printf("\nEnter choice : ");

	if ( scanf("%d", &choice) != 1 )
		choice = 0;

	switch(choice)
	{
		case TEST_THREAD_CREATE:
			thread_create();
			break;
		case TEST_THREAD_WAIT:
			thread_wait();
			break;
		case TEST_THREAD_EXIT:
			thread_exit();
			break;
		case TEST_THREAD_CANCEL:
			thread_cancel();
			break;
		default:
			printf("\nInvalid choice !!!");
			break;
	}

	DBG_PRINT("Main thread exits here");
	printf("\n");

	return 0;
}

// Returns how many threads started. A failed slot must not be joined later.
static int createThreads(pthread_t tid[], bool created[], void *(*routine)(void *))
{
	int count = 0;

	for ( int i = 0; i < THREAD_COUNT; i++ )
	{
		// pthread functions return the error number, and don't set errno
		int ret = pthread_create(&tid[i], NULL, routine, &threadNumber[i]);

		created[i] = (ret == 0);

		if ( ret == 0 )
		{
			DBG_PRINT("Thread %d created successfully", i+1);
			count++;
		}
		else
		{
			DBG_PRINT("Thread %d creation failed : %s", i+1, strerror(ret));
		}
	}

	return count;
}

// Shows what happens without a join: when main returns, the process
// ends and takes the threads with it, so "ends" is never printed.
int thread_create()
{
	pthread_t tid[THREAD_COUNT];
	bool created[THREAD_COUNT];

	createThreads(tid, created, doWork);

	usleep(200 * 1000);   // Long enough for them to start, but not to finish

	return 0;
}

int thread_wait()
{
	pthread_t tid[THREAD_COUNT];
	bool created[THREAD_COUNT];

	createThreads(tid, created, doWork);

	for ( int i = 0; i < THREAD_COUNT; i++ )
	{
		if ( created[i] )
		{
			int ret = pthread_join(tid[i], NULL);
			DBG_PRINT("Thread %d join %s", i+1, ret ? strerror(ret) : "SUCCESS");
		}
	}

	return 0;
}

int thread_exit()
{
	pthread_t tid[THREAD_COUNT];
	bool created[THREAD_COUNT];

	createThreads(tid, created, doWorkAndReturnValue);

	for ( int i = 0; i < THREAD_COUNT; i++ )
	{
		if ( created[i] )
		{
			void *result = NULL;
			int ret = pthread_join(tid[i], &result);

			if ( ret == 0 )
				DBG_PRINT("Thread %d join SUCCESS, returned %d", i+1, *(int *)result);
			else
				DBG_PRINT("Thread %d join failed : %s", i+1, strerror(ret));
		}
	}

	return 0;
}

int thread_cancel()
{
	void *res = NULL;
	pthread_t tid;
	int ret = pthread_create(&tid, NULL, doWorkWithCancelDisabled, NULL);

	if ( ret != 0 )
	{
		DBG_PRINT("Thread creation failed : %s", strerror(ret));
		return -1;
	}

	DBG_PRINT("Thread created successfully");

	sleep(1);   // Give thread a chance to get started

	DBG_PRINT("Sending cancellation request");
	ret = pthread_cancel(tid);
	if ( ret != 0 )
		DBG_PRINT("Thread cancel request failed : %s", strerror(ret));

	ret = pthread_join(tid, &res);
	DBG_PRINT("Thread join %s", ret ? strerror(ret) : "SUCCESS");

	if ( res == PTHREAD_CANCELED )
		DBG_PRINT("Thread status cancelled");
	else
		DBG_PRINT("Oops !!!!! Thread was not cancelled and we shouldn't be here");

	return 0;
}

void *doWorkWithCancelDisabled(void *)
{
	int ret;

	// Disable cancellation for a while, so the request waits instead of acting immediately
	ret = pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
	if ( ret != 0 )
		DBG_PRINT("Thread set cancel state to disable failed : %s", strerror(ret));

	DBG_PRINT("Thread cancel disabled");
	sleep(2);

	DBG_PRINT("Thread cancel enabling");
	ret = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	if ( ret != 0 )
		DBG_PRINT("Thread set cancel state to enable failed : %s", strerror(ret));

	DBG_PRINT("Thread cancel enabled");

	sleep(20);   // A cancellation point: the waiting request acts here
	DBG_PRINT("Oops !!!!! we shouldn't be here");

	return NULL;
}


void *doWork(void *arg)
{
	int number = *(int *)arg;

	DBG_PRINT("Thread %d processing", number);

	sleep(WORK_SECONDS);   // Stands in for real work

	DBG_PRINT("Thread %d ends", number);

	return NULL;
}

void *doWorkAndReturnValue(void *arg)
{
	int number = *(int *)arg;

	DBG_PRINT("Thread %d processing", number);

	sleep(WORK_SECONDS);

	DBG_PRINT("Thread %d ends", number);

	threadResult[number - 1] = number * 100;

	// The same as: return &threadResult[number - 1];
	pthread_exit(&threadResult[number - 1]);
}


// Threads call this at the same time, so everything here must be thread-safe: localtime_r rather
// than localtime, a local buffer, and a single printf so that the lines don't get mixed together.
void DBG_PRINT(const char* format, ...)
{
	char buffer[1024];
	struct timeval now;
	struct tm lt;
	va_list args;
	int len;

	gettimeofday(&now, NULL);
	localtime_r(&now.tv_sec, &lt);

	len = snprintf(buffer, sizeof(buffer), "\n%02d/%02d/%02d %02d%02d%02d:%03d => ", lt.tm_mon + 1, lt.tm_mday, lt.tm_year - 100, lt.tm_hour, lt.tm_min, lt.tm_sec, (int)(now.tv_usec / 1000));

	va_start(args, format);
	vsnprintf(buffer + len, sizeof(buffer) - len, format, args);
	va_end(args);

	printf("%s", buffer);
	fflush(stdout);
}
