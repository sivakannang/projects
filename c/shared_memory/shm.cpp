
/***************************** Shared Memory ***********************************
 *
 * Shared memory is the fastest form of IPC available, because one copy memory is available to all the threads or processes that share the memory
 * One program will create a memory portion which other processes (if permitted) can access.
 *
 * shmget()      -> allocates a shared memory segment
 * shmctl()      -> shared memory control
 * shmat()       -> attach to a shared memory segment identified by shmid
 * shmdt()       -> detach to a shared memory segment identified by shmid
 * ftok()        -> use to get an unique key/number across the system
 *
 * msg header    -> sys/shm.h
 * ipc header    -> sys/ipc.h
 * return        -> shmget = +ve ( a valid shm id),  shmctl +ve, shmat = valid address ( (void *)-1 for fail), shmdt = +ve
 *
*******************************************************************************/

/*****************************  IPC system cmds ********************************
 *
 * #man ipcs
 * #man ipcrm
 * #ipcs -m                 => Display information about shared memory
 * #ipcrm -m shmid          => Removes the shared memory set associated with ID shmid from the system
 * #ipcrm -M shmkey         => Remove the shared memory set associated with key shmkey from the system
 *
*******************************************************************************/

/***************************** API defenitions *********************************
 *
 * int shmget(key_t key, size_t size, int mode)
 * int shmctl(int shmid, int cmd, struct shmid_ds *buf)
 * void *shmat(int shmid, const void *shmaddr, int shmflg)
 * int shmdt(const void *shmaddr)
 * key_t ftok(const char *path, int id)
 *
 * path      -> any valid file path
 * id        -> could be any character
 * key       -> unique key ( can be generate using ftok())
 * size      -> memory size that we request 
 * mode      -> permission | mode ( IPC_PRIVATE, IPC_CREAT, IPC_EXCL )
 *               permission : should be in octal, pass always 0666
 *               mode       : IPC_CREAT   - create new shared memory segment
 *                            IPC_PRIVATE - create new shared memory segment but private to the creating process
 *                            0           - attach with an existing shared memory segment
 * shmid     -> shared memory id returned by shmget()
 * shmaddr   -> pointer of the shared memory segment
 * shmflg    -> 0 - Read and Write access, SHM_RDONLY - for read only access
 * cmd       -> IPC_STAT  - Copy information from the kernel data structure associated with shmid into the shmid_ds
 *              IPC_SET   - Write the values of some members of the shmid_ds structure
 *              IPC_RMID  - Immediately release the shared memory segment idenfied by the shmid
 

 struct shmid_ds {
 	struct ipc_perm shm_perm;    // Ownership and permissions
 	size_t          shm_segsz;   // Size of segment (bytes)
 	time_t          shm_atime;   // Last attach time
 	time_t          shm_dtime;   // Last detach time
 	time_t          shm_ctime;   // Last change time
 	pid_t           shm_cpid;    // PID of creator
 	pid_t           shm_lpid;    // PID of last shmat(2)/shmdt(2)
 	shmatt_t        shm_nattch;  // No. of current attaches
 ...
 };

 The ipc_perm structure is defined as follows (the highlighted fields are settable using IPC_SET):

 struct ipc_perm {
 	key_t          __key;    // Key supplied to shmget(2)
 	uid_t          uid;      // Effective UID of owner
 	gid_t          gid;      // Effective GID of owner
 	uid_t          cuid;     // Effective UID of creator
 	gid_t          cgid;     // Effective GID of creator
 	unsigned short mode;     // Permissions + SHM_DEST and SHM_LOCKED flags
 	unsigned short __seq;    // Sequence number
 };

 *******************************************************************************/



#include <stdio.h>
#include <sys/shm.h>
#include <sys/ipc.h>

#include "common.h"


int main()
{
	int shmid = 0;
	key_t key = 0;
	void *shmptr = NULL;
	int size = sizeof(STUDENT) + sizeof(EMPLOYEE) + sizeof(BOOK);
	char option = 0;

	key = ftok("shm.cpp", 'A');

	shmid = shmget(key, size, 0666 | IPC_CREAT);
	if ( shmid == -1){
		printf("\nShared memory allocation failed");
		return 1;
	}

	shmptr = shmat(shmid, NULL, 0);
	if ( shmptr == (void *)-1) {
		shmctl(shmid, IPC_RMID, NULL);
		printf("\nShared memory attach failed");
		return -1;
	}

	do{

		fpurge(stdin);

		switch(option)
		{
			case '1': getStudentInfo ( (STUDENT  *)getPtrFromSharedMem(shmptr, T_STUDENT ) ); break;
			case '2': getEmployeeInfo( (EMPLOYEE *)getPtrFromSharedMem(shmptr, T_EMPLOYEE) ); break;
			case '3': getBookInfo    ( (BOOK     *)getPtrFromSharedMem(shmptr, T_BOOK    ) ); break;
			case '4': goto END;
			default: break;
		}

		printf("\n\n --------- Select Choice --------------");
		printf("\n1. Student Entry");
		printf("\n2. Employee Entry");
		printf("\n3. Book Entry");
		printf("\n4. Exit");
		printf("\nEnter your choice : ");
		fpurge(stdin);
		

	}while ( (option = getchar()) != '4');


END:
	shmdt(shmptr);

	shmctl(shmid, IPC_RMID, NULL);

	return 0;
}
