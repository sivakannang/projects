
#include <stdio.h>
#include <errno.h>
#include "common.h"


int main()
{

	int ret = 0, sval = 0;

	DATA *data = (DATA *)shmAttach();

	if ( data == NULL)
	{
		printf("\nShared memory create failed");
		return -1;
	}

	/*ret = sem_init(&(data->lock), 1, 1); // second param = 1 , semaphore is between processes
	if ( ret == -1)
	{
		shmDetach((void *)data);
		return -1;
	}*/


	ret = sem_getvalue(&(data->lock), &sval);

	printf("\n1 => Process 2 waiting for lock : %d %d %d", ret, sval, errno);
	fflush(stdout);
	ret = sem_wait(&(data->lock));
	if ( ret == -1)
		printf("\n1 => Process 2 lock failed, errno %d", errno);
	printf("\n1 => Process 2 acquired lock");
	fflush(stdout);

	printf("\nName : %s", data->name);

	printf("\nEnter name : ");
	gets(data->name);

	ret = sem_post(&(data->lock));
	printf("\n1 => Process 2 released lock");	


	shmDetach( (void *)data);

	return 0;

} 


