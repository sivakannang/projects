// See msg_send.cpp

#include <errno.h>
#include <stdio.h>
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

	key = ftok("msg_send.cpp", 'A');
	if ( key == -1 )
	{
		perror("ftok(msg_send.cpp), run this from its own directory");
		return 1;
	}

	// IPC_CREAT, so this also works when it starts before msg_send
	msqid = msgget(key, IPC_CREAT | 0600);
	if ( msqid < 0 )
	{
		perror("msgget");
		return 1;
	}

	for (;;)
	{
		// Receive the next message of type 1, blocking until there is one.
		// mtext has no NUL, so print it using the length.
		ssize_t len = msgrcv(msqid, &msg, MAXSIZE, 1, 0);

		if ( len >= 0 )
		{
			printf("msg received : %.*s\n", (int)len, msg.mtext);
			fflush(stdout);
		}
		else if ( errno == EIDRM || errno == EINVAL )
		{
			printf("The queue was removed. Exiting.\n");
			return 0;
		}
		else if ( errno != EINTR )
		{
			perror("msgrcv");
			return 1;
		}
	}
}
