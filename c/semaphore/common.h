
#include <semaphore.h>

typedef struct
{
	sem_t lock;
	int counter;
	char name[48];
}DATA;


void *shmCreate();
void *shmAttach();
void shmDetach(void *shmptr);
void shmDestroy();

