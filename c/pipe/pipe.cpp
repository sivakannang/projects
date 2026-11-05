// g++ pipe.cpp -o pipe

/***************************** pipe ********************************************
 *
 * Pipes and FIFOs (named pipes) provide a one-way channel between processes.
 * Data written to the write end is buffered by the kernel until it is read from the read end.
 * The data is a stream of bytes, read in first in, first out order, with no message boundaries.
 *
 * int pipe(int pipefd[2])
 *
 * pipefd[0]   -> for reading
 * pipefd[1]   -> for writing
 *
 * return      -> pipe() = 0, or -1 on error
 *
 * header      -> unistd.h          ( pipe(), fork() )
 *             -> sys/wait.h        ( wait(), waitpid())
 *
 * read() returns 0 (end of file) only when every copy of the write end is closed.
 * A process that forgets to close its copy can leave the reader waiting forever.
 *
*******************************************************************************/

/***************************** Pipe Capacity ***********************************
 *
 * When the pipe is full, write() blocks, or fails with EAGAIN on a nonblocking pipe.
 * When the pipe is empty, read() blocks.
 *
 * Capacity  -> 65536 bytes by default on Linux. fcntl(fd, F_SETPIPE_SZ, size) can change it.
 * PIPE_BUF  -> 4096 bytes on Linux. Not the capacity: writes up to this size are atomic, so
 *              data from several writers can't get mixed together within them.
 *
*******************************************************************************/

/***************************** Named pipe **************************************
 *
 * An ordinary pipe has no name, so only related processes (through fork) can share it,
 * and it is gone when the last process closes it.
 * A named pipe appears as a file, so unrelated processes can open it by path.
 * The file stays until it is unlinked, but the data in it is only kept while it is open.
 *
 * int mkfifo(const char *pathname, mode_t mode)         -> make a FIFO special file (a named pipe)
 * int unlink(const char *path)                          -> remove the name
 *
 * return      -> mkfifo() = unlink() = 0, or -1 on error
 *
 * open() on a FIFO blocks until the other end is opened too. See named_pipe_reader.cpp and named_pipe_writer.cpp.
 *
*******************************************************************************/

/***************************** Linux *******************************************
 *
 * fork()           -> duplicates the process. The child gets a copy of the parent's memory,
 *                     open file descriptors and signal handlers, with its own process id.
 * getpid           -> Returns the process ID of the calling process
 * getppid          -> Returns the parent's process ID of the calling process
 * wait()           -> suspends the caller until one of its children terminates
 * waitpid()        -> suspends the caller until a specific child changes state
 *
 * atexit()         -> Registers a function to be called at normal process termination
 * exit()           -> Runs atexit functions and C++ static destructors, flushes stdio, then terminates
 * _exit()          -> Terminates immediately: no atexit functions and no stdio flush.
 *                     Use it in a forked child that isn't going to exec, so the child can't
 *                     flush a second copy of the parent's buffered output.
 *                     Both exit() and _exit() send SIGCHLD to the parent.
 *
 * abort()          -> Raises SIGABRT. atexit functions are not called. The process still
 *                     terminates even if a handler returns.
 *                  -> For a corrupt state, where running cleanup code could make things worse.
 *
 *
 * pid_t fork(void)                                     ( -1 failed || 0 in the child || pid of the child in the parent )
 *
 * pid_t wait(int *status)                              ( -1 no more children || pid of the exited child )
 * pid_t waitpid(pid_t pid, int *status, int options)
 *
 * int atexit(void (*function)(void))
 * void exit(int status)
 * void _exit(int status)
 *
 * pid_t getpid(void)
 * pid_t getppid(void)
 *
 * wait(&status) is the same as waitpid(-1, &status, 0)
 *
 * in waitpid()  -> pid can be  < -1  wait for any child whose process group ID is the absolute value of pid
 *                              -1    wait for any child
 *                              0     wait for any child in the caller's process group
 *                              > 0   wait for the child with this process ID
 *
 * --------------------------------- zombie ------------------------------------
 *
 * A child that has terminated, but hasn't been waited for, is a zombie.
 * The kernel keeps its PID and exit status so that the parent can collect them with wait().
 * Each zombie uses a slot in the process table, and if the table fills up, no new processes can start.
 * If the parent terminates, init adopts the zombies and waits for them.
 *
 * Run option 6, and within 2 minutes run this in another window:  ps -eaf | grep defunct
 * The child is shown as <defunct>, with status Z in ps -el.
 *
 * --------------------------------- POSIX -------------------------------------
 *
 * POSIX (Portable Operating System Interface) is the IEEE family of standards that defines the
 * API and shell utilities that Unix-like systems share. Certified systems include AIX, HP-UX,
 * Solaris and macOS. Most Linux distributions follow it closely but aren't certified.
 *
*******************************************************************************/

