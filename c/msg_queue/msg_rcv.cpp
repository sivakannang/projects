
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAXSIZE     128
 

 
struct msgbuf
{
    long    mtype;
    char    mtext[MAXSIZE];
};
 
 
int main()
{
    int msg_qid;
    key_t key;
    int ret = 0;
    struct msgbuf msg;
 
    key = ftok("msg_send.cpp", 'A');
 
    msg_qid = msgget(key, 0666);
    if ( msg_qid < 0 ){
	    fprintf(stdout, "msgget() failed");
	    exit(1);
    }
 

    for(;;){ 
	    //Receive an answer of message type 1.
	    printf("\n---------------- Waiting for msg --------------------");
	    memset(&msg, 0, sizeof(msg));
	    ret = msgrcv(msg_qid, &msg, MAXSIZE, 1, 0);
	    if ( ret < 0) {
		    printf("\nmsgrcv() failed");
		    exit(0);
	    }
	    else
		    printf("\nmsg received : %s", msg.mtext);
    }

    return 0;
}
