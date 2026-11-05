// g++ examples.cpp -o examples
//
// dup and dup2: redirecting stdout to a file, and restoring it afterwards.

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

typedef enum
{
	SUCCESS,
	FAIL
}STATUS;


STATUS exDup();
STATUS exDup_();
STATUS exDup2();


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
	const char *name;
	STATUS (*funcPtr)(void);
}LOOKUP;

static const LOOKUP data[] = {
	{ DUP_STDOUT,                    "1. dup     -> duplicating stdout",             exDup },
	{ DUP_STDOUT_TO_FILE,            "2. dup     -> redirect stdout to file",        exDup_},
	{ DUP2_STDOUT_TO_FILE,           "3. dup2    -> redirect stdout to file",        exDup2},
	{ END,                           "NULL",                                         NULL }
};

#define LOGGER "logger"

static const char MSG[] = "Hello ... This is a sample message !!!!!!";

// The stdio buffer must be flushed before each switch, or its text ends up in the wrong place
static int saveStdout()
{
	fflush(stdout);
	return dup(STDOUT_FILENO);
}

static void restoreStdout(int saved)
{
	fflush(stdout);
	dup2(saved, STDOUT_FILENO);
	close(saved);
}


int main()
{
	int i, choice = 0;

	do
	{
		for ( i = 0; data[i].index != END; i++ )
			printf("\n%s", data[i].name);

		printf("\n%d. Exit", END);
		printf("\nEnter your choice : ");

		int ret = scanf("%d", &choice);

		if ( ret == EOF )
			break;

		if ( ret != 1 )   // Not a number: skip the rest of the line
		{
			int c;
			while ( (c = getchar()) != '\n' && c != EOF );
			choice = 0;
		}

		for ( i = 0; data[i].index != END; i++ )
		{
			if ( data[i].index == choice )
			{
				data[i].funcPtr();
				break;
			}
		}

		printf("\n\n");

	} while ( choice != END );

	return 0;
}


STATUS exDup()
{
	int fd = dup(STDOUT_FILENO);
	if ( fd < 0)
	{
		perror("dup");
		return FAIL;
	}

	fflush(stdout);

	if ( write(fd, MSG, strlen(MSG)) == -1 )
		perror("write");

	close(fd);

	return SUCCESS;
}

STATUS exDup_()
{
	int saved, logger;

	logger = open(LOGGER, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if ( logger < 0)
	{
		printf("\nopen() file %s failed : %s", LOGGER, strerror(errno));
		return FAIL;
	}

	saved = saveStdout();
	if ( saved < 0 )
	{
		perror("dup");
		close(logger);
		return FAIL;
	}

	// dup() always uses the lowest free descriptor, so close stdout first...
	close(STDOUT_FILENO);

	// ...and the copy of logger then becomes descriptor 1, which is stdout
	if ( dup(logger) != STDOUT_FILENO)
	{
		close(logger);
		restoreStdout(saved);
		printf("\ndup() failed");
		return FAIL;
	}

	// As everything succeeded, the original descriptor isn't needed
	close(logger);

	printf("Now all stdout will be redirecting to logger\n");
	puts(MSG);

	restoreStdout(saved);
	printf("\nWrote the message to file '%s'. stdout is restored.", LOGGER);

	return SUCCESS;
}


// dup2 does the same as close + dup in one step, so another thread can't take descriptor 1 in between
STATUS exDup2()
{
	int saved, logger;

	logger = open(LOGGER, O_WRONLY | O_APPEND | O_CREAT, 0644);
	if ( logger < 0)
	{
		printf("\nopen() file %s failed : %s", LOGGER, strerror(errno));
		return FAIL;
	}

	saved = saveStdout();
	if ( saved < 0 )
	{
		perror("dup");
		close(logger);
		return FAIL;
	}

	if ( dup2(logger, STDOUT_FILENO) < 0)
	{
		close(logger);
		restoreStdout(saved);
		printf("\ndup2() failed");
		return FAIL;
	}

	close(logger);

	printf("Now all stdout will be redirecting to logger\n");
	puts(MSG);

	restoreStdout(saved);
	printf("\nAppended the message to file '%s'. stdout is restored.", LOGGER);

	return SUCCESS;
}
