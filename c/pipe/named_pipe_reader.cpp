#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>


int main()
{
	
	int fd = 0;
	int ret = 0;

	char myfifo[] = "/tmp/myfifo";
	char buf[1024] = {0};

	fd = open(myfifo, O_RDONLY);
	if ( fd < 0)
	{
		printf("\nopen() failed : %d", errno);
		return EXIT_FAILURE;
	}

	ret = read(fd, buf, sizeof(buf));

	close(fd);

	printf("\nRead %d bytes => %s %d", ret, buf, sizeof(buf));

	return EXIT_SUCCESS;
}
