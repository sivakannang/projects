// g++ -g -rdynamic signal.cpp -o signal        ( -rdynamic gives function names in the backtrace )

/***************************** Signals *****************************************
 *
 * A signal is an asynchronous notification sent to a process, or to one thread in it.
 * The kernel interrupts the target's normal flow of execution to deliver it.
 * If the process has registered a handler, the handler runs. Otherwise the default action happens.
 *
*******************************************************************************/

/***************************** Standard Signals ********************************
 *
 * The C standard defines only 6 signals, in signal.h (csignal in C++)
 *
 * SIGABRT   - abnormal termination.
 * SIGFPE    - arithmetic error, such as integer division by zero.
 * SIGILL    - invalid instruction.
 * SIGINT    - interactive attention request sent to the program.
 * SIGSEGV   - invalid memory access ( segmentation fault).
 * SIGTERM   - termination request sent to the program.
 *
 * POSIX systems such as Linux define many more. List them with: kill -l
 *
*******************************************************************************/

/***************************** Default Behaviours ******************************
 *
 * Term         -> Terminate the process
 * Core         -> Terminate the process and write a core dump (if enabled), for a debugger
 * Ign          -> Nothing happens
 * Stop         -> Suspend the process, keeping it in memory
 * Cont         -> Resume a stopped process
 *
 * 	Name		Event						Default
 *
 * 	SIGHUP		Terminal disconnected / controlling process ended	Term
 * 	SIGINT		Interrupt (ctrl-C)				Term
 * 	SIGQUIT		Quit (ctrl-\)					Core
 * 	SIGABRT		abort() called					Core
 * 	SIGILL		Invalid instruction				Core
 * 	SIGFPE		Arithmetic error				Core
 * 	SIGSEGV		Invalid memory access				Core
 * 	SIGKILL		Kill. Can't be caught, blocked or ignored	Term
 * 	SIGPIPE		Write to a pipe with no readers			Term
 * 	SIGALRM		alarm() timer expired				Term
 * 	SIGTERM		Termination request (default of 'kill pid')	Term
 * 	SIGCHLD		A child process stopped or ended		Ign
 * 	SIGCONT		Resume a stopped process			Cont
 * 	SIGTSTP		Stop from the terminal (ctrl-Z)			Stop
 * 	SIGSTOP		Stop. Can't be caught, blocked or ignored	Stop
 *
*******************************************************************************/

/***************************** Signal Handling  ********************************
 *
 * sigaction() -> sets the action for a signal. Prefer it to signal(), whose behaviour
 *                varies between systems and which can't set flags such as SA_RESTART.
 * raise()     -> sends a signal to the calling thread
 * kill()      -> sends a signal to a process or process group
 *
 * typedef void (*sighandler_t)(int)
 *
 * int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact)
 * sighandler_t signal(int signum, sighandler_t handler)     // Returns the old handler, or SIG_ERR
 * int raise(int sig)
 * int kill(pid_t pid, int sig)
 * int pthread_kill(pthread_t thread, int sig)
 *
 * return   -> sigaction() = raise() = kill() = pthread_kill() = 0 on success
 * header   -> signal.h
 *
 * A handler that returns from SIGSEGV, SIGFPE or SIGILL caused by the code itself goes back
 * to the same faulting instruction, which faults again, forever. It must end the process.
 *
*******************************************************************************/

/***************************** Signal Safe functions ***************************
 *
 * A handler can interrupt the program in the middle of anything, including inside malloc or
 * printf while they hold a lock. So a handler may only call async-signal-safe functions.
 *
 * Full list: man 7 signal-safety
 *
 * Safe     -> write, read, open, close, _exit, kill, raise, alarm, sigaction, waitpid, wait, sem_post
 * Not safe -> printf, sprintf, malloc, free, exit, and most of stdio and pthread
 *
 * The simplest safe design is for the handler to only set a volatile sig_atomic_t flag,
 * and for the main loop to check it and do the real work.
 *
 * A handler should also save and restore errno, if it calls anything that can change it.
 *
*******************************************************************************/


