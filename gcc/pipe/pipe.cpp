
/***************************** pipe ********************************************
 *
 * Pipes and FIFOs (also known as named pipes) provide a unidirectional interprocess communication channel.
 * A pipe has a read end and a write end.
 * Data written to the write end of a pipe can be read from the read end of the pipe
 * Data written to the write end of the pipe is buffered by the kernel until it is read from the read end of the pipe.
 *
 * int pipe(int pipefd[2])            // use _pipe for windows
 *
 * pipefd[0]   -> for reading
 * pipefd[1]   -> for writing
 *
 * return      -> pipe() = 0
 *
 * header      -> unistd.h          ( pipe(), fork() )
 *             -> sys/wait.h        ( wait(), waitpid())
 *
 * reference   -> https://www.cs.rutgers.edu/~pxk/416/notes/c-tutorials/pipe.html
 *
*******************************************************************************/

/***************************** Pipe Capacity ***********************************
 *
 * A pipe has a limited capacity.
 * If the pipe is full, then a write() will block or fail
 * An application should be designed so that a reading process consumes data as soon as it is available, so that a writing process does not remain blocked. 
 * In linux pipe buffer size is PIPE_BUF = 4096
 *
*******************************************************************************/

/***************************** Named pipe **************************************
 *
 *  A traditional pipe is "unnamed" and lasts only as long as the process.
 *  A named pipe, however, can last as long as the system is up, beyond the life of the process.
 *  It can be deleted if no longer used.
 *  Usually a named pipe appears as a file, and generally processes attach to it for inter-process communication.
 *
 * int mkfifo(const char *pathname, mode_t mode)         -> make a FIFO special file (a named pipe)
 * int unlink(const char *path)                          -> remove a link to a file
 *
 * return      -> mkfifo() = unlink() = 
 * 
*******************************************************************************/

/***************************** Linux *******************************************
 *
 * fork()           -> fork() causes a process to duplicate. The child process is an almost-exact duplicate of the original parent process.
 *                  -> it inherits a copy of its parent's code, data, stack, open file descriptors, and signal table.
 *                  -> However, the parent and child have different process id numbers and parent process id numbers
 *                  -> http://www.cs.cityu.edu.hk/~lwang/fork
 *
 * getpid           -> Returns the process ID of the calling process
 * getppid          -> Returns the parent's process ID of the calling process
 * wait()           -> It suspends execution of the calling process until one of its children terminates
 * waitpid()        -> It suspends execution of the calling process until a child specified by pid argument has changed state 
 *
 * atexit()         -> Register a function to be called at normal process termination
 * exit()           -> The exit() function causes normal process termination and the value of status & 0377 is returned to the parent and atexit() wil be invoked
 * _exit()          -> Same as exit() but _exit() will not invoke atexit() and SIGCHLD signal will be send it to parent, child processes should use _exit()
 * 
 * abort()          -> terminate the current process, atexit() will not be called
 *                  -> SIGABRT signal will be raised and the signal handler will be invoked but signal handler will not return and process will be terminate.
 *                  -> Use abort() if your program is in a possibly corrupt state and you consider it too dangerous to try to do anything further.
 *                  -> exit() will cause any atexit functions, and in C++ destructors of static objects, to be called.
 *                  -> This is usually what you want for a clean exit, but it could be catastrophic if, for example, they overwrite a file with corrupt data
 *
 *
 * pid_t fork(void)                                     ( -1 fork failed || 0 to child process || pid of the new child process to parent process)
 *
 * pid_t wait(int *status)                              ( -1 'no / no more' child processes || pid of the exited child process)
 * pid_t waitpid(pid_t pid, int *status, int options)
 *
 * int atexit(void (*function)(void))
 * void exit(int status)
 * void _exit(int status)
 *
 * pid_t getpid(void)
 * pid_t getppid(void)
 *
 * wait (&status) is equal to waitpid(-1, &status, 0)
 *  
 * in waitpid()  -> pid can be  < -1  meaning wait for any child process whose process group ID is equal to the absolute value of pid
 *                              -1    meaning wait for any child process
 *                              0     meaning wait for any child process whose process group ID is equal to that of the calling process
 *                              > 0   meaning wait for the child whose process ID is equal to the value of pid
 *
 * exec() - https://stackoverflow.com/questions/20823371/what-is-the-difference-between-the-functions-of-the-exec-family-of-system-calls
 *
 * --------------------------------- zombie ------------------------------------
 * 
 * A child that terminates, but has not been waited for becomes a "zombie".
 *
 * The kernel maintains a minimal set of information about the zombie process (PID, termination status, resource usage information) in order to allow the parent to later perform a wait to obtain information about the child.
 *
 * As long as a zombie is not removed from the system via a wait(), it will consume a slot in the kernel process table, and if this table fills, it will not be possible to create further processes.
 *
 * If a parent process terminates, then its "zombie" children (if any) are adopted by init(1), which automatically performs a wait to remove the zombies
 *
 *
 * # ps -eaf  ( Now check the output of this command (within 2 minute) in another window )
 *
 * The child process is marked as <defunct>, and its status code is Z, for zombie
 *
 * When the program exits, the child process is inherited by init. This process should cleans up the zombie proces automatically.
 *
 *
 *
 * --------------------------------- POSIX -------------------------------------
 *
 * POSIX  -> Portable operating system interface
 *
 * POSIX is a family of standards specified by the IEEE Computer Society for maintaining compatibility between operating systems.
 *
 * POSIX defines the API, along with command line shells and utility interfaces, for software compatibility with variants of Unix and other operating systems
 *
 * Many systems follow POSIX closely, but few are actually certified by the Open Group which maintains the standard. Notable certified ones include
 *
 *   -> AIX (IBM)
 *   -> HP-UX (HP) 
 *   -> Solaris (Oracle)
 *   -> OSX (Apple)
 *
 *   Most Linux distros are very compliant, but not certified because they don't want to pay the compliance check 

*******************************************************************************/

