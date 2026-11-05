// g++ msg_send.cpp -o msg_send
// g++ msg_rcv.cpp -o msg_rcv
//
// Run both from this directory, in either order. Type lines in msg_send, and an empty line to finish.

/***************************** Message Queue ***********************************
 *
 * Processes exchange messages through a queue kept by the kernel.
 * Each message has a type, so a reader can select which messages it wants.
 * Processes find the same queue by using the same key.
 *
 * Unlike a pipe, a message queue keeps message boundaries, and the queue stays in the
 * kernel, with its messages, until it is removed or the system restarts.
 *
 * msgget()      -> create a new queue, or get an existing one
 * msgsnd()      -> place a message in the queue
 * msgrcv()      -> take a message from the queue
 * msgctl()      -> perform a control operation on a queue
 * ftok()        -> build a key from a file path and an id
 *
 * msg header    -> sys/msg.h
 * ipc header    -> sys/ipc.h
 * return        -> msgsnd() = msgctl() = 0
 *                  msgrcv() = number of bytes copied into mtext, which can be 0
 *                  msgget() = queue id, which can be 0
 *                  ftok()   = key
 *                  All return -1 on error.
 *
 * Linux limits  -> /proc/sys/kernel/msgmax (largest message, default 8192 bytes)
 *                  /proc/sys/kernel/msgmnb (largest queue, default 16384 bytes)
 *
*******************************************************************************/

/*****************************  IPC system cmds ********************************
 *
 * #ipcs -q                  => Display information about system message queues
 * #ipcrm -q msqid           => Remove the message queue with this ID
 * #ipcrm -Q msgkey          => Remove the message queue with this key
 *
*******************************************************************************/

/***************************** API definitions *********************************
 *
 * key_t ftok(const char *path, int id)
 * int msgget(key_t key, int msgflg)
 * int msgsnd(int msqid, const void *msgp, size_t msgsz, int msgflg)
 * ssize_t msgrcv(int msqid, void *msgp, size_t msgsz, long msgtyp, int msgflg)
 * int msgctl(int msqid, int cmd, struct msqid_ds *buf)
 *
 * path      -> an existing file. The key comes from its inode, so the key isn't guaranteed to be unique.
 * id        -> any non-zero value in the low 8 bits
 * key       -> from ftok(), or IPC_PRIVATE for a new queue that other processes can't find by key
 * msgget's msgflg -> permissions | IPC_CREAT | IPC_EXCL
 *               permissions : in octal. Use 0600 unless other users need access
 *               IPC_CREAT   : create the queue if it doesn't exist
 *               IPC_EXCL    : with IPC_CREAT, fail if it already exists
 *               0           : only get an existing queue
 * msqid     -> queue id returned by msgget()
 * msgp      -> pointer to the message
 * msgsz     -> size of mtext only, not counting mtype
 * msgtyp    ->  0 - the first message on the queue
 *               positive - the first message of this type
 *               negative - the first message with the lowest type <= the absolute value
 * msgflg    -> IPC_NOWAIT  - fail immediately instead of blocking, with EAGAIN or ENOMSG
 *              MSG_EXCEPT  - with a positive msgtyp, the first message of any other type (Linux)
 *              MSG_NOERROR - cut a message longer than msgsz, instead of failing with E2BIG
 *              MSG_COPY    - copy the message at a position without removing it (Linux)
 *
 * cmd       -> IPC_STAT  - copy the queue's information into the msqid_ds
 *              IPC_SET   - set some of its members from the msqid_ds
 *              IPC_RMID  - remove the queue immediately. Its unread messages are lost,
 *                          and blocked readers and writers fail with EIDRM.
 *
 * The first member of a message must be a long mtype greater than 0:
 *
 * struct Message
 * {
 *     long    mtype;
 *     char    mtext[MAXSIZE];
 * };
 *
 * Don't name it msgbuf: glibc's sys/msg.h has its own struct msgbuf when _GNU_SOURCE is
 * defined, which g++ always does, so that name doesn't compile.
 *
 * struct msqid_ds {
 *     struct ipc_perm msg_perm;     // Ownership and permissions
 *     time_t          msg_stime;    // Time of last msgsnd
 *     time_t          msg_rtime;    // Time of last msgrcv
 *     time_t          msg_ctime;    // Time of last change
 *     unsigned long   __msg_cbytes; // Current number of bytes in queue (nonstandard)
 *     msgqnum_t       msg_qnum;     // Current number of messages in queue
 *     msglen_t        msg_qbytes;   // Maximum number of bytes allowed in queue
 *     pid_t           msg_lspid;    // PID of last msgsnd
 *     pid_t           msg_lrpid;    // PID of last msgrcv
 * };
 *
 * struct ipc_perm {
 *     key_t          __key;       // Key supplied to msgget
 *     uid_t          uid;         // Effective UID of owner (settable with IPC_SET)
 *     gid_t          gid;         // Effective GID of owner (settable with IPC_SET)
 *     uid_t          cuid;        // Effective UID of creator
 *     gid_t          cgid;        // Effective GID of creator
 *     unsigned short mode;        // Permissions (settable with IPC_SET)
 *     unsigned short __seq;       // Sequence number
 * };
 *
*******************************************************************************/


#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>

#define MAXSIZE 128

struct Message
{
	long mtype;
	char mtext[MAXSIZE];
};


int main()
{
	int msqid;
	key_t key;
	struct Message msg;
	int status = 0;

	key = ftok("msg_send.cpp", 'A');
	if ( key == -1 )
	{
		perror("ftok(msg_send.cpp), run this from its own directory");
		return 1;
	}

	msqid = msgget(key, IPC_CREAT | 0600);
	if ( msqid < 0 )
	{
		perror("msgget");
		return 1;
	}

	memset(&msg, 0, sizeof(msg));
	msg.mtype = 1;

	printf("Enter msg : ");
	fflush(stdout);

	while ( fgets(msg.mtext, sizeof(msg.mtext), stdin) )
	{
		size_t len = strlen(msg.mtext);

		if ( len > 0 && msg.mtext[len - 1] == '\n' )
			msg.mtext[--len] = '\0';

		if ( len == 0 )
			break;

		// Blocks while the queue is full, until the reader catches up
		if ( msgsnd(msqid, &msg, len, 0) < 0 )
		{
			perror("msgsnd");
			status = 1;
			break;
		}

		printf("Enter msg : ");
		fflush(stdout);
	}

	// Any messages the reader hasn't taken yet are lost here
	if ( msgctl(msqid, IPC_RMID, NULL) < 0 )
	{
		perror("msgctl(IPC_RMID)");
		status = 1;
	}

	return status;
}
