// g++ named_pipe_reader.cpp -o named_pipe_reader
//
// See named_pipe_writer.cpp

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

int main()
{
	int fd;
	ssize_t ret;

	const char myfifo[] = "/tmp/myfifo";
	char buf[1024];

	// Whichever side starts first creates it, so EEXIST is fine
	if ( mkfifo(myfifo, 0600) == -1 && errno != EEXIST )
	{
		perror("mkfifo");
		return EXIT_FAILURE;
	}

	fd = open(myfifo, O_RDONLY);
	if ( fd < 0)
	{
		perror("open");
		return EXIT_FAILURE;
	}

	ret = read(fd, buf, sizeof(buf) - 1);   // Leave room for the NUL

	close(fd);

	if ( ret == -1 )
	{
		perror("read");
		return EXIT_FAILURE;
	}

	buf[ret] = '\0';

	printf("Read %zd bytes => %s\n", ret, buf);

	return EXIT_SUCCESS;
}
