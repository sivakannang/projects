


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>


int main()
{

	int fd = 0;
	int ret = 0;

	char myfifo[] = "/tmp/myfifo";
	char msg[] = "Hello ... This is simple data to pipe";

	mkfifo(myfifo, 0666);

	fd = open(myfifo, O_WRONLY);
	if ( fd < 0 )
	{
		printf("\nopen() failed : %d", errno);
		return EXIT_FAILURE;
	}

	ret = write(fd, msg, strlen(msg));
	printf("\nWritten %d bytes => %s", ret, msg);
	fflush(stdout);

	close(fd);

	printf("\npipe unlink() in process");
	fflush(stdout);

	unlink(myfifo);

	printf("\npipe unlink() done");
	fflush(stdout);

	return EXIT_SUCCESS;
}
