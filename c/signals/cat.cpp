#include "stdio.h"
#include "stdlib.h"
#include "signal.h"

int displayFile(char *filename);
void sigHandler(int sig);

int main(int argc, char *argv[])
{
	if ( argc != 2)
	{
		printf("\nPass filename as argument");
		return -1;
	}

	signal(SIGPIPE, sigHandler);

	displayFile(argv[1]);
}

int displayFile(char *filename)
{
	int c = 0;
	FILE *fp = fopen(filename, "r");
	if ( fp == NULL)
	{
		printf("\nFile not found");
		return -1;
	}

	while ( (c = fgetc(fp)) != EOF)
		putc(c, stdout);

	fclose(fp);

	return 0;
}

void sigHandler(int sig)
{
	printf("\nReceived signal %d", sig);
}
