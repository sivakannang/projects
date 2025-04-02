/*************************** Scope *********************************************
 *
 * Standard input and output format specifiers
 * Standard input and output files/devices
 * Standard input and output functions
 * Variable length arguments
 * Getting system time and logging
 * File structure and file operation functions
 * Command line arguments and environment variables
 *
 * Exercise : make 'cat', 'tail' application for windows
 * ****************************************************************************/


/*************************** Printing / Reading data types *********************
 *
 * %d  - int (same as %i)
 * %ld - long int (same as %li)
 * %f  - float
 * %lf - double
 * %c  - char
 * %s  - string
 * %x  - hexadecimal
 * %o  - Octal
 * %u  - unsigned int
 *
 * int i = 20 can be represent in Decimal  = 20, Hex = 0x14, Octal = 024, binary = 00010100
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
 * char *gets(char *s)
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
 * int fpurge(FILE *stream)
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
 * fflush()   -> flush the stream
 *            -> flush method for output stream and not for input stream
 *            -> standard
 *
 * fpurge     -> flush the stream ( can be use it for flush stdin )
 *            -> supported in stdio_ext.h and not in stdio.h 
 *            -> non standard, its from BSD
 *
 ******************************************************************************/



/*************************** Low level file i/o ********************************
 *
 * int  open  ( char  *filename,  int modes,  int  permission  )
 * int  read  ( int  fd,  void  *buffer,  int  nbyte )
 * int  write ( int  fd,  void  *buffer,  int  nbyte  )
 * off_t lseek( int fd, off_t offset, int whence)
 * int  close (  int  fd  )
 * 
 * modes      => O_RDONLY | O_WRONLY | O_RDWR | O_APPEND | O_BINARY | O_TEXT | O_TRUNC | O_CREAT
 * permission => S_IWRITE | S_IREAD
 * whence     => SEEK_SET | SEEK_CUR | SEEK_END
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
 * char *getcwd(char buf[.size], size_t size);
 * char *get_current_dir_name(void);
 * int stat(const char *restrict pathname, struct stat *statbuf);
 * int truncate(const char *path, off_t length);
 *
 * ***************************************************************************/

/********* Commnad line arguments and Environment variables capture ************
 
int main(int argc, char *argv[], char **env)
{
	int offset = 0;
	File_Log("Number of arguments : %d", argc);

	while ( offset < argc){
		File_Log("Argument[%02d] => %s", offset, argv[offset]);
		offset++;
	}


	while(*env)
		File_Log("Environment => %s", *env++);

	return 0;
}

******************************************************************************/

#ifdef _WIN32
	#include <io.h>

#else
	#include <unistd.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>


typedef enum
{
	FAIL,
	SUCCESS
}STATUS;

int File_Read(char *filename, char *buffer, int len);
int File_Write(char *filename, char *buffer, int len);
int File_Append(char *filename, char *buffer, int len);
int File_Log(const char* format, ...);
STATUS File_Print(char *filename);

int average(int count, ...);
char *FGETS(char *s, int max, FILE *fp);
int FPUTS(const char *s, FILE *fp);
int Time_Get(char *outField);
void printFileStruct(FILE *fp);




int File_Read(char *filename, char *buffer, int len)
{
	int ret = 0;
	FILE * fp = fopen(filename, "r");
	if ( fp == NULL)
		return 0;

	ret = fread(buffer, 1, len, fp);

	fclose(fp);

	return ret;
}

char *File_Read(char *filename)
{
	FILE *fp = NULL;
	struct stat property = {0};
	char *buffer = NULL;
	int ret = 0;

	stat(filename, &property);

	fp = fopen(filename, "r");
	if ( fp == NULL)
		return NULL;

	buffer = (char *)malloc(property.st_size+1);

	ret = fread(buffer, 1, property.st_size, fp);

	fclose(fp);

	buffer[property.st_size] = 0;

	printf("\nsize : =>%ld %d<=", property.st_size, ret);

	return buffer;

}


