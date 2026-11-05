// g++ msg_ctl.cpp -o msg_ctl
//
// Performs one control operation on one message queue, using msgctl().
// Find the msqid with: ipcs -q
//
// Careful with IPC_SET: if you remove your own read permission, this program can't reset it.

#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <time.h>

void do_msgctl(int msqid, int cmd, struct msqid_ds *buf);

// %li accepts C notation: 0x for hexadecimal, a leading 0 for octal, otherwise decimal
static long readNumber(const char *prompt)
{
	long value;

	fprintf(stderr, "%s", prompt);

	if ( scanf("%li", &value) != 1 )
	{
		fprintf(stderr, "Expected a number\n");
		exit(1);
	}

	return value;
}

int main()
{
	struct msqid_ds buf;
	int msqid, cmd;

	fprintf(stderr, "All numeric input follows C conventions: 0x... is hexadecimal, 0... is octal, otherwise decimal.\n");

	msqid = (int)readNumber("Enter the msqid: ");

	fprintf(stderr, "\tIPC_RMID = %d\n", IPC_RMID);
	fprintf(stderr, "\tIPC_SET = %d\n", IPC_SET);
	fprintf(stderr, "\tIPC_STAT = %d\n", IPC_STAT);
	cmd = (int)readNumber("Enter the value for the command: ");

	switch ( cmd )
	{
		case IPC_SET:
			fprintf(stderr, "Before IPC_SET, get current values:\n");
			/* fall through */

		case IPC_STAT:
			do_msgctl(msqid, IPC_STAT, &buf);
			fprintf(stderr, "msg_perm.uid = %ld\n", (long)buf.msg_perm.uid);
			fprintf(stderr, "msg_perm.gid = %ld\n", (long)buf.msg_perm.gid);
			fprintf(stderr, "msg_perm.cuid = %ld\n", (long)buf.msg_perm.cuid);
			fprintf(stderr, "msg_perm.cgid = %ld\n", (long)buf.msg_perm.cgid);
			fprintf(stderr, "msg_perm.mode = %#o, access permissions = %#o\n", (unsigned)buf.msg_perm.mode, (unsigned)(buf.msg_perm.mode & 0777));
			fprintf(stderr, "msg_cbytes = %lu\n", (unsigned long)buf.msg_cbytes);
			fprintf(stderr, "msg_qbytes = %lu\n", (unsigned long)buf.msg_qbytes);
			fprintf(stderr, "msg_qnum = %lu\n", (unsigned long)buf.msg_qnum);
			fprintf(stderr, "msg_lspid = %ld\n", (long)buf.msg_lspid);
			fprintf(stderr, "msg_lrpid = %ld\n", (long)buf.msg_lrpid);
			fprintf(stderr, "msg_stime = %s", buf.msg_stime ? ctime(&buf.msg_stime) : "Not Set\n");
			fprintf(stderr, "msg_rtime = %s", buf.msg_rtime ? ctime(&buf.msg_rtime) : "Not Set\n");
			fprintf(stderr, "msg_ctime = %s", ctime(&buf.msg_ctime));

			if ( cmd == IPC_STAT )
				break;

			// Read into a long first: these members have different sizes
			buf.msg_perm.uid = (uid_t)readNumber("Enter msg_perm.uid: ");
			buf.msg_perm.gid = (gid_t)readNumber("Enter msg_perm.gid: ");
			buf.msg_perm.mode = (unsigned short)readNumber("Enter msg_perm.mode: ");
			buf.msg_qbytes = (msglen_t)readNumber("Enter msg_qbytes: ");
			do_msgctl(msqid, IPC_SET, &buf);
			break;

		case IPC_RMID:
			do_msgctl(msqid, IPC_RMID, NULL);
			break;

		default:
			fprintf(stderr, "Unknown command %d\n", cmd);
			return 1;
	}

	return 0;
}

// Reports the call and its result. Exits if it fails, because this example doesn't try to recover.
void do_msgctl(int msqid, int cmd, struct msqid_ds *buf)
{
	int ret;

	fprintf(stderr, "\nmsgctl: Calling msgctl(%d, %d, %s)\n", msqid, cmd, buf ? "&buf" : "NULL");

	ret = msgctl(msqid, cmd, buf);
	if ( ret == -1 )
	{
		perror("msgctl: msgctl failed");
		exit(1);
	}

	fprintf(stderr, "msgctl: msgctl returned %d\n", ret);
}
