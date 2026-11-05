// See process_1.cpp

#include <stdio.h>
#include <unistd.h>
#include "common.h"

#define READY_WAIT_MS 10000

int main()
{
	int waited = 0;
	DATA *data = (DATA *)shmAttach();

	if ( data == NULL)
		return 1;

	// The segment can exist before process 1 has initialized the semaphores in it
	while ( __atomic_load_n(&data->ready, __ATOMIC_ACQUIRE) == 0 )
	{
		if ( waited >= READY_WAIT_MS )
		{
			fprintf(stderr, "Timed out waiting for process 1\n");
			shmDetach(data);
			return 1;
		}

		usleep(100 * 1000);
		waited += 100;
	}

	printf("Process 2 waiting for process 1\n");
	semWait(&data->process1Done);

	semWait(&data->lock);
	printf("Process 2 acquired lock\n");

	printf("Name : %s\n", data->name);

	printf("Enter name : ");
	fflush(stdout);
	readName(data->name, sizeof(data->name));

	sem_post(&data->lock);
	printf("Process 2 released lock\n");

	// Last use of the semaphores in this process: after this, process 1 may destroy them
	sem_post(&data->process2Done);

	shmDetach(data);

	return 0;
}