int File_Write(char *filename, char *buffer, int len)
{
	int ret = 0;
	FILE * fp = fopen(filename, "w");
	if ( fp == NULL)
		return 0;

	ret = fwrite(buffer, 1, len, fp);

	fclose(fp);

	return ret;
}

int File_Append(char *filename, char *buffer, int len)
{
	int ret = 0;
	FILE * fp = fopen(filename, "a");
	if ( fp == NULL)
		return 0;

	ret = fwrite(buffer, 1, len, fp);

	fclose(fp);

	return ret;
}

STATUS File_Print(char *filename)
{
	int ch = 0;

	FILE * fp = fopen(filename, "a");
	if ( fp == NULL)
		return FAIL;

	while ( (ch = getc(fp)) != EOF)
		putc(ch, stdout);

	fclose(fp);

	return SUCCESS;

}

int File_Log(const char* format, ...)
{
	int ret=0;
	int month = 0;
	int fp = 0;
	char buffer[15001] = {'\0'};
	char logfile[30] = {'\0'};	

	va_list args;

	month = Time_Get(buffer);

	va_start(args, format);
	vsprintf (&buffer[strlen(buffer)], format, args );
	va_end(args);	

	sprintf(logfile, "log_%02d.txt", month);

	printf("%s", buffer);


	fp = open (logfile, O_WRONLY|O_APPEND|O_CREAT, S_IWRITE | S_IREAD); // S_IWRITE - Open or Create with write permission 
	
	if(fp<0)
	{
		switch(errno)
		{
			case EACCES: //return _ERR_FILE_ACCESS; 
			case EMFILE: //return _ERR_FILE_TOO_MANY_HANDLERS;
			default: //return _ERR_FILE_UNKNOWN;
				printf("\n errno = %d , errstr = %s" , errno, strerror(errno));
				return -1;
		}
	}

	ret = write(fp, (const char *)buffer,strlen(buffer));

	close(fp);

	return ret;
	

}


char *FGETS(char *s, int max, FILE *fp)
{
	int c;
	char *p;

	p = s;
	while ( --max > 0 && (c = getc(fp)) != EOF)
		if ( (*p++ = c) == '\n')
			break;

	*p = '\0';

	return (c == EOF && p == s) ? NULL : s;

}

int FPUTS(const char *s, FILE *fp)
{
	int c;

	while( c = *s++)
		putc(c, fp);

	return ferror(fp) ? EOF : 0;
}

void printFileStruct(FILE *fp)
{
#ifdef _WIN32
	printf("\n\n\n------------------------------------------");
	printf("\n _ptr      =>%s<=", fp->_ptr);
	printf("\n _cnt      =>%d<=", fp->_cnt);
	printf("\n _base     =>%s<=", fp->_base);
	printf("\n _flag     =>%d<=", fp->_flag);
	printf("\n _file     =>%d<=", fp->_file);
	printf("\n _charbuf  =>%d<=", fp->_charbuf);
	printf("\n _bufsiz   =>%d<=", fp->_bufsiz);
	printf("\n _tmpfname =>%s<=", fp->_tmpfname);
#else
	
#endif
}


int Time_Get(char *outField)
{

	time_t t = time(0);
	struct tm* lt = localtime(&t);
 	char time_str[128] = {0};
	
	sprintf(time_str, "\n%02d%02d%02d: %02d/%02d/%02d: => ", lt->tm_hour, lt->tm_min, lt->tm_sec, lt->tm_mon + 1, lt->tm_mday, lt->tm_year - 100);

	strncat(outField, time_str, strlen(time_str));
	return lt->tm_mon + 1;
}


int average(int count, ...)
{
	int sum = 0;

	va_list valist;
	va_start(valist, count);

	for ( int i = 0; i < count; i++)
		sum += va_arg(valist, int);

	return sum/count;
}


int main(int argc, char *argv[], char **env)
{
	int offset = 0;
	File_Log("Number of arguments : %d", argc);

	while ( offset < argc){
		File_Log("Argument[%02d] => %s", offset, argv[offset]);
		offset++;
	}


	while(*env)
		File_Log("Environment => %s", *env++);

	File_Log("average = %d", average(5, 1, 2, 3, 4, 5));


	return 0;
}

