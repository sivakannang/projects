// g++ named_pipe_writer.cpp -o named_pipe_writer
//
// Run this and named_pipe_reader in two windows, in either order.
// open() blocks until the other side has opened the FIFO too.

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int main()
{
	int fd;
	ssize_t ret;

	const char myfifo[] = "/tmp/myfifo";
	const char msg[] = "Hello ... This is simple data to pipe";

	// Whichever side starts first creates it, so EEXIST is fine
	if ( mkfifo(myfifo, 0600) == -1 && errno != EEXIST )
	{
		perror("mkfifo");
		return EXIT_FAILURE;
	}

	fd = open(myfifo, O_WRONLY);
	if ( fd < 0 )
	{
		perror("open");
		return EXIT_FAILURE;
	}

	ret = write(fd, msg, strlen(msg));
	if ( ret == -1 )
		perror("write");
	else
		printf("Written %zd bytes => %s\n", ret, msg);

	close(fd);

	// This only removes the name. A reader that already has it open can still read the data.
	unlink(myfifo);

	return (ret == -1) ? EXIT_FAILURE : EXIT_SUCCESS;
}
