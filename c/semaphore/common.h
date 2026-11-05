#ifndef SEMAPHORE_COMMON_H
#define SEMAPHORE_COMMON_H

#include <semaphore.h>

typedef struct
{
	int ready;           // Set last by process 1, so process 2 knows the semaphores are initialized
	sem_t lock;          // Mutual exclusion: only one process uses name at a time
	sem_t process1Done;  // Ordering: process 1 has written name
	sem_t process2Done;  // Ordering: process 2 has finished, so process 1 can clean up
	char name[48];
}DATA;


void *shmCreate();
void *shmAttach();
void shmDetach(void *shmptr);
void shmDestroy();

// sem_wait that carries on if a signal interrupts it. Returns 0, or -1 on error.
int semWait(sem_t *sem);

// Reads a line into name, without the newline. name is left empty at end of input.
void readName(char *name, int size);

#endif
