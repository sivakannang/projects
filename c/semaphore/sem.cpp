// g++ sem.cpp -pthread -o sem

/***************************** Semaphores **************************************
 *
 * A semaphore is a counter that threads or processes use to coordinate.
 * sem_wait decrements it, blocking while it is 0. sem_post increments it, waking one waiter.
 *
 * Types
 *  -> Binary   semaphore    - values are only 0 or 1, used as a lock
 *  -> Counting semaphore    - counts free units of a resource, but not which unit is free
 *
 * Binary semaphore vs mutex
 *  -> A mutex has an owner: only the thread that locked it may unlock it.
 *  -> A semaphore has no owner: any thread can post it, so one can signal another that something
 *     has happened. Use a mutex for mutual exclusion, and a semaphore for signaling.
 *
 * sem_init()        -> initialize an unnamed semaphore
 * sem_wait()        -> decrement, blocking while the value is 0
 * sem_trywait()     -> decrement without blocking. Fails with EAGAIN if the value is 0
 * sem_timedwait()   -> decrement, failing with ETIMEDOUT at an absolute time
 * sem_post()        -> increment
 * sem_getvalue()    -> read the current value
 * sem_destroy()     -> destroy an unnamed semaphore. No thread may still be waiting on it
 *
 * sem_open(), sem_close(), sem_unlink() -> the same for named semaphores, which unrelated
 *                                          processes can open by name, without shared memory
 *
 * header        -> semaphore.h
 * link with     -> -pthread
 * return        -> all return 0, or -1 with errno set, except sem_open
 *
*******************************************************************************/

/****************** Example - Producer Consumer problem ************************
 *
 *  Producer - puts data into a fixed-size queue
 *  Consumer - takes data from the queue
 *
 *  The producer must wait when the queue is full, and the consumer when it is empty.
 *
 *  Solution, with 2 counting semaphores and a lock:
 *    empty = queue size, full = 0, mutex = 1
 *
 *    Producer : wait(empty); lock(mutex); add item;    unlock(mutex); post(full);
 *    Consumer : wait(full);  lock(mutex); remove item; unlock(mutex); post(empty);
 *
 *  The order matters: taking the lock before wait(empty) can deadlock.
 *
*******************************************************************************/

/*****************************  IPC system cmds ********************************
 *
 * ipcs and ipcrm only show System V semaphores (semget), not the POSIX ones used here.
 *
 * #ipcs -s                 => Display information about System V semaphores
 * #ipcrm -s semid          => Removes the semaphore set with this ID
 * #ipcrm -S semkey         => Removes the semaphore set with this key
 * #ls /dev/shm             => Named POSIX semaphores appear here, as sem.<name>
 *
*******************************************************************************/

/***************************** API definitions *********************************
 *
 * int sem_init(sem_t *sem, int pshared, unsigned int value)
 * int sem_wait(sem_t *sem)
 * int sem_trywait(sem_t *sem)
 * int sem_timedwait(sem_t *sem, const struct timespec *abs_timeout)
 * int sem_post(sem_t *sem)
 * int sem_getvalue(sem_t *sem, int *sval)
 * int sem_destroy(sem_t *sem)
 *
 * pshared     -> 0        - shared between the threads of a process (a global or heap variable)
 *                non zero - shared between processes (must be in shared memory)
 * value       -> the initial value
 * abs_timeout -> struct timespec {
 *                     time_t tv_sec;      // Seconds
 *                     long   tv_nsec;     // Nanoseconds [0 .. 999999999]
 *                }
 *
*******************************************************************************/


#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define THREAD_COUNT 5

void *handler(void *ptr);

sem_t lock;
int counter; /* shared variable */

int main()
{
	int ids[THREAD_COUNT];
	pthread_t threads[THREAD_COUNT];
	int i, ret, started = 0;

	// Value 1 makes this a binary semaphore. The second argument is 0: shared between threads only.
	if ( sem_init(&lock, 0, 1) == -1 )
	{
		perror("sem_init");
		return 1;
	}

	for ( i = 0; i < THREAD_COUNT; i++ )
	{
		ids[i] = i + 1;

		ret = pthread_create(&threads[i], NULL, handler, &ids[i]);
		if ( ret != 0 )
		{
			fprintf(stderr, "pthread_create : %s\n", strerror(ret));
			break;
		}

		started++;
	}

	for ( i = 0; i < started; i++ )
		pthread_join(threads[i], NULL);

	sem_destroy(&lock);

	printf("Final counter : %d\n", counter);

	return 0;
}

void *handler(void *ptr)
{
	int x = *(int *)ptr;

	printf("Thread %d: Waiting to enter critical region...\n", x);
	sem_wait(&lock);

	/* START CRITICAL REGION */
	printf("Thread %d: Now in critical region...\n", x);
	sleep(1);
	printf("Thread %d: Counter Value: %d\n", x, counter);
	printf("Thread %d: Incrementing Counter...\n", x);
	counter++;
	printf("Thread %d: New Counter Value: %d\n", x, counter);
	printf("Thread %d: Exiting critical region...\n", x);
	/* END CRITICAL REGION */

	sem_post(&lock);

	return NULL;
}
