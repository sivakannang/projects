

/***************************** Message Queue ***********************************
 *
 * Two (or more) processes can exchange information via access to a common system message queue. 
 * The sending process places via some (OS) message-passing module a message onto a queue which can be read by another process.
 * Each message is given an identification or type so that processes can select the appropriate message.
 * Process must share a common key in order to gain access to the queue in the first place
 *
 * Examples: Most of the one way communications
 * a. Printers
 * b. Stock Exchange
 * c. Flight departure/arrivals
 *
 * msgget()      -> create a new queue or attaching with an existing queue
 * msgsnd()      -> Use to place a msg in the queue
 * msgrcv()      -> Receive a message from the queue
 * msgctl()      -> Performs a control operation in a msg queue
 * ftok()        -> use to get an unique key/number across the system
 *
 * msg header    -> sys/msg.h
 * ipc header    -> sys/ipc.h
 * return        -> msgsnd() = msgctl() = 0, msgrcv() = +ve (number of bytes copied into queue), msgget() = +ve  (queue id), ftok() = +ve (key)
 *
*******************************************************************************/

/*****************************  IPC system cmds ********************************
 *
 * #man ipcs
 * #man ipcrm
 * #ipcs -q                  => Display information about system message queues
 * #ipcrm -q msqid           => Remove the message queue associated with the ID msqid from the system
 * #ipcrm -Q msgkey          => Remove the message queue associated with key msgkey from the system
 *
*******************************************************************************/

/***************************** API defenitions *********************************
 *
 * key_t ftok(const char *path, int id)
 * int msgget(key_t key, int mode)
 * int msgsnd(int msqid, const void *msgp, size_t msgsz, int msgflg)
 * ssize_t msgrcv(int msqid, void *msgp, size_t msgsz, long msgtyp, int msgflg)
 * int msgctl(int msqid, int cmd, struct msqid_ds *buf)
 *
 * path      -> any valid file path
 * id        -> could be any character
 * key       -> unique key ( can be generate using ftok())
 * mode      -> permission | mode ( IPC_PRIVATE, IPC_CREAT, IPC_EXCL )
 *               permission : should be in octal, pass always 0666
 *               mode       : IPC_CREAT - create new queue, IPC_PRIVATE - create new queue but private to the creating process, 0 - attach with an existing queue
 * msqid     -> msg queue id returned by msgget()
 * msgp      -> pointer of message buffer
 * msgsz     -> size of the message buffer
 * msgtyp    -> Type of message we wish to read. may be one of
 *               0 - The first message on the queue will be returned.
 *               a positive integer - the first message on the queue whose type (mtype) equals this integer (unless a certain flag is set in msgflg, see below)
 *               a negative integer - the first message on the queue whose type is less then or equal to the absolute value of this integer.
 * msgflg    -> IPC_NOWAIT  - if not able to send or not able to recv msg returns immediately with error EAGAIN or ENOMSG
 *              MSG_COPY    -
 *              MSG_EXCEPT  - Used with msgtyp greater than 0 to read the first message on the queue with message type that differs from msgtyp
 *              MSG_NOERROR - By default if the message text has length greater than msgsz, then the message isn't removed from the queue and the system call fails returning -1 with errno set to E2BIG. When MSG_NOERROR is specified, then the message text will be truncated (and the truncated part will be lost) 
 *
 * cmd       -> IPC_STAT  - Copy information from the kernel data structure associated with msqid into the msqid_ds
 *              IPC_SET   - Write the values of some members of the msqid_ds structure
 *              IPC_RMID  - Immediately remove the message queue idenfied by the msqid 
 *
struct msgbuf
{
    long    mtype;
    char    mtext[MAXSIZE];
};
struct msqid_ds {
    struct ipc_perm msg_perm;     // Ownership and permissions
    time_t          msg_stime;    // Time of last msgsnd(2)
    time_t          msg_rtime;    // Time of last msgrcv(2)
    time_t          msg_ctime;    // Time of last change
    unsigned long   __msg_cbytes; // Current number of bytes in queue (nonstandard)
    msgqnum_t       msg_qnum;     // Current number of messages in queue
    msglen_t        msg_qbytes;   // Maximum number of bytes allowed in queue
    pid_t           msg_lspid;    // PID of last msgsnd(2)
    pid_t           msg_lrpid;    // PID of last msgrcv(2)
};
The ipc_perm structure is defined as follows (the highlighted fields are settable using IPC_SET):
struct ipc_perm {
    key_t          __key;       // Key supplied to msgget(2)
    uid_t          uid;         // Effective UID of owner
    gid_t          gid;         // Effective GID of owner
    uid_t          cuid;        // Effective UID of creator
    gid_t          cgid;        // Effective GID of creator
    unsigned short mode;        // Permissions
    unsigned short __seq;       // Sequence number
};
*******************************************************************************/



#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#define MAXSIZE     128
 
struct msgbuf
{
    long    mtype;
    char    mtext[MAXSIZE];
};


int main()
{
    int msqid;
    key_t key;
    int ret = 0;
    struct msgbuf msg = {0};
    size_t buflen;
 
    key = ftok("msg_send.cpp", 'A');
    printf("\n key value is : %d", key);
 
    msqid = msgget(key, IPC_CREAT | IPC_EXCL | 0666 );   //Get the message queue ID for the given key
    if ( msqid < 0 ){
	    fprintf(stdout, "msgget() failed : %d %d", msqid, errno);
	    exit(1);
    }
 
    //Message Type
    msg.mtype = 1;
    printf("\nEnter msg : ");

    while(fgets(msg.mtext, sizeof(msg.mtext), stdin))
    { 
	    if ( (buflen = strlen(msg.mtext)) < 2)
		    break;

	    ret = msgsnd(msqid, (void *)&msg, buflen, IPC_NOWAIT);
	    if ( ret < 0){
		    printf("\nmsgsnd() failed");
		    break;
	    }

	    memset(&msg.mtext, 0, sizeof(msg.mtext));
	    printf("\nEnter msg : ");
    }

    ret = msgctl(msqid, IPC_RMID, NULL);
    if ( ret < 0)
	    printf("\nRemove message queue failed");
 
    return 0;
}
