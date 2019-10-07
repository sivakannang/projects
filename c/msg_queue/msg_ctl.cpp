/*
 * msgctl.c:  Illustrate the msgctl() function.
 *
 * This is a simple exerciser of the msgctl() function.  It allows
 * you to perform one control operation on one message queue.  It
 * gives up immediately if any control operation fails, so be
 careful
 * not to set permissions to preclude read permission; you won't
 be
 * able to reset the permissions with this code if you do.
 */
#include   <stdio.h>
#include   <sys/types.h>
#include   <sys/ipc.h>
#include   <sys/msg.h>
#include   <time.h>
#include <stdlib.h>

void do_msgctl(int msqid, int cmd, struct msqid_ds *buf);
extern void perror();
char warning_message[] = "If you remove read permission yourself, this program will fail frequently!";

int main()
{
	struct msqid_ds buf;     /* queue descriptor buffer for IPC_STAT
				    and IP_SET commands */
	int    cmd,  /* command to be given to msgctl() */
	       msqid;  /* queue ID to be given to msgctl() */

	fprintf(stderr, "All numeric input is expected to follow C conventions:\n");
	fprintf(stderr, "\t0x... is interpreted as hexadecimal,\n");
	fprintf(stderr, "\t0... is interpreted as octal,\n");
	fprintf(stderr, "\totherwise, decimal.\n");

	/* Get the msqid and cmd arguments for the msgctl() call. */
	fprintf(stderr, "Please enter arguments for msgctls() as requested.");
	fprintf(stderr, "\nEnter the msqid: ");
	scanf("%i", &msqid);
	fprintf(stderr, "\tIPC_RMID = %d\n", IPC_RMID);
	fprintf(stderr, "\tIPC_SET = %d\n", IPC_SET);
	fprintf(stderr, "\tIPC_STAT = %d\n", IPC_STAT);
	fprintf(stderr, "\nEnter the value for the command: ");
	scanf("%i", &cmd);

	 switch (cmd) {
	 case IPC_SET:
	 /* Modify settings in the message queue control structure.
	 */
		 fprintf(stderr, "Before IPC_SET, get current values:");
		 /* fall through to IPC_STAT processing */
	 case IPC_STAT:
		 /* Get a copy of the current message queue control
		  * structure and show it to the user. */
		 do_msgctl(msqid, IPC_STAT, &buf);
		 fprintf(stderr, "msg_perm.uid = %d\n", buf.msg_perm.uid);
		 fprintf(stderr, "msg_perm.gid = %d\n", buf.msg_perm.gid);
		 fprintf(stderr, "msg_perm.cuid = %d\n", buf.msg_perm.cuid);
		 fprintf(stderr, "msg_perm.cgid = %d\n", buf.msg_perm.cgid);
		 fprintf(stderr, "msg_perm.mode = %#o, ", buf.msg_perm.mode);
		 fprintf(stderr, "access permissions = %#o\n", buf.msg_perm.mode & 0777);
		 fprintf(stderr, "msg_cbytes = %d\n", buf.msg_cbytes);
		 fprintf(stderr, "msg_qbytes = %d\n", buf.msg_qbytes);
		 fprintf(stderr, "msg_qnum = %d\n", buf.msg_qnum);
		 fprintf(stderr, "msg_lspid = %d\n", buf.msg_lspid);
		 fprintf(stderr, "msg_lrpid = %d\n", buf.msg_lrpid);
		 fprintf(stderr, "msg_stime = %s", buf.msg_stime ? ctime(&buf.msg_stime) : "Not Set\n");
		 fprintf(stderr, "msg_rtime = %s", buf.msg_rtime ? ctime(&buf.msg_rtime) : "Not Set\n");
		 fprintf(stderr, "msg_ctime = %s", ctime(&buf.msg_ctime));
		 if (cmd == IPC_STAT)
			 break;
		 /*  Now continue with IPC_SET. */
		 fprintf(stderr, "Enter msg_perm.uid: ");
		 scanf ("%hi", &buf.msg_perm.uid);
		 fprintf(stderr, "Enter msg_perm.gid: ");
		 scanf("%hi", &buf.msg_perm.gid);
		 fprintf(stderr, "%s\n", warning_message);
		 fprintf(stderr, "Enter msg_perm.mode: ");
		 scanf("%hi", &buf.msg_perm.mode);
		 fprintf(stderr, "Enter msg_qbytes: ");
		 scanf("%hi", &buf.msg_qbytes);
		 do_msgctl(msqid, IPC_SET, &buf);
		 break;
	 case IPC_RMID:
	 default:
		 /* Remove the message queue or try an unknown command. */
		 do_msgctl(msqid, cmd, (struct msqid_ds *)NULL);
		 break;
	 }
	 exit(0);
}

/*
 * Print indication of arguments being passed to msgctl(), call
 * msgctl(), and report the results. If msgctl() fails, do not
 * return; this example doesn't deal with errors, it just reports
 * them.
 */
void do_msgctl(int msqid, int cmd, struct msqid_ds *buf)
{
	int rtrn;  /* hold area for return value from msgctl()*/

	fprintf(stderr, "\nmsgctl: Calling msgctl(%d, %d, %s)\n", msqid, cmd, buf ? "&buf" : "(struct msqid_ds *)NULL");
	rtrn = msgctl(msqid, cmd, buf);
	if (rtrn == -1) {
		perror("msgctl: msgctl failed");
		exit(1);
	} else {
		fprintf(stderr, "msgctl: msgctl returned %d\n", rtrn);
	}
}
