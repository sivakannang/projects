/********************************************************* GCC *******************************************************************
 *
 * gcc arguments:
 *
 * 	-E                    : stop after preprocesser output, do not invoke assembler
 * 	-S                    : stop after assembler output. do not invoke compiler
 * 	-c                    : stop after compiler output, do not invoke linker
 * 	-svae-temps           : keep all intermediate code ( .c/.cpp -> .i/.ii -> .s -> .o/.obj -> .exe/.out)
 * 	-o [filename]         : generate output file with the given name ( ex : gcc main.c -o main )
 * 	-Wall                 : show all warnings
 * 	-Werror               : convert/treat all the warnings as error
 * 	-v                    : Print (on standard error output) the commands executed to run the stages of compilation
 * 	-g                    : generates debug information to be used by GDB debugger
 * 	-ansi                 : compile in ansi or ISOC 89 mode, modern C/C++ style/features will be suppressed
 * 	-pie                  : create a position independent code
 * 	-shared               : create a shared library
 * 	--std=[]              : Determine the language standard, possible values [C89|C90|C99|C9x|C11|C1x... C++0x,C++11,C++14,C++1x..]
 * 	-D[macro]             : Use compile time macros using -D option ( ex : gcc -DWIN32 main.c)
 * 	-U[macro]             : Cancel any previous definition of name, either built in or provided with a -D option.
 * 	-@[filename]          : Read command-line options from file ( gcc main.c @opt_file , opt_file content is '-Wall -o main')
 * 	-O[1|2|3|g|s|...]     : Optimization level. optimize the code for size or time or debug experience and performence
 * 	-f[unsigned-char]     : interpret char as unsigned char in the source file
 * 	-f[signed-char]       : treat all the char as signed char in the source file
 * 	-f[PIC]               : produce Position Independent Code for shared libraries, which allows shared lib to load in any address instead of specific address
 * 	-I[path]              : include header files path
 * 	-L[path]              : include library path
 * 	-l[libname]           : link with shared library ( ex : -lpthread )
 * 	-Wl[options]          : pass options to the linker with comma seperated values
 * 				(ex: -Wl,-rpath,/home/siva/lib -> inform to linker look for the dependency libraries in the specified runpath)
 *
 *
 * GNU binary utilities:
 *
 * 	- gcc                 : GNU C compiler
 * 	- g++                 : GNU C++ compiler
 * 	- cpp                 : C Preprocessor
 * 	- as                  : GNU Assembler
 * 	- ld                  : GNU Linker
 * 	- ldd                 : List library dependencies
 * 	- ldconfig            : configure dynamic linker run time bindings (update cache /etc/ld.so.cache)
 * 	- ar                  : GNU Archive - use for static library management ( build, list, ...)
 * 	- nm                  : List object file symbols
 * 	- objcopy             : copy and translate object files
 * 	- objdump             : display object file information
 * 	- size                : List text, data, bss segment size
 * 	- strings             : Display printable characters from a file
 * 	- strip               : Discard symbols from object file
 * 	- c++filt             : Demangle command
 * 	- addr2line           : Convert address into filename and numbers
 * 	- readelf             : Display ELF object file info
 *
 * 	ex :
 * 	$ cpp main.c -o main.i
 * 	$ gcc -S main.i -o main.s
 * 	$ as main.s -o main.o
 * 	$ ld -o main main.o ...libraries..
 *
 *
 *
 * ***************************************************** gdb **********************************************************************
 *
 * $ g++ -g app.c -o app    - compile with 'g' option which enables more debug information in the out file.
 *
 * $ gdb app                - start gdb
 *
 * $ h[elp]                 - list available cmd class
 * $ help/h class           - 'l breakpoints'  , list detail about a breakpoints cmd class
 *
 * $ r[un] [args]           - 'r'              , run 
 * 			    - 'r arg0 arg1'    , run with arguments
 *
 * $ l[ist] []              - 'l'              , list 10 lines
 *                          - 'l 20'
 *                          - 'l 20,50'        , list between line 20 to 50
 *                          - 'l main.cpp:20'  , list around line 20 in the file main.cpp
 *                          - 'l main'         , list around the function main
 *                          - 'l main.cpp:main', list around the function main in file main.cpp
 *
 * $ p[rint]  [var]         - Print value of var called
 * $ p * [var]              - Print what is pointed to by var
 * $ p/x var                - Print value of var in hex format
 * $ p [var] <tab>          - List all variables starting with <var>
 * $ p $eax                 - print current value of register
 *
 * $ set [var=val]          - set or overwrite a value
 *
 * $ backtrace or bt [+-N]  - produces a stack trace of the function calls that lead to a seg fault 
 *
 * $ where                  - show detail, where the application crashed
 *
 * $ kill                   - Kill the program being debugged
 *
 * $ <Enter>                - Execute the last cmd
 *
 *
 *
 * breakpoints:             - pause a program on a specific location
 * 
 * $ b[reak]		    - 'b'              , set breakpoint at current line
 * 			    - 'b 20'           , set breakpoint at line number 20
 *                          - 'b +N'           , puts a breakpoint N lines down from the current line
 * 			    - 'b main.c:20'    , set breakpoint at line number 20, in file main.c
 * 			    - 'b main'         , set breakpoint at function main
 * 			    - 'b main.c:main'  , set breakpoint at function main, in file main.c
 *
 * $ i[nfo] b               - list the break points
 * $ i[nfo] r[egisters]     - list or display the value in all registers.
 * $ i[nfo] watch           - list watch pointers
 * $ i[nfo] args            - list arguments
 * $ i[nfo] local           - list local variables
 * $ i[nfo] frame           - list frame info
 * $ i[nfo] catch           - list of all the exception handlers that are active in the current stack frame at the current point of execution
 *
 * $ clear [file|line|fn]   - clear a break point
 *
 * $ d[elete] [bp number]   - delete a breakpoint by it's number, if the number not specified delete all breakpoint's
 *
 * $ r[un]                  - run to the next breakpoint or end
 *
 * $ s[tep]                 - Single step, if it is function dive into the function and wait
 *                          - 's N' - Runs the next N lines of the program
 *
 * $ n[ext]                 - Single instruction, if it is the function execute the whole function and wait in the next line
 *
 * $ c[ontinue]             - Continue to the next breakpoint or end
 *
 * $ f[inish]               - Finish current function, loop, etc
 *
 * $ u[ntil]                - until is like next, except that if you are at the end of a loop, until will continue execution until the loop is
 *                            exited, whereas next will just take you back up to the beginning of the loop
 *                          - 'u N' - Runs until you get N lines in front of the current line
 *
 *
 *
 * watchpoints :            - watch a variable, when ever the variable value get changed pause the execution there
 *
 * $ watch [var]            - watch the variable 'var'
 *
 * $ watch [ var == NULL ] - conditional watchpoint
 *
 * $ f[rame] [N|addr]      - Selects a stack frame or displays the currently selected stack frame.
 *
 * $ up [N]                - Move n frames up the stack
 *
 * $ down [N]              - Move n frames down the stack
 *
 * up|down[-silently] N    - They do their work silently, without causing display of the new frame.
 *
 *
 * $ help thread
 * $ info threads          - list the threads
 * $ thread [thread_no]    - command to switch among threads
 * $ thread apply [thread-id-list | all [-ascending]] command   - a command to apply a command to a list of threads
 *
 *
 *
 *
 *
 *
 *
 *
 **********************************************************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

void printArgs(int argc, char **argv);
int fact(int num);

void thread_test();
void *display(void *arg);

char name[100] = "sivakannan";

int main(int argc, char *argv[]) {
	
	printf("\nEntered main()");

	printArgs(argc, argv);

	int result = fact(5);

	thread_test();


	char arr[100] = {0};
	memcpy(NULL, arr, sizeof(arr));
	printf("\narr : %s", arr);


	printf("\nExiting main()");

	return 0;

} 

int fact( int n) {
	if ( n == 0 )
		return 1;
	return n * fact(n-1);
}

void printArgs(int argc, char **argv) {

	static int counter;

	while ( *argv )
		printf("\n argv = %s", *argv++);

	int i = 0;
	for ( i = 0; i < 10; i++)
		printf("\ni = %d", i);

	printf("\n fn exit - i = %d" , i);
}


void *display(void *arg)
{
	int i = *(int *)arg;
	do {
		printf("\n I'm from Thread %d", i);
		sleep(1);
	} while ( --i > 0);

	return NULL;
	
}

void thread_test() {

	pthread_t tid[10] = {0};
	
	int i = 0;
	for ( i = 0; i < 10; i++ ) {
		int ret = pthread_create(&tid[i], NULL, display, (void *)&i);
		printf("\n Thread %d creation %s", i, (ret == 0) ? "Success" : "Fail");
	}

	for ( i = 0; i < 10; i++ ) {
		int ret = pthread_join(tid[i], NULL);
		printf("\n Thread %d join %s", i, (ret == 0) ? "Success" : "Fail");
	}

}
