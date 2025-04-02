/***************************** Dirent ******************************************
 *
 * Dirent use to perform open, read and close operations on directories.
 *
 * dirent.h is the part of gcc/g++ sdk kit and not the part of visual c++ kit, so for windows it need to be include explicitly.
 *
 * unistd.h and direct.h(windows) provides, mkdir(), rmdir(), chdir() and getcwd() features
 *
*******************************************************************************/

/***************************** API defenitions *********************************
 *
 * char *getcwd(char *buf, size_t size)
 * int mkdir(const char *pathname, mode_t mode)
 * int rmdir(const char *pathname)
 * int chdir(const char *path);
 *
 * DIR *opendir(const char *name)
 * struct dirent *readdir(DIR *dirp)
 * int closedir(DIR *dirp)
 *
 * struct dirent {
 * 	ino_t          d_ino;       // inode number
 * 	off_t          d_off;       // not an offset; see NOTES
 * 	unsigned short d_reclen;    // length of this record
 * 	unsigned char  d_type;      // type of file; not supported by all filesystem types
 * 	char           d_name[256]; // filename
 * };
 *
 * return        -> mkdir() = rmdir() = chdir() = closedir() = 0, getcwd() = opendir() = readdir() = valid pointer
 * header        -> dirent.h, unistd.h (linux) , direct.h(windows)
 *
*******************************************************************************/





#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <ctype.h>


#ifdef _WIN32
#include "dirent.h"
#include <direct.h>
bool isWindows = true;
#else
#include <dirent.h>
#include <unistd.h>
bool isWindows = false;
#endif


typedef enum
{
	FAIL = -1,
	SUCCESS = 0
}STATUS;

bool Dir_Exists(char *path);
DIR *Dir_Open(char *path);
STATUS Dir_Close(DIR *dir);

STATUS Dir_Make(char *path);
STATUS Dir_Remove(char *path);
char *Dir_GetCurrentPath(char *path);

STATUS Dir_List(char *path, int level);

bool ISDIGIT_STR(const char *str);
STATUS makeAlienoPackage(char *srcPath, char *dstPath, char *nameIdx, int level);
char *getNameIdx(char *dirName, char *nameIdx);

void help();


int main(int argc, char *args[])
{
	if ( argc != 2)
	{
		help();
		return FAIL;
	}

	Dir_List(args[1], 0);


	return 0;
}

void help()
{
	printf("\n\n---------------------------");
	printf("\nNumber of arguments should be 2");
	printf("\ndirent path");
	printf("\n---------------------------");
}

DIR *Dir_Open(char *path)
{
	DIR *dir = opendir(path);
	if ( dir == NULL)
	{
		printf("\ndiropen() failed : %s -> %d", path, errno);
		return NULL;
	}

	return dir;
}


STATUS Dir_Close(DIR *dir)
{
	int ret = closedir(dir);

	if ( ret ){
		printf("\nclosedir() failed : %d", errno);
		return FAIL;
	}

	return SUCCESS;
}

bool Dir_Exists(char *path)
{
	struct stat sb;

	if ( (stat(path, &sb) == 0) && S_ISDIR(sb.st_mode) )
		return true;

	return false;
}

STATUS Dir_Make(char *path)
{

#ifdef _WIN32
	int ret = _mkdir(path);
#else
	int ret = mkdir(path, 0775);
#endif
	
	if ( ret != SUCCESS)
	{
		printf("\nmkdir() failed : %s -> %d", path, errno);
		return FAIL;
	}

	return SUCCESS;
}

STATUS Dir_Remove(char *path)
{

#ifdef _WIN32
	int ret = _rmdir(path);
#else
	int ret = rmdir(path);
#endif

	if ( ret != SUCCESS)
	{
		printf("\nrmdir() failed : %s -> %d", path, errno);
		return FAIL;
	}

	return SUCCESS;
}

char *Dir_GetCurrentPath(char *path)
{

#ifdef _WIN32
	char *ptr = _getcwd(path, 256);
#else
	char *ptr = getcwd(path, 256);
#endif

	if ( ptr == NULL)
	{
		printf("\ngetcwd() failed -> %d", errno);
		return NULL;
	}

	return ptr;

}


STATUS Dir_List(char *path, int level)
{
	STATUS status = FAIL;
	struct dirent *entry = NULL;
	char newPath[512] = {0};

	DIR *dir = Dir_Open(path);
	if ( dir == NULL)
		return FAIL;

	while ( entry = readdir(dir) ) {

		if ( entry->d_type & DT_DIR ) {
			
			if ( (strcmp(entry->d_name, ".") == 0) || (strcmp(entry->d_name, "..") == 0) )
				continue;
	
			printf("%*s- %s\n", level*2, "", entry->d_name);

			sprintf(newPath, "%s%s%s", path, isWindows ? "\\" : "/", entry->d_name);	
			Dir_List(newPath, level+1);

		}else{
			printf("%*s- %s\n", level*2, "", entry->d_name);
		}
	}

	Dir_Close(dir);

	return SUCCESS;
}

bool ISDIGIT_STR(const char *str)
{
	if ( !str || !*str )
		return false;

	while(isdigit(*str))
		str++;

	return *str ? false : true;
	
}


/***************************************************** Alieno Host Package Creation******************************/

char *getNameIdx(char *dirName, char *nameIdx)
{
	//1 2 15 f1 f15
	char *ptr = dirName;

	if ( ISDIGIT_STR(ptr))
	{
		sprintf(nameIdx, "%s_I_", dirName);
	}
	else
	{
		int grpNo = atoi(ptr + 1);
		char whr = ptr[0];
		whr = toupper(whr);
		sprintf(nameIdx, "%d_%c_", grpNo, whr);
	}	

	return nameIdx;

}


STATUS makeAlienoPackage(char *srcPath, char *dstPath, char *nameIdx, int level)
{
	STATUS status = FAIL;
	struct dirent *entry = NULL;
	char newPath[512] = {0};
	char newNameIdx[48] = {0};
	char dstFileName[256] = {0};

	DIR *dir = Dir_Open(srcPath);
	if ( dir == NULL)
		return FAIL;

	while ( entry = readdir(dir) ) {

		if ( entry->d_type & DT_DIR ) {
			
			if ( (strcmp(entry->d_name, ".") == 0) || (strcmp(entry->d_name, "..") == 0) )
				continue;
	
			printf("%*s- %s\n", level*2, "", entry->d_name);

			sprintf(newPath, "%s%s%s", srcPath, isWindows ? "\\" : "/", entry->d_name);
			//sprintf(newNameIdx, "%s%s_", nameIdx, entry->d_name);
			getNameIdx(entry->d_name, newNameIdx);
			makeAlienoPackage(newPath, dstPath, newNameIdx, level+1);

		}else{
			char cmd[1024] = {0};
			memset(dstFileName, 0, sizeof(dstFileName));	
			sprintf(dstFileName, "%s%s", nameIdx, entry->d_name);

			sprintf(cmd, "%s %s%s%s %s%s%s", isWindows ? "copy" : "cp", srcPath, isWindows ? "\\" : "/", entry->d_name,  dstPath, isWindows ? "\\" : "/", dstFileName);
			system(cmd);

			printf("%*s- %s -> %s -> %s\n", level*2, "", entry->d_name, dstFileName, cmd);
		}
	}

	Dir_Close(dir);

	return SUCCESS;
}

/***************************************************** Alieno Host Package Creation******************************/
