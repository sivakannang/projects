/***************************** Signals *****************************************
 *
 * What is threads?
 * Why thread is required ?
 * Drawbacks of thread ?
 * process vs thread ?
 * fork vs thread ?
 * order of thread execution ?
 *
*******************************************************************************/

/***************************** API Definition **********************************
 *
 * pthread_create()           -> create a new thread
 * pthread_cancel()           -> send a cancellation request to a thread ( 1.pthread_cleanup_push(), 2.pthread_key_create(), 3.pthread_exit() )
 * pthread_setcancelstate()   -> Enbale / Disable thread cancel state
 *                            -> By default thread cancel state enabled ( PTHREAD_CANCEL_DISABLE, PTHREAD_CANCEL_ENABLE)
 * pthread_exit()             -> terminates the calling thread
 *                            -> returns a value via it's argument retval that (if the thread is joinable) in the same process that calls pthread_join
 * pthread_join()             -> suspends execution of calling thread until target threads completes execution
 * pthread_detach()           -> detach a thread
 *                            -> Every thread gets allocated thread resources when it get created.
 *                            -> By default the all the threads are joinable and thread_join() will reclaim the thread resources.
 *                            -> In some situations we may don't want to call thread_join().
 *                            -> In those cases we should use thread_detach() for inform that reclaim the thread resources at end of the thread task.
 * pthread_self()             -> obtain ID of the calling thread
 * pthread_equal()            -> compare thread IDs
 * pthread_kill()             -> send a signal to a thread, it doesn't kill the thread, its just send signal to the specified thread
 * pthread_attr_init()        ->
 * pthread_attr_destroy()     ->
 *
 * pthread_mutex_init()       ->
 * pthread_mutex_lock()       ->
 * pthread_mutex_trylock()    ->
 * pthread_mutex_unlock()     ->
 * pthread_mutex_destroy()    ->
 * pthread_mutexattr_init()   ->
 * pthread_mutexattr_destroy()->
 *
 * pthread_rwlock_init()      ->
 * pthread_rwlock_rdlock()    ->
 * pthread_rwlock_tryrdlock() ->
 * pthread_rwlock_wrlock()    ->
 * pthread_rwlock_trywrlock() ->
 * pthread_rwlock_unlock()    ->
 * pthread_rwlock_destroy()   ->
 * pthread_rwlockattr_init()  ->
 * pthread_rwlockattr_destroy()->
 *
 *
 * int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg)
 * void pthread_exit(void *retval)
 * int pthread_cancel(pthread_t thread)
 * int pthread_join(pthread_t thread, void **retval)
 * int pthread_detach(pthread_t thread)
 * pthread_t pthread_self(void)
 * int pthread_equal(pthread_t t1, pthread_t t2)
 * int pthread_kill(pthread_t thread, int sig)
 * int pthread_attr_init(pthread_attr_t *attr)
 * int pthread_attr_destroy(pthread_attr_t *attr)
 *
 * int pthread_mutex_init(pthread_mutex_t *restrict mutex, const pthread_mutexattr_t *restrict attr)
 * int pthread_mutex_lock(pthread_mutex_t *mutex)
 * int pthread_mutex_trylock(pthread_mutex_t *mutex)
 * int pthread_mutex_unlock(pthread_mutex_t *mutex)
 * int pthread_mutex_destroy(pthread_mutex_t *mutex)
 * int pthread_mutexattr_init(pthread_mutexattr_t *attr)
 * int pthread_mutexattr_destroy(pthread_mutexattr_t *attr)
 *
 * int pthread_rwlock_init(pthread_rwlock_t *rwlock, const pthread_rwlockattr_t *attr)
 * int pthread_rwlock_rdlock(pthread_rwlock_t *rwlock)
 * int pthread_rwlock_tryrdlock(pthread_rwlock_t *rwlock)
 * int pthread_rwlock_wrlock(pthread_rwlock_t *rwlock)
 * int pthread_rwlock_trywrlock(pthread_rwlock_t *rwlock)
 * int pthread_rwlock_unlock(pthread_rwlock_t *rwlock)
 * int pthread_rwlock_destroy(pthread_rwlock_t *rwlock)
 * int pthread_rwlockattr_init(pthread_rwlockattr_t *attr)
 * int pthread_rwlockattr_destroy(pthread_rwlockattr_t *attr)
 *
 *
 * HANDLE WINAPI CreateThread(
 *   _In_opt_  LPSECURITY_ATTRIBUTES  lpThreadAttributes,
 *   _In_      SIZE_T                 dwStackSize,
 *   _In_      LPTHREAD_START_ROUTINE lpStartAddress,
 *   _In_opt_  LPVOID                 lpParameter,
 *   _In_      DWORD                  dwCreationFlags,
 *   _Out_opt_ LPDWORD                lpThreadId)
 *
 *   DWORD WINAPI WaitForSingleObject(
 *   _In_ 	HANDLE hHandle,
 *   _In_ 	DWORD  dwMilliseconds)
 *
 *   DWORD WINAPI WaitForMultipleObjects(
 *   _In_       DWORD  nCount,
 *   _In_       const HANDLE *lpHandles,
 *   _In_       BOOL   bWaitAll,
 *   _In_       DWORD  dwMilliseconds)
 *
 *
 * HANDLE WINAPI CreateMutex(
 * _In_opt_ LPSECURITY_ATTRIBUTES lpMutexAttributes,
 * _In_     BOOL                  bInitialOwner,
 * _In_opt_ LPCTSTR               lpName)
 *
 * BOOL WINAPI ReleaseMutex( _In_ HANDLE hMutex)
 *
 *
*******************************************************************************/



