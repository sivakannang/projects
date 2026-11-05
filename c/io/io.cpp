/*************************** Scope *********************************************
 *
 * g++ io.cpp -o io
 *
 * Standard input and output format specifiers
 * Standard input and output files/devices
 * Standard input and output functions
 * Variable length arguments
 * Getting system time and logging
 * File operation functions
 * Command line arguments and environment variables
 *
 * Exercise : make 'cat' and 'tail' applications
 * ****************************************************************************/


/*************************** Printing / Reading data types *********************
 *
 * %d  - int (same as %i in printf. In scanf, %i also accepts 0x.. and 0.. prefixes)
 * %ld - long int (same as %li)
 * %zu - size_t
 * %f  - printf: double (float is promoted to double). scanf: float
 * %lf - printf: double. scanf: double
 * %c  - char
 * %s  - string (in scanf, always give a width such as %47s, or it can overflow)
 * %x  - hexadecimal
 * %o  - Octal
 * %u  - unsigned int
 * %p  - pointer
 *
 * int i = 20 can be represented in Decimal = 20, Hex = 0x14, Octal = 024, binary = 00010100
 *
 ******************************************************************************/



/*************************** Standard files ************************************
 *
 * C programming treats all the devices as files.
 * So devices such as the display are addressed in the same way as files and the following three files are automatically opened
 *
 *
 *     Standard File              File Pointer              Device        File descriptor No
 *
 *     Standard Input             stdin                     Keyboard      0
 *     Standard Output            stdout                    Screen        1
 *     Standard Error             stderr                    Screen        2
 *
 ******************************************************************************/



/*************************** Standard input/output *****************************
 *
 * int getc(FILE *fp)
 * int putc(int c, FILE *fp)
 *
 * int getchar(void)
 * int putchar(int c)
 *
 * char *fgets(char *s, int size, FILE *fp)     // gets() was removed in C11: it can't limit the length
 * int   puts(const char *s)
 *
 * int printf(const char *format, ...)
 * int scanf(const char *format, ...)
 *
 * int sprintf(char *str, const char *format, ...)
 * int snprintf(char *str, size_t size, const char *format, ...)
 * int sscanf(const char *str, const char *format, ...)
 *
 ******************************************************************************/



/*************************** High level file i/o *******************************
 *
 * FILE *fopen(const char *path, const char *mode)
 *
 * int fgetc(FILE *fp)
 * int fputc(int c, FILE *fp)
 *
 * char *fgets(char *s, int size, FILE *fp)
 * int   fputs(const char *s, FILE *fp)
 *
 * size_t fread(void *ptr, size_t size, size_t nmemb, FILE * fp)
 * size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *fp)
 *
 * int fprintf(FILE *fp, const char *format, ...)
 * int fscanf(FILE *fp, const char *format, ...)
 *
 * int fseek(FILE *fp, long offset, int whence)
 * long ftell(FILE *fp)
 * void rewind(FILE *fp)
 * int fflush(FILE *fp)
 * void __fpurge(FILE *stream)
 * int fileno(FILE *stream)
 * int ftruncate(int fildes, off_t length);
 * int fstat(int fildes, struct stat *buf);
 *
 * int fclose(FILE *fp)
 *
 * mode   => r | w | a | r+ | w+ | a+ | rb | wb | ab | r+b | w+b | a+b
 * whence => SEEK_SET | SEEK_CUR | SEEK_END
 *
 *
 * fflush()   -> writes buffered output to the file
 *            -> standard C for output streams. fflush(stdin) is undefined in standard C.
 *
 * __fpurge() -> discards buffered data, including unread input
 *            -> Linux, declared in stdio_ext.h. Non-standard (BSD has fpurge)
 *            -> to skip the rest of an input line, read up to '\n' with getchar() instead
 *
 ******************************************************************************/



/*************************** Low level file i/o ********************************
 *
 * int     open  ( const char *pathname, int flags, mode_t mode )    // mode is only used with O_CREAT
 * ssize_t read  ( int fd, void *buffer, size_t count )              // may read less than count
 * ssize_t write ( int fd, const void *buffer, size_t count )        // may write less than count
 * off_t   lseek ( int fd, off_t offset, int whence )
 * int     close ( int fd )
 *
 * flags  => O_RDONLY | O_WRONLY | O_RDWR | O_APPEND | O_TRUNC | O_CREAT | O_NONBLOCK
 * mode   => S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH ...  or octal such as 0644
 * whence => SEEK_SET | SEEK_CUR | SEEK_END
 *
 *
 * int dup(int oldfd)
 * int dup2(int oldfd, int newfd)
 * int dup3(int oldfd, int newfd, int flags)
 *
 * dup()         -> it creates a copy of the file descriptor oldfd, using the lowest-numbered unused descriptor for the new descriptor
 * dup2()        -> same as dup() but instead of using the lowest-numbered unused file descriptor, it uses the descriptor number specified in newfd
 *               -> If the descriptor newfd was previously open, it is silently closed before being reused
 * 
 ******************************************************************************/

