// See shm.cpp

#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "common.h"


int main()
{
	int shmid;
	key_t key;
	const SHARED_DATA *data;

	key = ftok("shm.cpp", 'A');
	if ( key == -1 )
	{
		perror("ftok(shm.cpp), run this from its own directory");
		return 1;
	}

	shmid = shmget(key, sizeof(SHARED_DATA), 0600);
	if ( shmid == -1 )
	{
		perror("shmget (is shm running?)");
		return 1;
	}

	// This program only displays the data, so attach read only
	data = (const SHARED_DATA *)shmat(shmid, NULL, SHM_RDONLY);
	if ( data == (void *)-1 )
	{
		perror("shmat");
		return 1;
	}

	displayStudentInfo(&data->student);
	displayEmployeeInfo(&data->employee);
	displayBookInfo(&data->book);

	shmdt(data);

	return 0;
}
