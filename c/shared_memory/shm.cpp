// g++ shm.cpp common.cpp -o shm
// g++ shm_access.cpp common.cpp -o shm_access
//
// Run shm from this directory and enter some data. While it is still running, run
// shm_access from another window to read the same memory. shm removes the segment when it exits.

/***************************** Shared Memory ***********************************
 *
 * Shared memory is the fastest form of IPC: once it is attached, processes read and write
 * the same memory directly, with no system call and no copy through the kernel.
 *
 * The kernel doesn't synchronize access, though. If one process can write while another
 * reads, protect the data with a semaphore or a mutex: see ../semaphore. This example doesn't.
 *
 * shmget()      -> create a shared memory segment, or get an existing one
 * shmat()       -> attach a segment to this process's address space
 * shmdt()       -> detach a segment
 * shmctl()      -> perform a control operation on a segment
 * ftok()        -> build a key from a file path and an id
 *
 * shm header    -> sys/shm.h
 * ipc header    -> sys/ipc.h
 * return        -> shmget = segment id, which can be 0, or -1 on error
 *                  shmat  = address of the segment, or (void *)-1 on error
 *                  shmctl = shmdt = 0, or -1 on error
 *
 * A segment stays in the kernel until it is removed or the system restarts, even with
 * no process attached. A program that crashes before IPC_RMID leaves it behind.
 *
*******************************************************************************/

/*****************************  IPC system cmds ********************************
 *
 * #ipcs -m                 => Display information about shared memory
 * #ipcrm -m shmid          => Removes the segment with this ID
 * #ipcrm -M shmkey         => Removes the segment with this key
 *
*******************************************************************************/

/***************************** API definitions *********************************
 *
 * int shmget(key_t key, size_t size, int shmflg)
 * void *shmat(int shmid, const void *shmaddr, int shmflg)
 * int shmdt(const void *shmaddr)
 * int shmctl(int shmid, int cmd, struct shmid_ds *buf)
 * key_t ftok(const char *path, int id)
 *
 * path      -> an existing file. The key comes from its inode, so the key isn't guaranteed to be unique.
 * id        -> any non-zero value in the low 8 bits
 * key       -> from ftok(), or IPC_PRIVATE for a new segment that other processes can't find by key
 * size      -> requested size, rounded up to a whole number of pages. New segments are filled with zeros.
 * shmget's shmflg -> permissions | IPC_CREAT | IPC_EXCL
 *               permissions : in octal. Use 0600 unless other users need access
 *               IPC_CREAT   : create the segment if it doesn't exist
 *               IPC_EXCL    : with IPC_CREAT, fail if it already exists
 *               0           : only get an existing segment
 * shmid     -> segment id returned by shmget()
 * shmaddr   -> shmat: NULL lets the kernel choose the address. shmdt: the address shmat returned
 * shmat's shmflg  -> 0 for read and write access, SHM_RDONLY for read only
 * cmd       -> IPC_STAT  - copy the segment's information into the shmid_ds
 *              IPC_SET   - set some of its members from the shmid_ds
 *              IPC_RMID  - mark the segment for removal. It is really destroyed when the
 *                          last process detaches, and until then the key no longer finds it.
 *
 * struct shmid_ds {
 *     struct ipc_perm shm_perm;    // Ownership and permissions
 *     size_t          shm_segsz;   // Size of segment (bytes)
 *     time_t          shm_atime;   // Last attach time
 *     time_t          shm_dtime;   // Last detach time
 *     time_t          shm_ctime;   // Last change time
 *     pid_t           shm_cpid;    // PID of creator
 *     pid_t           shm_lpid;    // PID of last shmat/shmdt
 *     shmatt_t        shm_nattch;  // No. of current attaches
 *     ...
 * };
 *
 * struct ipc_perm {
 *     key_t          __key;    // Key supplied to shmget
 *     uid_t          uid;      // Effective UID of owner (settable with IPC_SET)
 *     gid_t          gid;      // Effective GID of owner (settable with IPC_SET)
 *     uid_t          cuid;     // Effective UID of creator
 *     gid_t          cgid;     // Effective GID of creator
 *     unsigned short mode;     // Permissions (settable with IPC_SET), + SHM_DEST and SHM_LOCKED flags
 *     unsigned short __seq;    // Sequence number
 * };
 *
*******************************************************************************/


#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "common.h"


int main()
{
	int shmid;
	key_t key;
	SHARED_DATA *data;
	bool running = true;

	key = ftok("shm.cpp", 'A');
	if ( key == -1 )
	{
		perror("ftok(shm.cpp), run this from its own directory");
		return 1;
	}

	shmid = shmget(key, sizeof(SHARED_DATA), 0600 | IPC_CREAT);
	if ( shmid == -1 )
	{
		perror("shmget");
		return 1;
	}

	data = (SHARED_DATA *)shmat(shmid, NULL, 0);
	if ( data == (void *)-1 )
	{
		perror("shmat");
		shmctl(shmid, IPC_RMID, NULL);
		return 1;
	}

	while ( running )
	{
		char option[16];

		printf("\n --------- Select Choice --------------\n");
		printf("1. Student Entry\n");
		printf("2. Employee Entry\n");
		printf("3. Book Entry\n");
		printf("4. Exit\n");

		if ( !readLine("Enter your choice : ", option, sizeof(option)) )
			break;

		switch ( option[0] )
		{
			case '1': running = getStudentInfo(&data->student); break;
			case '2': running = getEmployeeInfo(&data->employee); break;
			case '3': running = getBookInfo(&data->book); break;
			case '4': running = false; break;
			default: printf("Invalid choice\n"); break;
		}
	}

	shmdt(data);

	shmctl(shmid, IPC_RMID, NULL);

	return 0;
}