#include <errno.h>
#include <execinfo.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

void buggy_function(char *p);
void simpleFork();
void printTrace();
void sigHandler(int sig);

// write() is safe in a handler, but printf isn't, so these format text by hand
static void safePrint(const char *text)
{
	ssize_t ignored = write(STDOUT_FILENO, text, strlen(text));
	(void)ignored;
}

static void safePrintInt(int value)
{
	char digits[16];
	int i = sizeof(digits);

	digits[--i] = '\0';

	do {
		digits[--i] = (char)('0' + value % 10);
		value /= 10;
	} while ( value > 0 );

	safePrint(&digits[i]);
}

static void received(const char *name, int sig)
{
	safePrint("\nReceived signal ");
	safePrint(name);
	safePrint(" ");
	safePrintInt(sig);
}

static void install(int sig)
{
	struct sigaction action;

	memset(&action, 0, sizeof(action));
	action.sa_handler = sigHandler;
	sigemptyset(&action.sa_mask);
	action.sa_flags = SA_RESTART;      // Blocked calls such as the menu's read carry on after the handler

	if ( sigaction(sig, &action, NULL) == -1 )
		printf("\nsigaction(%d) failed : %s", sig, strerror(errno));
}

// Returns the number typed. Exits at end of input, and returns -1 for anything that isn't a number.
static int readChoice()
{
	int choice, c;
	int ret = scanf("%d", &choice);

	if ( ret == EOF )
		exit(0);

	if ( ret == 1 )
		return choice;

	while ( (c = getchar()) != '\n' && c != EOF );

	return -1;
}


int main()
{
	void *preload[1];

	// The first backtrace() call may allocate memory, which isn't safe in a handler, so do it now
	backtrace(preload, 1);

	while ( 1 )
	{
		printf("\n\n\n ---------- Signal Test --------------");
		printf("\n%02d. SIGABRT                      -> Abort", SIGABRT);
		printf("\n%02d. SIGFPE                       -> Arithmetic error", SIGFPE);
		printf("\n%02d. SIGILL                       -> Invalid instruction", SIGILL);
		printf("\n%02d. SIGSEGV                      -> Access violation / Segmentation fault", SIGSEGV);
		printf("\n%02d. SIGTERM                      -> Terminate -> 'kill pid'", SIGTERM);
		printf("\n%02d. SIGINT                       -> Interrupt -> CTRL+C", SIGINT);
		printf("\n%02d. SIGQUIT                      -> Quit      -> CTRL+\\", SIGQUIT);
		printf("\n%02d. SIGTSTP                      -> Stop      -> CTRL+Z", SIGTSTP);
		printf("\n%02d. SIGSTOP                      -> Shows that it can't be caught", SIGSTOP);
		printf("\n%02d. SIGCONT                      -> Resume    -> kill -SIGCONT pid", SIGCONT);
		printf("\n%02d. SIGCHLD                      -> Child ended, so the parent doesn't have to block in wait()", SIGCHLD);
		printf("\n%02d. SIGALRM                      -> Alarm timer", SIGALRM);
		printf("\n99. Exit");
		printf("\nEnter choice : ");

		int choice = readChoice();

		switch(choice)
		{
			case SIGABRT:
				install(SIGABRT);
				abort();   // The process still ends, even though the handler returns
				break;

			case SIGFPE:
			{
				volatile int zero = 0;   // volatile, so the compiler can't remove the division

				install(SIGFPE);
				printf("\nDividing by zero, result %d", 5 / zero);

				// Still running: x86 traps on this, but ARM just returns 0
				printf("\nThis CPU didn't trap, so raising SIGFPE directly");
				fflush(stdout);
				raise(SIGFPE);
				break;
			}

			case SIGILL:
				install(SIGILL);
				fflush(stdout);
				raise(SIGILL);
				break;

			case SIGSEGV:
				install(SIGSEGV);
				printf("\nsignal SIGSEGV initialized");
				fflush(stdout);
				buggy_function(NULL);
				break;

			case SIGTERM:
				install(SIGTERM);
				printf("\nsignal SIGTERM initialized. The process now survives 'kill pid'. Use 'kill -SIGKILL pid' to force it.");
				fflush(stdout);
				kill(getpid(), SIGTERM);
				break;

			case SIGINT:
				install(SIGINT);
				printf("\nsignal SIGINT initialized, press CTRL+C to raise interrupt signal");
				break;

			case SIGALRM:
				install(SIGALRM);
				alarm(15);
				printf("\nsignal SIGALRM initialized, timer set to 15 seconds, repeating");
				break;

			case SIGQUIT:
				install(SIGQUIT);
				printf("\nsignal SIGQUIT initialized, press CTRL+\\ to raise quit signal");
				break;

			case SIGTSTP:
				install(SIGTSTP);
				printf("\nsignal SIGTSTP initialized, press CTRL+Z. With this handler the process won't stop.");
				break;

			case SIGSTOP:
				install(SIGSTOP);   // Prints the error: SIGSTOP and SIGKILL can't be caught
				break;

			case SIGCONT:
				install(SIGCONT);
				printf("\nsignal SIGCONT initialized, use 'kill -SIGCONT pid' to raise it");
				break;

			case SIGCHLD:
				install(SIGCHLD);
				printf("\nsignal SIGCHLD initialized, forking a child. The handler collects it, to avoid a zombie.");
				simpleFork();
				break;

			case 99:
				exit(0);

			default:
				printf("\nInvalid choice , try again !!!");
				break;
		}
	}
}


