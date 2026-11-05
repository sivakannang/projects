#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "common.h"

static int shmid = -1;

// The file must exist, so run the programs from this directory
static key_t getKey()
{
	key_t key = ftok("common.cpp", 'B');

	if ( key == -1 )
		perror("ftok(common.cpp)");

	return key;
}

void *shmCreate()
{
	void *shmptr;
	key_t key = getKey();

	if ( key == -1 )
		return NULL;

	shmid = shmget(key, sizeof(DATA), 0600 | IPC_CREAT | IPC_EXCL);

	// A crashed earlier run can leave a segment behind, with old semaphores in it. Replace it.
	if ( shmid == -1 && errno == EEXIST )
	{
		int old = shmget(key, 0, 0600);

		if ( old != -1 )
			shmctl(old, IPC_RMID, NULL);

		shmid = shmget(key, sizeof(DATA), 0600 | IPC_CREAT | IPC_EXCL);
	}

	if ( shmid == -1 )
	{
		perror("shmget");
		return NULL;
	}

	shmptr = shmat(shmid, NULL, 0);
	if ( shmptr == (void *)-1 )
	{
		perror("shmat");
		shmctl(shmid, IPC_RMID, NULL);
		return NULL;
	}

	return shmptr;
}

void *shmAttach()
{
	void *shmptr;
	key_t key = getKey();

	if ( key == -1 )
		return NULL;

	shmid = shmget(key, sizeof(DATA), 0600);
	if ( shmid == -1 )
	{
		perror("shmget (is process_1 running?)");
		return NULL;
	}

	shmptr = shmat(shmid, NULL, 0);
	if ( shmptr == (void *)-1 )
	{
		perror("shmat");
		return NULL;
	}

	return shmptr;
}


void shmDetach(void *shmptr)
{
	shmdt(shmptr);
}

void shmDestroy()
{
	shmctl(shmid, IPC_RMID, NULL);
}

int semWait(sem_t *sem)
{
	int ret;

	do {
		ret = sem_wait(sem);
	} while ( ret == -1 && errno == EINTR );

	return ret;
}

void readName(char *name, int size)
{
	if ( fgets(name, size, stdin) == NULL )
		name[0] = '\0';

	name[strcspn(name, "\n")] = '\0';
}
