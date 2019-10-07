
#include <stdio.h>
#include <unistd.h>
#include "common.h"


int main()
{

	int ret = 0, sval = 0;

	DATA *data = (DATA *)shmCreate();

	if ( data == NULL)
	{
		printf("\nShared memory create failed");
		return -1;
	}

	printf("\nShared memory allocated");

	ret = sem_init(&(data->lock), 1, 1); // second param = 1 , semaphore is between processes
	if ( ret == -1)
	{
		shmDetach((void *)data);
		shmDestroy();
		return -1;
	}

	ret = sem_getvalue(&(data->lock), &sval);

	printf("\n1 => Process 1 waiting for lock : %d %d", ret, sval);
	ret = sem_wait(&(data->lock));
	if ( ret == -1)
		printf("\n1 => process 1 lock failed");

	ret = sem_getvalue(&(data->lock), &sval);
	printf("\n1 => Process 1 acquired lock : %d %d", ret, sval);


	printf("\nEnter name : ");
	gets(data->name);

	sem_post(&(data->lock));
	printf("\n1 => Process 1 released lock");

	printf("\n2 => Process 1 waiting for lock");
	fflush(stdout);
	sleep(1);
	ret = sem_wait(&(data->lock));
	printf("\n2 => Process 1 acquired lock , ret = %d", ret);

	printf("\nName : %s", data->name);
	sem_post(&(data->lock));
	printf("\n2 => Process 1 released lock");
	sem_destroy(&(data->lock));
	printf("\n2 => Process 1 destroyed semaphore");

	printf("\nName : %s", data->name);
	shmDetach( (void *)data);
	shmDestroy();
	printf("\n");

	return 0;

} 


