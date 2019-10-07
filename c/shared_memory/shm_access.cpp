
#include <stdio.h>
#include <sys/shm.h>
#include <sys/ipc.h>

#include "common.h"


int main()
{
	int shmid = 0;
	key_t key = 0;
	void *shmptr = NULL;
	int size = sizeof(STUDENT) + sizeof(EMPLOYEE) + sizeof(BOOK);

	key = ftok("shm.cpp", 'A');

	shmid = shmget(key, size, 0666 );
	if ( shmid == -1){
		printf("\nShared memory allocation failed");
		return 1;
	}

	shmptr = shmat(shmid, NULL, 0);
	if ( shmptr == (void *)-1) {
		printf("\nShared memory attach failed");
		return -1;
	}

	STUDENT *student = (STUDENT *)getPtrFromSharedMem(shmptr, T_STUDENT);
	EMPLOYEE *employee = (EMPLOYEE *)getPtrFromSharedMem(shmptr, T_EMPLOYEE);
	BOOK *book = (BOOK *)getPtrFromSharedMem(shmptr, T_BOOK);

	displayStudentInfo(student);
	displayEmployeeInfo(employee);
	displayBookInfo(book);


	shmdt(shmptr);

	return 0;
}
