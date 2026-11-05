// g++ cat.cpp -o cat
//
// A simple cat, to show broken pipe handling:   ./cat bigfile | head -1
//
// When head exits, the next write gets SIGPIPE, which terminates the process by default.
// With SIGPIPE ignored, as here, the write fails with EPIPE instead, and the program must
// check for that, or it would keep reading the whole file for nothing.

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>

int displayFile(const char *filename);

int main(int argc, char *argv[])
{
	if ( argc != 2)
	{
		fprintf(stderr, "usage: %s <filename>\n", argv[0]);
		return 1;
	}

	signal(SIGPIPE, SIG_IGN);

	return (displayFile(argv[1]) == 0) ? 0 : 1;
}

int displayFile(const char *filename)
{
	int c = 0;
	int ret = 0;
	FILE *fp = fopen(filename, "r");

	if ( fp == NULL)
	{
		perror(filename);
		return -1;
	}

	while ( (c = fgetc(fp)) != EOF)
	{
		if ( putc(c, stdout) == EOF )
		{
			fprintf(stderr, "write failed : %s\n", strerror(errno));
			ret = -1;
			break;
		}
	}

	if ( ferror(fp) )
	{
		perror(filename);
		ret = -1;
	}

	fclose(fp);

	// Buffered output can fail here too, for a small file
	if ( fflush(stdout) == EOF && ret == 0 )
	{
		fprintf(stderr, "write failed : %s\n", strerror(errno));
		ret = -1;
	}

	return ret;
}