/***************************** Neccessity of piping ****************************
 *
 * You are writing into the pipe and reading from it. That one can do even from a file.
 * Now let’s see another scenario. If we add more writes before read anything and then start reading, first read call will fetch the first written message and so on
 * pipe() is helpful in Implementing a QUEUE strategy(First in First out)
 * FIFO: the message which was written first will be available for the first read, then the message which came second, for the second read
 *
*******************************************************************************/

/***************************** Broken pipe *************************************
 *
 * A broken pipe is one, where the reader end has been closed and the writer end is being written into, for example
 *
 * # cat foo | less
 *
 * The cat process holds the writing end of the pipe, and the less process the reading one.
 * If the reader process closes the pipe, the pipe is broken (and therefore useless).
 * The writer process will receive the broken pipe error ( SIGPIPE signal ) from the operating system
 *
*******************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>


#ifdef _WIN32
	#include <io.h>
	#include <process.h>
#else
	#include <unistd.h>
	#include <sys/wait.h>
#endif

typedef enum
{
	FAIL = -1,
	SUCCESS = 0
}STATUS;

int pipe_within_same_process();
STATUS pipe_between_parent_child_process();
STATUS pipe_between_processes();
STATUS pipe_between_two_child_processes();
STATUS pipe_run_cmd();


void run_pipe(int fd[]);
void run_source(int fd[]);
void rundest(int fd[]);
void zombie();
void cleanup();

int main()
{
	int choice = 0;
	//printf("\nPipe buffer size : %d\n", PIPE_BUF );

	atexit(cleanup);

	printf("\n\n\n --------- Select your choice -----------");
	printf("\n1. pipe within same process");
#ifndef _WIN32
	printf("\n2. pipe between client and parent process");
	printf("\n3. pipe between processes with cmds");
	printf("\n4. Pipe between two child processes");
	printf("\n5. Pipe run cmd - 'ls -lh | grep pipe.cpp");
	printf("\n6. zombie");
#endif
	printf("\n7. Exit");
	printf("\nEnter your choice : ");	
	scanf("%d", &choice);

	switch(choice)
	{
		case 1: pipe_within_same_process(); break;
#ifndef _WIN32
		case 2: pipe_between_parent_child_process(); break;
		case 3: pipe_between_processes(); break;
		case 4: pipe_between_two_child_processes(); break;
		case 5: pipe_run_cmd(); 
		case 6: zombie(); break;
#endif
		case 7: printf("\nExiting"); break;
		default: printf("\nEntered an invalid choice !! Try again"); break;
	}

	return 0;
}


int pipe_within_same_process()
{
	int ret = 0;
	int fd[2] = {0};
	char data[] = "Hello ...., this is the simple example of pipe within the same process";
	char rcv[1024] = {0};

#ifdef _WIN32
	ret = _pipe(fd, 128, O_BINARY);
#else
	ret = pipe(fd);
#endif
	if ( ret == -1)
	{
		printf("\npipe() failed : %d", errno);
		return FAIL;
	}


	ret = write(fd[1], data, strlen(data));

	ret = read(fd[0], rcv, sizeof(rcv));

	printf("\nRCVED %d bytes: %s", ret, rcv);

	return SUCCESS;

}

#ifndef _WIN32

STATUS pipe_between_parent_child_process()
{
	int ret = 0, status = 0;
	pid_t pid = 0;
	int fd[2] = {0};
	char data[] = "Hello ...., this is the simple example of pipe_between_parent_child_process";
	char rcv[1024] = {0};

	ret = pipe(fd);
	if ( ret == -1)
	{
		printf("\npipe() failed : %d", errno);
		return FAIL;
	}

	pid = fork();

	if ( pid == -1)
	{
		printf("\nfork() failed : %d", errno);
		return FAIL;
	}
	else if ( pid == 0)
	{
		close(fd[1]);
		ret = read(fd[0], rcv, sizeof(rcv));
		printf("\nRCVED %d bytes: %s", ret, rcv);
		puts(" ");
		_exit(EXIT_SUCCESS); // very important, if we not give exit() caller will get return value from child also, along with parent process return
	}
	else
	{
		close(fd[0]);
		write(fd[1], data, strlen(data));
	        waitpid(pid, &status, 0);
	}

	return SUCCESS;
}

STATUS pipe_between_processes()
{
	int ret = 0;
	pid_t pid = 0, status = 0;
	int fd[2] = {0};

	ret = pipe(fd);
	if ( ret == -1)
	{
		printf("\npipe() failed : %d", errno);
		return FAIL;
	}

	switch ( pid = fork() )
	{
		case 0:   // child
			run_pipe(fd);
			_exit(EXIT_SUCCESS);
			break;
		default:  // parent
			while ( (pid = wait( &status)) != -1)
				fprintf(stderr, "\nprocess %d exits with %d", pid, WEXITSTATUS(status));
			break;
		case -1:
			printf("\nfork() failed");
			break;
	}

        return SUCCESS;	
}

STATUS pipe_between_two_child_processes()
{
	int ret = 0;
	pid_t pid = 0, status = 0;
	int fd[2] = {0};

	ret = pipe(fd);
	if ( ret == -1)
	{
		printf("\npipe() failed : %d", errno);
		return FAIL;
	}

	switch ( pid = fork() )
	{
		case 0:   // child
			run_pipe(fd);
			_exit(EXIT_SUCCESS);
		default:  // parent
			while ( (pid = wait( &status)) != -1)
				fprintf(stderr, "\nprocess %d exits with %d", pid, WEXITSTATUS(status));
			break;
		case -1:
			printf("\nfork() failed");
			break;
	}

        return SUCCESS;	
}

STATUS pipe_run_cmd()
{
	int fd[2];
	
	pipe(fd);

	if ( !fork() )
	{
		close(STDOUT_FILENO);
		dup(fd[1]);
		close(fd[0]);
		execlp("ls", "ls", "-lh", NULL);
	}
	else
	{
		close(STDIN_FILENO);
		dup(fd[0]);
		close(fd[1]);
		execlp("grep", "grep", "pipe.cpp", NULL);
	}
}

char *cmd1[] = { "/bin/ls", "-al",  NULL };
char *cmd2[] = { "/usr/bin/tr", "a-z", "A-Z", NULL };

void run_pipe(int fd[])
{
	pid_t pid = 0;

	switch ( pid = fork() )
	{
		case 0: //child
			close(fd[1]);                 // the child does not need this end of the pipe
			dup2(fd[0], STDIN_FILENO);    // this end of the pipe becomes the standard output
			execvp(cmd2[0], cmd2);        // run the command
			perror(cmd2[0]);              // print error if failed
			_exit(EXIT_SUCCESS);
		default: //parent
			close(fd[0]);                 // the parent does not need this end of the pipe
			dup2(fd[1], STDOUT_FILENO);   // this end of the pipe becomes the standard output
			execvp(cmd1[0], cmd1);        // run the command
			perror(cmd1[0]);              // print error if failed
			wait(NULL);
			break;
		case -1:
			printf("\nfork() failed");
			break;
	}
}


void run_source(int fd[])                             // run the first part of the pipeline, cmd1 
{
	pid_t pid = 0;

	switch ( pid = fork() )
	{
		case 0:                               // child
			close(fd[0]);                 // this process don't need the other end
			dup2(fd[1], STDOUT_FILENO);   // this end of the pipe becomes the standard output 
			execvp(cmd1[0], cmd1);        // run the command
			perror(cmd1[0]);              // print error if failed
			_exit(EXIT_SUCCESS);
		default:                              //parent
			wait(NULL);
			break;
		case -1:
			printf("\nfork() failed");
			break;
	}
}


void rundest(int fd[])                               // run the second part of the pipeline, cmd2
{
	pid_t pid = 0;

	switch ( pid = fork() )
	{
		case 0:                               // child
			close(fd[1]);                 // this process don't need the other end
			dup2(fd[0], STDIN_FILENO);    // this end of the pipe becomes the standard output 
			execvp(cmd2[0], cmd2);        // run the command
			perror(cmd2[0]);              // print error if failed
			_exit(EXIT_SUCCESS);
		default:                              // parent
			wait(NULL);
			break;
		case -1:
			printf("\nfork() failed");
			break;
	}
}

void zombie()
{
	pid_t pid = 0;

	pid = fork();

	if ( pid > 0 )      // parent process
		sleep(120);
	else
		exit(0);

}

#endif

void cleanup()
{
	printf("\nHi buddy, all set for exit !!!!!!!!");
	printf("\nI'm exiting now");
}