/*
 * http://www.thegeekstuff.com/2012/04/terminate-c-thread/
 * fork() and thread() differences
 * processes and threads difference
 * synchronization, race condition, mutually exclusive, deadlocks
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <stdarg.h>

#ifdef _WIN32
	#include <windows.h>
#else
	#include <unistd.h>
	#include <pthread.h>
#endif


typedef enum
{
	TEST_THREAD_CREATE = 1,
	TEST_THREAD_WAIT,
	TEST_THREAD_EXIT,
	TEST_THREAD_CANCEL
}TEST_TYPE;

int thread_create();
int thread_wait();
int thread_exit();
int thread_cancel();

void *doSomething_1(void *arg);
void *doSomething_2(void *arg);
void *doSomething_3(void *arg);

void DBG_PRINT(const char* format, ...);


pthread_t tid[2];
int ret1, ret2;

int main()
{
	int choice = 0;

	printf("\n ----------- Thread Test ------------");
	printf("\n%d. Thread create", TEST_THREAD_CREATE);
	printf("\n%d. Thread wait", TEST_THREAD_WAIT);
	printf("\n%d. Thread exit", TEST_THREAD_EXIT);
	printf("\n%d. Thread cancel", TEST_THREAD_CANCEL);
	printf("\nEnter choice : ");
	scanf("%d", &choice);
	switch(choice)
	{
		case TEST_THREAD_CREATE:
			thread_create();
			break;
		case TEST_THREAD_WAIT:
			thread_wait();
			break;
		case TEST_THREAD_EXIT:
			thread_exit();
			break;
		case TEST_THREAD_CANCEL:
			thread_cancel();
			break;
		default:
			printf("\nInvalid choice !!!");
			break;

	}
	DBG_PRINT("Main thread exits here");


	return 0;
}

int thread_create()
{
	int i=0, ret = 0;

	while ( i < 2 )
	{
		ret = pthread_create(&tid[i], NULL, &doSomething_1, NULL);
		if ( ret == 0)
			DBG_PRINT("Thread %d created successfully", i+1);
		else
			DBG_PRINT("Thread %d creation failed : %s", i+1, strerror(errno));
		i++;
	}

	return 0;

}

int thread_wait()
{
	int i=0, ret = 0;

	while ( i < 2 )
	{
		ret = pthread_create(&tid[i], NULL, &doSomething_1, NULL);
		if ( ret == 0)
			DBG_PRINT("Thread %d created successfully", i+1);
		else
			DBG_PRINT("Thread %d creation failed : %s", i+1, strerror(errno));
		i++;
	}

	i = 0;
	while( i < 2)
	{
		ret = pthread_join(tid[i], NULL);
		DBG_PRINT("Thread %d join %s", i+1, ret ? "FAIL" : "SUCCESS");
		i++;
	}

	return 0;

}

int thread_exit()
{
	int i=0, ret = 0;
	int *ptr[2] = {0};

	while ( i < 2 )
	{
		ret = pthread_create(&tid[i], NULL, &doSomething_2, NULL);
		if ( ret == 0)
			DBG_PRINT("Thread %d created successfully", i+1);
		else
			DBG_PRINT("Thread %d creation failed : %s", i+1, strerror(errno));
		i++;
	}

	i = 0;
	while( i < 2)
	{
		ret = pthread_join(tid[i], (void **)&ptr[i]);
		DBG_PRINT("Thread %d join %s, returned %d", i+1, ret ? "FAIL" : "SUCCESS", *ptr[i]);
		i++;
	}

	return 0;

}

int thread_cancel()
{
	int ret  = 0;
	void *res = NULL;
	pthread_t tid;
		
	ret = pthread_create(&tid, NULL, &doSomething_3, NULL);
	if ( ret == 0)
		DBG_PRINT("Thread created successfully");
	else
		DBG_PRINT("Thread creation failed : %s", strerror(errno));

	sleep(2);   // Give thread a chance to get started

	DBG_PRINT("Thread sending cancellation request");
	ret = pthread_cancel(tid);
	if ( ret != 0 )
		DBG_PRINT("\nThread cancel request failed");

	ret = pthread_join(tid, &res);
	DBG_PRINT("Thread join %s", ret ? "FAIL" : "SUCCESS");
	
	if ( res == PTHREAD_CANCELED)
		DBG_PRINT("Thread status cancelled");
	else
		DBG_PRINT("Oops !!!!! Thread was not cancelled and we shouldn't be here");

}

void *doSomething_3(void *arg)
{
	int ret = 0;

	// Disable cancellation for a while, so that we don't immediately react to a cancellation request
	ret = pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
	if ( ret != 0)
		DBG_PRINT("Thread set cancel state to disable failed");
	DBG_PRINT("Thread cancel disabled");
	sleep(5);

	// Enable cancellation
	DBG_PRINT("Thread cancel enabling"); 
	ret = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	if ( ret != 0)
		DBG_PRINT("Thread set cancel state to enable failed");

	DBG_PRINT("Thread cancel enabled"); 

	sleep(20);
	DBG_PRINT("Oops !!!!! we shouldn't be here");
}


void *doSomething_1(void *arg)
{
	unsigned long i = 0;
	pthread_t id = pthread_self();

	if ( pthread_equal(id, tid[0]) )
		DBG_PRINT("Thread 1 processing");
	else
		DBG_PRINT("Thread 2 processing");

	for ( i = 0; i < 0xFFFFFFFF; i++);

	if ( pthread_equal(id, tid[0]) )
		DBG_PRINT("Thread 1 ends");
	else
		DBG_PRINT("Thread 2 ends");

	return NULL;
}

void *doSomething_2(void *arg)
{
	unsigned long i = 0;
	pthread_t id = pthread_self();

	if ( pthread_equal(id, tid[0]) )
		DBG_PRINT("Thread 1 processing");
	else
		DBG_PRINT("Thread 2 processing");

	for ( i = 0; i < 0xFFFFFFFF; i++);

	if ( pthread_equal(id, tid[0]) )
	{
		DBG_PRINT("Thread 1 ends");
		ret1 = 100;
		pthread_exit((void *)&ret1);
	}
	else
	{
		DBG_PRINT("Thread 2 ends");
		ret2 = 200;
		pthread_exit((void *)&ret2);
	}

	return NULL;
}


int Time_Get(char *outField)
{

	time_t t = time(0);
	struct tm* lt = localtime(&t);
 	char time_str[40] = {0};
	struct timeval now = {0};
	long long milliSeconds = 0;
	gettimeofday(&now, NULL);
	//struct timespec spec;
	//clock_gettime(CLOCK_REALTIME, &spec);
	//milliSeconds = round(spec.tv_nsec / 1.0e6); // Convert nanoseconds to milliseconds

	milliSeconds = now.tv_usec/1000;
	
	sprintf(time_str, "\n%02d/%02d/%02d %02d%02d%02d:%04ld => ", lt->tm_mon + 1, lt->tm_mday, lt->tm_year - 100, lt->tm_hour, lt->tm_min, lt->tm_sec, milliSeconds);

	strncat(outField, time_str, strlen(time_str));
	return lt->tm_mon + 1;
}

void DBG_PRINT(const char* format, ...)
{
	char buffer[1024] = {'\0'};

	va_list args;

	Time_Get(buffer);

	va_start(args, format);
	vsprintf (&buffer[strlen(buffer)], format, args );
	va_end(args);	

	printf("%s", buffer);

}