void sigHandler(int sig)
{
	int savedErrno = errno;     // Otherwise the interrupted code could see a changed errno

	switch(sig)
	{
		case SIGABRT:
			received("SIGABRT", sig);
			break;

		// Returning would run the faulting instruction again, forever, so end the process.
		// _exit is safe here, and exit isn't.
		case SIGFPE:
			received("SIGFPE", sig);
			_exit(128 + sig);

		case SIGILL:
			received("SIGILL", sig);
			_exit(128 + sig);

		case SIGSEGV:
			received("SIGSEGV", sig);
			printTrace();
			_exit(128 + sig);

		case SIGTERM:
			received("SIGTERM", sig);
			break;

		case SIGINT:
			received("SIGINT", sig);
			break;

		case SIGALRM:
			received("SIGALRM", sig);
			alarm(15);     // setting alarm again
			break;

		case SIGQUIT:
			received("SIGQUIT", sig);
			break;

		case SIGTSTP:
			received("SIGTSTP", sig);
			break;

		case SIGCONT:
			received("SIGCONT", sig);
			break;

		case SIGCHLD:
		{
			pid_t pid;

			received("SIGCHLD", sig);

			// Several children can end while one SIGCHLD is pending, so collect them all
			while ( (pid = waitpid(-1, NULL, WNOHANG)) > 0 )
			{
				safePrint(", child pid is ");
				safePrintInt(pid);
			}
			break;
		}
	}

	errno = savedErrno;
}


void simpleFork()
{
	fflush(stdout);   // So the child doesn't inherit a copy of unprinted output

	switch( fork() )
	{
		case -1:
			perror("fork");
			break;

		case 0:
			safePrint("\nChild process");
			_exit(0);   // Without this, the child would carry on and run the parent's menu too

		default:
			printf("\nParent process");
			break;
	}
}


void buggy_function(char *p)
{
	*(volatile char *)p = 'x';
}


void printTrace()
{
	void *trace[32];
	int size = backtrace(trace, 32);

	safePrint("\n-----------Backtrace----------\n");

	// backtrace_symbols calls malloc, but this version writes straight to the descriptor
	backtrace_symbols_fd(trace, size, STDOUT_FILENO);
}
