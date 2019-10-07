
// http://www.cquestions.com/2011/02/data-segment-in-c.html
//http://stackoverflow.com/questions/341817/is-there-a-replacement-for-unistd-h-for-windows-visual-c
#ifdef _WIN32
#include "io.h"
#define STDOUT_FILENO 1
#else
#include "unistd.h"
#endif

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "errno.h"

#include <fcntl.h>
#include <sys/stat.h>

typedef enum
{
	SUCCESS,
	FAIL
}STATUS;


STATUS exDup();
STATUS exDup_();
STATUS exDup2();
void switchcase();


typedef enum
{
	DUP_STDOUT = 1,
	DUP_STDOUT_TO_FILE,
	DUP2_STDOUT_TO_FILE,
	END
}VALUE;

typedef struct
{
	VALUE index;
	char name[96];
	STATUS (*funcPtr)(void);
	
}LOOKUP;

LOOKUP data[] = {
	{ DUP_STDOUT,                    "1. dup     -> duplicating stdout",             exDup },
	{ DUP_STDOUT_TO_FILE,            "2. dup     -> redirect stdout to file",        exDup_},
	{ DUP2_STDOUT_TO_FILE,           "3. dup2    -> redirect stdout to file",        exDup2},
	{ END,                           "NULL",                                         NULL }

};

#define LOGGER "logger"

char MSG[] = "Hello ... This is a sample message !!!!!!";



int main()
{
	int i = 0, choice = 0;
	

	do
	{
		i = choice = 0;

		while ( data[i].index != END) {
			printf("\n%s", data[i].name);
			i++;
		}

		printf("\n%d. Exit", END);
		printf("\nEnter your choice : ");
		scanf("%d", &choice);

		i = 0;
		while( data[i].index != END) {

			if ( data[i].index == choice){
				data[i].funcPtr();
				break;
			}
			i++;
		}
		printf("\n\n");

	}while ( choice != END);



	return 0;
}


STATUS exDup()
{
	int fd = dup(STDOUT_FILENO);
	if ( fd < 0)
	{
		printf("\ndup() failed : %d", errno);
		return FAIL;
	}

	write(fd, MSG, strlen(MSG));

	return SUCCESS;

}

STATUS exDup_()
{
	int logger = open(LOGGER, O_WRONLY | O_CREAT | O_TRUNC, S_IWRITE);  
	if ( logger < 0)
	{
		printf("\nopen() file %s failed : %d", LOGGER, errno);
		return FAIL;
	}

	// The lowest file descriptor now stdout, so close stdout
	if ( close(STDOUT_FILENO) < 0)
	{
		close(logger);
		printf("\nclose() stdout failed");
		return FAIL;
	}

	// Now dup() will duplicate the logger descriptor and will get the lowest descriptor number which is STDOUT_FILENO
	if ( dup(logger) != STDOUT_FILENO)
	{
		close(logger);
		printf("\ndup() failed");
		return FAIL;
	}

	// As everything succeeds, we may close the original file
	close(logger);

	// stdout redirection to a file
	printf("\nNow all stdout will be redirecting to logger\n");
	puts(MSG);

	return SUCCESS;

}


STATUS exDup2()
{
	int logger = open(LOGGER, O_WRONLY | O_APPEND | O_CREAT , S_IWRITE);  
	if ( logger < 0)
	{
		printf("\nopen() file %s failed : %d", LOGGER, errno);
		return FAIL;
	}

	// Now dup2() will duplicate the logger descriptor and will give new descriptor number as STDOUT_FILENO
	if ( dup2(logger, STDOUT_FILENO) < 0)
	{
		close(logger);
		printf("\ndup2() failed");
		return FAIL;
	}

	// As everything succeeds, we may close the original file
	close(logger);

	// stdout redirection to a file
	printf("\nNow all stdout will be redirecting to logger\n");
	puts(MSG);

	return SUCCESS;
	

}

void switchcase()
{
	int ch = -1;
	
	switch(ch)
	{
		case 1: printf("\nI'm in case 1"); break;
		default: printf("\nI'm in default"); break;
		case -1: printf("\nI'm in case -1");
	}
}


