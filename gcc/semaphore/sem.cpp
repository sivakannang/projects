
/***************************** Semaphores **************************************
 *
 * Semaphores are a technique, that use to synchronize the access of shared/critical section between the multiple threads/processes
 * Semaphores can be binary (0 or 1) or can have additional values
 * Semaphores are commonly use for two purposes: to share a common memory space and to share access to files
 *
 * Types
 *  -> Binary   semaphore    - A semaphore with count 1 ( so values will be either 1 or 0 )
 *  -> Counting semaphore    - A semaphore with count more than 1, it keeps track of how many units free but does not keep track of which unit is free 
 *
 *
 * sem_init()        -> initialize an unnamed semaphore
 * sem_wait()        -> lock - decrements the semaphore,  If the semaphore value zero, then the call blocks until either it becomes possible to perform the decrement 
 * sem_trywait()     -> try lock - non block mode, returns immediately 
 * sem_timedwait()   -> try lock with a timeout period
 * sem_post()        -> unlock - increments the semaphore 
 * sem_destroy()     -> destroy an unnamed semaphore
 *
 *
 * msg header    -> semaphore.h
 * ipc header    -> sys/ipc.h
 * link lib      -> rt or pthread
 * return        -> sem_init() = sem_wait() = sem_trywait() = sem_timedwait() = sem_post() = sem_destroy() = 0
 *
*******************************************************************************/

/****************** Example - Producer Consumer problem ************************
 *
 *  Producer - which produces a piece of data and puts it in a queue
 *  Consumer - which consumes a piece of data from a queue 
 *   
 *  Producer and Consumer share a fixed buffer like queue and the problem happens when
 *      Queue is full  --- > But producer still continues to produce data and add it into queue.
 *      Queue is empty ----> But consumer still tries to remove data from the queue.
 *
 *  Solution - Semaphore
 *
*******************************************************************************/

/*****************************  IPC system cmds ********************************
 *
 * #man ipcs
 * #man ipcrm
 * #ipcs -s                 => Display information about semaphores
 * #ipcrm -s semid          => Removes the semaphore set associated with ID semid from the system
 * #ipcrm -S semkey         => Remove the semaphore set associated with key semkey from the system
 *
*******************************************************************************/

/***************************** API defenitions *********************************
 *
 * int sem_init(sem_t *sem, int pshared, unsigned int value)
 * int sem_wait(sem_t *sem)
 * int sem_trywait(sem_t *sem)
 * int sem_timedwait(sem_t *sem, const struct timespec *abs_timeout)
 * int sem_post(sem_t *sem)
 * int sem_destroy(sem_t *sem)
 *
 * sem         -> initialized unnamed semaphore
 * pshared     -> argument indicates whether this semaphore is to be shared between the threads of a process, or between processes
 *                 zero     - semaphore is shared between the threads of a process, and should be located at some address that is visible to all threads (e.g., a global variable, or a variable allocated dynamically on the heap)
 *                 non zero - shared between processes, and should be located in a region of shared memory 
 * value       -> specifies the initial value for the semaphore ( number of locks )
 * abs_timeout -> struct timespec {
                  	time_t tv_sec;      // Seconds
                        long   tv_nsec;     // Nanoseconds [0 .. 999999999]
                  }
 *
*******************************************************************************/
/*
 *
 * race conditions and dead lock
 * thread safe
 * binary semaphore vs mutex
 * producer - consumer problem
 *
 */




#include <unistd.h>     /* Symbolic Constants */
#include <sys/types.h>  /* Primitive System Data Types */ 
#include <errno.h>      /* Errors */
#include <stdio.h>      /* Input/Output */
#include <stdlib.h>     /* General Utilities */
#include <pthread.h>    /* POSIX Threads */
#include <string.h>     /* String handling */
#include <semaphore.h>  /* Semaphore */

/* prototype for thread routine */
void *handler ( void *ptr );


sem_t lock;
int counter; /* shared variable */

int main()
{
    int i[5];
    pthread_t thread_a;
    pthread_t thread_b;
    pthread_t thread_c;
    pthread_t thread_d;
    pthread_t thread_e;
    
    i[0] = 1; /* argument to threads */
    i[1] = 2;
    i[2] = 3;
    i[3] = 4;
    i[4] = 5;
    
    sem_init(&lock, 0, 1);      // initialize semaphore to 1 - binary semaphore  , Only one can use resource at a time 
                                 /* second param = 0 - semaphore is local */
                                 
    pthread_create (&thread_a, NULL, &handler, (void *) &i[0]);
    pthread_create (&thread_b, NULL, &handler, (void *) &i[1]);
    pthread_create (&thread_c, NULL, &handler, (void *) &i[2]);
    pthread_create (&thread_d, NULL, &handler, (void *) &i[3]);
    pthread_create (&thread_e, NULL, &handler, (void *) &i[4]);
    
    pthread_join(thread_a, NULL);
    pthread_join(thread_b, NULL);
    pthread_join(thread_c, NULL);
    pthread_join(thread_d, NULL);
    pthread_join(thread_e, NULL);

    sem_destroy(&lock); /* destroy semaphore */
                  
    exit(0);
} 

void *handler ( void *ptr )
{
    int x; 
    x = *((int *) ptr);
    printf("Thread %d: Waiting to enter critical region...\n", x);
    sem_wait(&lock);       /* down semaphore */
    /* START CRITICAL REGION */
    printf("Thread %d: Now in critical region...\n", x);
    sleep(3);
    printf("Thread %d: Counter Value: %d\n", x, counter);
    printf("Thread %d: Incrementing Counter...\n", x);
    counter++;
    printf("Thread %d: New Counter Value: %d\n", x, counter);
    printf("Thread %d: Exiting critical region...\n", x);
    /* END CRITICAL REGION */    
    sleep(3);
    sem_post(&lock);       /* up semaphore */
    
    pthread_exit(0); /* exit thread */

    return (void *)" ";    // This is wrong
}
