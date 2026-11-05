// g++ process_1.cpp common.cpp -pthread -o process_1
// g++ process_2.cpp common.cpp -pthread -o process_2
//
// Two processes share a name through shared memory. Run process_1 first, then process_2 in
// another window, both from this directory.
//
// lock makes access to the name mutually exclusive. The two Done semaphores set the order:
// process 1 writes, process 2 reads and replaces, process 1 reads the result and cleans up.
// Without them, process 1 could destroy the semaphores while process 2 is still using them.

#include <stdio.h>
#include "common.h"


int main()
{
	DATA *data = (DATA *)shmCreate();

	if ( data == NULL)
		return 1;

	// The second argument is 1 because these semaphores are shared between processes,
	// which is why they must live in shared memory
	if ( sem_init(&data->lock, 1, 1) == -1 || sem_init(&data->process1Done, 1, 0) == -1 || sem_init(&data->process2Done, 1, 0) == -1 )
	{
		perror("sem_init");
		shmDetach(data);
		shmDestroy();
		return 1;
	}

	// Release ordering: anyone who sees ready == 1 also sees the initialized semaphores
	__atomic_store_n(&data->ready, 1, __ATOMIC_RELEASE);

	printf("Process 1 waiting for lock\n");
	semWait(&data->lock);
	printf("Process 1 acquired lock\n");

	printf("Enter name : ");
	fflush(stdout);
	readName(data->name, sizeof(data->name));

	sem_post(&data->lock);
	printf("Process 1 released lock\n");

	sem_post(&data->process1Done);

	printf("Process 1 waiting for process 2\n");
	semWait(&data->process2Done);

	semWait(&data->lock);
	printf("Name : %s\n", data->name);
	sem_post(&data->lock);

	// Safe now: process 2 has finished with them
	sem_destroy(&data->lock);
	sem_destroy(&data->process1Done);
	sem_destroy(&data->process2Done);

	shmDetach(data);
	shmDestroy();

	return 0;
}
