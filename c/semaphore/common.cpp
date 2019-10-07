
#include <stdio.h>
#include <sys/shm.h>
#include "common.h"

int shmid = 0;

void *shmCreate()
{
	
	key_t key = 0;
	void *shmptr = NULL;
	int size = sizeof(DATA);
	char option = 0;

	key = ftok("common.cpp", 'B');

	shmid = shmget(key, size, 0666 | IPC_CREAT);
	if ( shmid == -1){
		printf("\nShared memory allocation failed");
		return NULL;
	}

	printf("\nDel -> shared memory allocated");

	shmptr = shmat(shmid, NULL, 0);
	if ( shmptr == (void *)-1) {
		shmctl(shmid, IPC_RMID, NULL);
		printf("\nShared memory attach failed");
		return NULL;
	}

	printf("\nDel -> shared memory attached");

	return shmptr;

}

void *shmAttach()
{
	key_t key = 0;
	void *shmptr = NULL;
	int size = sizeof(DATA);
	char option = 0;

	key = ftok("common.cpp", 'B');

	shmid = shmget(key, size, 0666);
	if ( shmid == -1){
		printf("\nShared memory allocation failed");
		return NULL;
	}

	shmptr = shmat(shmid, NULL, 0);
	if ( shmptr == (void *)-1) {
		printf("\nShared memory attach failed");
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


