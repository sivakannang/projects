/***************************** Dirent ******************************************
 *
 * g++ dirent.cpp -o dirent
 * ./dirent <path>
 *
 * dirent.h opens, reads and closes directories.
 * unistd.h provides rmdir(), chdir() and getcwd(), and sys/stat.h provides mkdir().
 *
*******************************************************************************/

/***************************** API definitions *********************************
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
 * 	off_t          d_off;       // not an offset; see NOTES in man readdir
 * 	unsigned short d_reclen;    // length of this record
 * 	unsigned char  d_type;      // type of file; DT_UNKNOWN on some filesystems
 * 	char           d_name[256]; // filename
 * };
 *
 * return -> mkdir, rmdir, chdir, closedir: 0 on success, -1 on error
 *        -> getcwd, opendir: valid pointer on success, NULL on error
 *        -> readdir: valid pointer, or NULL at the end of the directory or on error
 *
*******************************************************************************/

#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>


typedef enum
{
	FAIL = -1,
	SUCCESS = 0
}STATUS;

bool Dir_Exists(const char *path);
DIR *Dir_Open(const char *path);
STATUS Dir_Close(DIR *dir);

STATUS Dir_Make(const char *path);
STATUS Dir_Remove(const char *path);
char *Dir_GetCurrentPath(char *path, size_t size);

STATUS Dir_List(const char *path, int level);

void help();


int main(int argc, char *args[])
{
	if ( argc != 2)
	{
		help();
		return 1;
	}

	return (Dir_List(args[1], 0) == SUCCESS) ? 0 : 1;
}

void help()
{
	printf("usage: dirent <path>\n");
}

DIR *Dir_Open(const char *path)
{
	DIR *dir = opendir(path);

	if ( dir == NULL)
		printf("opendir() failed : %s -> %s\n", path, strerror(errno));

	return dir;
}


STATUS Dir_Close(DIR *dir)
{
	if ( closedir(dir) != 0 )
	{
		printf("closedir() failed : %s\n", strerror(errno));
		return FAIL;
	}

	return SUCCESS;
}

bool Dir_Exists(const char *path)
{
	struct stat sb;

	return (stat(path, &sb) == 0) && S_ISDIR(sb.st_mode);
}

STATUS Dir_Make(const char *path)
{
	if ( mkdir(path, 0775) != 0 )
	{
		printf("mkdir() failed : %s -> %s\n", path, strerror(errno));
		return FAIL;
	}

	return SUCCESS;
}

STATUS Dir_Remove(const char *path)
{
	if ( rmdir(path) != 0 )
	{
		printf("rmdir() failed : %s -> %s\n", path, strerror(errno));
		return FAIL;
	}

	return SUCCESS;
}

char *Dir_GetCurrentPath(char *path, size_t size)
{
	char *ptr = getcwd(path, size);

	if ( ptr == NULL)
		printf("getcwd() failed -> %s\n", strerror(errno));

	return ptr;
}


// d_type isn't filled in on every filesystem, so ask lstat when it is DT_UNKNOWN.
// lstat doesn't follow symlinks, so a symlink loop can't cause endless recursion.
static bool isDirectory(const char *path, const struct dirent *entry)
{
	struct stat sb;

	if ( entry->d_type != DT_UNKNOWN )
		return entry->d_type == DT_DIR;   // A value, not a bit mask: DT_SOCK also contains the DT_DIR bit

	return (lstat(path, &sb) == 0) && S_ISDIR(sb.st_mode);
}

STATUS Dir_List(const char *path, int level)
{
	STATUS status = SUCCESS;
	struct dirent *entry = NULL;

	DIR *dir = Dir_Open(path);
	if ( dir == NULL)
		return FAIL;

	errno = 0;

	while ( (entry = readdir(dir)) != NULL ) {

		char newPath[PATH_MAX];

		if ( (strcmp(entry->d_name, ".") == 0) || (strcmp(entry->d_name, "..") == 0) )
			continue;

		printf("%*s- %s\n", level*2, "", entry->d_name);

		if ( snprintf(newPath, sizeof(newPath), "%s/%s", path, entry->d_name) >= (int)sizeof(newPath) )
		{
			printf("%*s  path too long, skipping\n", level*2, "");
			status = FAIL;
		}
		else if ( isDirectory(newPath, entry) && Dir_List(newPath, level+1) != SUCCESS )
		{
			status = FAIL;
		}

		errno = 0;   // Only an errno set by readdir itself should count below
	}

	if ( errno != 0 )
	{
		printf("readdir() failed : %s -> %s\n", path, strerror(errno));
		status = FAIL;
	}

	Dir_Close(dir);

	return status;
}
