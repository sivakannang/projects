/******************************** Program to practise gdb on ********************************
 *
 * g++ -g gdb_practice.cpp -pthread -o app
 * gdb app
 *
 * Notes : gcc flags and gdb commands are in cmds/gcc_gdb.txt
 *
 * Try these at the (gdb) prompt :
 *
 * b fact if n==2 , r , bt , fin                         - Conditional stop in a recursion, then see the return value
 * b printArgs , r a b , n , watch i if i == 5 , c       - Stop when a loop variable reaches 5
 * b display , r , i threads , thread apply all bt       - See all the threads, the main thread is still creating the others
 * r crash , bt , f 1 , i locals                         - Crash on purpose, then find who passed the null pointer
 *
 *************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

void printArgs(int argc, char **argv);
int fact(int num);

void thread_test();
void *display(void *arg);
void copyTo(char *dest);

char name[100] = "sivakannan";

int main(int argc, char *argv[]) {

	printf("Entered main(), name = %s\n", name);

	printArgs(argc, argv);

	int result = fact(5);
	printf("fact(5) = %d\n", result);

	thread_test();

	if ( argc > 1 && strcmp(argv[1], "crash") == 0 ) {
		char *dest = NULL;
		copyTo(dest);        // Crashes on purpose, for practising with bt
	}

	printf("Exiting main()\n");

	return 0;

}

int fact( int n) {
	if ( n == 0 )
		return 1;
	return n * fact(n-1);
}

void printArgs(int argc, char **argv) {

	int i = 0;
	for ( i = 0; i < argc; i++)
		printf("argv[%d] = %s\n", i, argv[i]);

	for ( i = 0; i < 10; i++)
		printf("i = %d\n", i);

	printf("fn exit - i = %d\n" , i);
}

void copyTo(char *dest) {
	char arr[100] = {0};
	memcpy(dest, arr, sizeof(arr));
}

void *display(void *arg)
{
	int i = *(int *)arg;
	do {
		printf("I'm from Thread %d\n", i);
		sleep(1);
	} while ( --i > 0);

	return NULL;

}

void thread_test() {

	pthread_t tid[10] = {0};
	int ids[10];         // One id per thread. A pointer to the loop variable would be read by a thread while the loop changes it.

	int i = 0;
	for ( i = 0; i < 10; i++ ) {
		ids[i] = i;
		int ret = pthread_create(&tid[i], NULL, display, &ids[i]);
		printf("Thread %d creation %s\n", i, (ret == 0) ? "Success" : "Fail");
	}

	for ( i = 0; i < 10; i++ ) {
		int ret = pthread_join(tid[i], NULL);
		printf("Thread %d join %s\n", i, (ret == 0) ? "Success" : "Fail");
	}

}