/*************************** File operations **********************************
 *
 * int remove(const char *pathname);
 * int rename(const char *oldpath, const char *newpath);
 * int chmod(const char *pathname, mode_t mode);
 * int chown(const char *pathname, uid_t owner, gid_t group);
 * int mkdir(const char *pathname, mode_t mode);
 * int rmdir(const char *pathname);
 * int chdir(const char *path);
 * char *getcwd(char *buf, size_t size);
 * char *get_current_dir_name(void);
 * int stat(const char *pathname, struct stat *statbuf);
 * int truncate(const char *path, off_t length);
 *
 * ***************************************************************************/

#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>


typedef enum
{
	FAIL,
	SUCCESS
}STATUS;

int File_Read(const char *filename, char *buffer, int len);
char *File_Read(const char *filename);
int File_Write(const char *filename, const char *buffer, int len);
int File_Append(const char *filename, const char *buffer, int len);
int File_Log(const char* format, ...) __attribute__((format(printf, 1, 2)));
STATUS File_Print(const char *filename);

int average(int count, ...);
char *FGETS(char *s, int max, FILE *fp);
int FPUTS(const char *s, FILE *fp);


int File_Read(const char *filename, char *buffer, int len)
{
	int ret = 0;
	FILE * fp = fopen(filename, "r");
	if ( fp == NULL)
		return 0;

	ret = fread(buffer, 1, len, fp);

	fclose(fp);

	return ret;
}

// Returns the whole file as a NUL-terminated string, or NULL. The caller must free it.
char *File_Read(const char *filename)
{
	struct stat property;
	char *buffer = NULL;
	size_t ret = 0;

	FILE *fp = fopen(filename, "r");
	if ( fp == NULL)
		return NULL;

	if ( fstat(fileno(fp), &property) != 0 )
	{
		fclose(fp);
		return NULL;
	}

	buffer = (char *)malloc(property.st_size+1);
	if ( buffer == NULL )
	{
		fclose(fp);
		return NULL;
	}

	ret = fread(buffer, 1, property.st_size, fp);

	fclose(fp);

	buffer[ret] = 0;   // ret, because the file can shrink after fstat

	return buffer;
}


int File_Write(const char *filename, const char *buffer, int len)
{
	int ret = 0;
	FILE * fp = fopen(filename, "w");
	if ( fp == NULL)
		return 0;

	ret = fwrite(buffer, 1, len, fp);

	fclose(fp);

	return ret;
}

int File_Append(const char *filename, const char *buffer, int len)
{
	int ret = 0;
	FILE * fp = fopen(filename, "a");
	if ( fp == NULL)
		return 0;

	ret = fwrite(buffer, 1, len, fp);

	fclose(fp);

	return ret;
}

STATUS File_Print(const char *filename)
{
	int ch = 0;

	FILE * fp = fopen(filename, "r");
	if ( fp == NULL)
		return FAIL;

	while ( (ch = getc(fp)) != EOF)
		putc(ch, stdout);

	fclose(fp);

	return SUCCESS;
}

int File_Log(const char* format, ...)
{
	char buffer[4096];
	char logfile[30];
	time_t t = time(NULL);
	struct tm lt;
	va_list args;
	int fd, ret, len;

	localtime_r(&t, &lt);   // localtime returns a shared static buffer, which isn't thread-safe

	len = snprintf(buffer, sizeof(buffer), "\n%02d%02d%02d: %02d/%02d/%02d: => ", lt.tm_hour, lt.tm_min, lt.tm_sec, lt.tm_mon + 1, lt.tm_mday, lt.tm_year - 100);

	va_start(args, format);
	ret = vsnprintf(buffer + len, sizeof(buffer) - len, format, args);
	va_end(args);

	if ( ret > 0 )
		len = ((size_t)(len + ret) < sizeof(buffer)) ? len + ret : (int)sizeof(buffer) - 1;

	printf("%s", buffer);

	snprintf(logfile, sizeof(logfile), "log_%02d.txt", lt.tm_mon + 1);

	fd = open(logfile, O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR);
	if ( fd < 0 )
	{
		printf("\n errno = %d , errstr = %s", errno, strerror(errno));
		return -1;
	}

	ret = write(fd, buffer, len);

	close(fd);

	return ret;
}


char *FGETS(char *s, int max, FILE *fp)
{
	int c = 0;   // Stays 0 if max is too small for the loop to run
	char *p = s;

	while ( --max > 0 && (c = getc(fp)) != EOF)
		if ( (*p++ = c) == '\n')
			break;

	*p = '\0';

	return (c == EOF && p == s) ? NULL : s;
}

int FPUTS(const char *s, FILE *fp)
{
	int c;

	while( (c = *s++) )
		putc(c, fp);

	return ferror(fp) ? EOF : 0;
}


int average(int count, ...)
{
	int sum = 0;
	va_list valist;

	if ( count <= 0 )
		return 0;

	va_start(valist, count);

	for ( int i = 0; i < count; i++)
		sum += va_arg(valist, int);

	va_end(valist);

	return sum/count;
}


// Command line arguments and environment variables
int main(int argc, char *argv[], char **env)
{
	File_Log("Number of arguments : %d", argc);

	for ( int offset = 0; offset < argc; offset++ )
		File_Log("Argument[%02d] => %s", offset, argv[offset]);

	// Screen only: environment variables often hold tokens and passwords, so keep them out of log files
	while(*env)
		printf("\nEnvironment => %s", *env++);

	File_Log("average = %d", average(5, 1, 2, 3, 4, 5));

	printf("\n");

	return 0;
}