/***************************** Broken pipe *************************************
 *
 * Writing to a pipe whose read end is closed by every process is a broken pipe, for example:
 *
 * # cat foo | less          ( quit less while cat is still writing )
 *
 * The writer receives SIGPIPE, which terminates it by default.
 * If SIGPIPE is ignored or handled, write() fails with EPIPE instead.
 *
*******************************************************************************/


#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

typedef enum
{
	FAIL = -1,
	SUCCESS = 0
}STATUS;

STATUS pipe_within_same_process();
STATUS pipe_between_parent_child_process();
STATUS pipe_between_processes();
STATUS pipe_between_two_child_processes();
STATUS pipe_run_cmd();

pid_t run_source(int fd[], const char *const cmd[]);
pid_t run_dest(int fd[], const char *const cmd[]);
STATUS run_pipeline(const char *const source[], const char *const dest[]);
void zombie();
void cleanup();

static const char *const CMD_LS[]   = { "ls", "-al", NULL };
static const char *const CMD_TR[]   = { "tr", "a-z", "A-Z", NULL };
static const char *const CMD_LSH[]  = { "ls", "-lh", NULL };
static const char *const CMD_GREP[] = { "grep", "pipe.cpp", NULL };

int main()
{
	int choice = 0;
	STATUS status = SUCCESS;

	atexit(cleanup);

	printf("\n\n\n --------- Select your choice -----------");
	printf("\n1. pipe within same process");
	printf("\n2. pipe between parent and child process");
	printf("\n3. pipe between a child and a grandchild, running cmds");
	printf("\n4. pipe between two child processes, running cmds");
	printf("\n5. pipe run cmd - 'ls -lh | grep pipe.cpp'");
	printf("\n6. zombie");
	printf("\n7. Exit");
	printf("\nEnter your choice : ");

	if ( scanf("%d", &choice) != 1 )
		choice = 0;

	switch(choice)
	{
		case 1: status = pipe_within_same_process(); break;
		case 2: status = pipe_between_parent_child_process(); break;
		case 3: status = pipe_between_processes(); break;
		case 4: status = pipe_between_two_child_processes(); break;
		case 5: status = pipe_run_cmd(); break;
		case 6: zombie(); break;
		case 7: printf("\nExiting"); break;
		default: printf("\nEntered an invalid choice !! Try again"); break;
	}

	return (status == SUCCESS) ? 0 : 1;
}


STATUS pipe_within_same_process()
{
	int fd[2];
	char data[] = "Hello ...., this is the simple example of pipe within the same process";
	char rcv[1024] = {0};
	ssize_t ret;

	if ( pipe(fd) == -1 )
	{
		perror("pipe");
		return FAIL;
	}

	if ( write(fd[1], data, strlen(data)) == -1 )
		perror("write");

	ret = read(fd[0], rcv, sizeof(rcv) - 1);   // Leave room for the NUL

	printf("\nRCVED %zd bytes: %s", ret, rcv);

	close(fd[0]);
	close(fd[1]);

	return SUCCESS;
}


STATUS pipe_between_parent_child_process()
{
	int fd[2];
	pid_t pid;
	char data[] = "Hello ...., this is the simple example of pipe_between_parent_child_process";

	if ( pipe(fd) == -1 )
	{
		perror("pipe");
		return FAIL;
	}

	fflush(stdout);   // So the child doesn't inherit a copy of unprinted output

	pid = fork();

	if ( pid == -1 )
	{
		perror("fork");
		close(fd[0]);
		close(fd[1]);
		return FAIL;
	}

	if ( pid == 0 )   // Child: reads
	{
		char rcv[1024] = {0};
		ssize_t ret;

		close(fd[1]);
		ret = read(fd[0], rcv, sizeof(rcv) - 1);
		printf("\nRCVED %zd bytes: %s\n", ret, rcv);
		fflush(stdout);   // _exit doesn't flush

		// Without this, the child would return from this function and carry on running the parent's code
		_exit(EXIT_SUCCESS);
	}

	// Parent: writes
	close(fd[0]);

	if ( write(fd[1], data, strlen(data)) == -1 )
		perror("write");

	close(fd[1]);
	waitpid(pid, NULL, 0);

	return SUCCESS;
}


static void exec_or_die(const char *const cmd[])
{
	// execvp takes char *const[] for historical reasons, but doesn't change the strings
	execvp(cmd[0], const_cast<char *const *>(cmd));
	perror(cmd[0]);       // Only reached if exec failed
	_exit(127);           // The shell's exit code for "command not found"
}

// First part of a pipeline: the command's stdout goes into the pipe
pid_t run_source(int fd[], const char *const cmd[])
{
	pid_t pid = fork();

	if ( pid == 0 )
	{
		dup2(fd[1], STDOUT_FILENO);   // The write end becomes stdout
		close(fd[0]);                 // The command must only hold the end it uses
		close(fd[1]);
		exec_or_die(cmd);
	}

	return pid;
}

// Second part of a pipeline: the command's stdin comes from the pipe
pid_t run_dest(int fd[], const char *const cmd[])
{
	pid_t pid = fork();

	if ( pid == 0 )
	{
		dup2(fd[0], STDIN_FILENO);    // The read end becomes stdin
		close(fd[0]);
		close(fd[1]);                 // Otherwise this command would never see end of file
		exec_or_die(cmd);
	}

	return pid;
}

// Runs "source | dest" the way a shell does, and returns when both have finished.
// The two must run at the same time: waiting for source first would deadlock
// whenever it writes more than the pipe holds, because nobody is reading yet.
STATUS run_pipeline(const char *const source[], const char *const dest[])
{
	int fd[2];
	pid_t sourcePid, destPid;

	if ( pipe(fd) == -1 )
	{
		perror("pipe");
		return FAIL;
	}

	fflush(stdout);

	sourcePid = run_source(fd, source);
	destPid = (sourcePid == -1) ? -1 : run_dest(fd, dest);

	// The parent must close both ends, or dest would never see end of file
	close(fd[0]);
	close(fd[1]);

	if ( sourcePid == -1 || destPid == -1 )
		perror("fork");

	if ( sourcePid != -1 )
		waitpid(sourcePid, NULL, 0);

	if ( destPid != -1 )
		waitpid(destPid, NULL, 0);

	return (sourcePid == -1 || destPid == -1) ? FAIL : SUCCESS;
}

// The parent forks a child, and the child forks a grandchild, with the pipe between those two.
// The child becomes the last command, so when the parent's wait returns, all the output has been written.
STATUS pipe_between_processes()
{
	int fd[2];
	int status = 0;
	pid_t pid;

	if ( pipe(fd) == -1 )
	{
		perror("pipe");
		return FAIL;
	}

	fflush(stdout);

	pid = fork();

	if ( pid == -1 )
	{
		perror("fork");
		close(fd[0]);
		close(fd[1]);
		return FAIL;
	}

	if ( pid == 0 )
	{
		if ( run_source(fd, CMD_LS) == -1 )
		{
			perror("fork");
			_exit(EXIT_FAILURE);
		}

		dup2(fd[0], STDIN_FILENO);
		close(fd[0]);
		close(fd[1]);
		exec_or_die(CMD_TR);
	}

	close(fd[0]);
	close(fd[1]);

	if ( waitpid(pid, &status, 0) != -1 )
		fprintf(stderr, "\nprocess %d exits with %d", pid, WEXITSTATUS(status));

	return SUCCESS;
}

STATUS pipe_between_two_child_processes()
{
	return run_pipeline(CMD_LS, CMD_TR);
}

STATUS pipe_run_cmd()
{
	return run_pipeline(CMD_LSH, CMD_GREP);
}


void zombie()
{
	pid_t pid;

	fflush(stdout);

	pid = fork();

	if ( pid == -1 )
		perror("fork");
	else if ( pid == 0 )
		_exit(EXIT_SUCCESS);     // The child ends here, and stays a zombie until the parent waits
	else
		sleep(120);              // The parent never calls wait: check with ps in another window
}


void cleanup()
{
	printf("\nHi buddy, all set for exit !!!!!!!!");
	printf("\nI'm exiting now\n");
}
