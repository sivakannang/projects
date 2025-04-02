// gcc rsa_server.c rsa.c -o rsa_server -lcrypto

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/err.h>

#define BUFSIZE 2048

void error(char *msg) {

	perror(msg);
	exit(1);
}

struct ipek_resp
{

	char msg_len[2];
	char msg_type[1];
	char msg_version[1];
	char resp_code[5];
	char encrypted_ipek[4096];
};

int main(int argc, char **argv) {

	int parentfd; 
	int childfd;
	int portno; 
	int clientlen; 
	struct sockaddr_in serveraddr; 
	struct sockaddr_in clientaddr; 
	struct hostent *hostp;
	unsigned char buf[BUFSIZE]; 
	char *hostaddrp; 
	int optval; 
	int n; 
	int i;
	struct ipek_resp ipek_r;


	if (argc != 2) 
	{

		fprintf(stderr, "usage: %s <port>\n", argv[0]);
		exit(1);
	}
	portno = atoi(argv[1]);


	parentfd = socket(AF_INET, SOCK_STREAM, 0);
	if (parentfd < 0) 
		error("ERROR opening socket");


	optval = 1;
	setsockopt(parentfd, SOL_SOCKET, SO_REUSEADDR, 
			(const void *)&optval , sizeof(int));

	bzero((char *) &serveraddr, sizeof(serveraddr));

	serveraddr.sin_family = AF_INET;

	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

	serveraddr.sin_port = htons((unsigned short)portno);

	if (bind(parentfd, (struct sockaddr *) &serveraddr, 
				sizeof(serveraddr)) < 0) 
		error("ERROR on binding");


	if (listen(parentfd, 5) < 0) /* allow 5 requests to queue up */ 
		error("ERROR on listen");

	clientlen = sizeof(clientaddr);
	memset(&ipek_r,'\0',sizeof(ipek_r));

	ipek_r.msg_type[0] = 0x1;
	ipek_r.msg_version[0] = 0x1;
	ipek_r.resp_code[0] = 0x6;	

		//childfd = accept(parentfd, (struct sockaddr *) &clientaddr, &clientlen);

	while (1) {

		printf("\n--------- Listening --------------");
		childfd = accept(parentfd, (struct sockaddr *) &clientaddr, &clientlen);
		if (childfd < 0) 
			error("ERROR on accept");

		bzero(buf, BUFSIZE);
		n = recv(childfd, buf, BUFSIZE, 0);
		//n = read(childfd, buf, BUFSIZE);
		if (n < 0) 
			error("ERROR reading from socket");
		printf("server received %d bytes: \n", n);
		for(i=0; i < n; i++)
			printf("%.2X ",buf[i]);

		

		printf("\n");

		{
			int temp_byte1 , temp_byte2 = 0;
			int len=0;
			int ipek_len = 16;
			unsigned char send_buffer[4096];

			//char ipek[16] = {0x83,0xad,0xb6,0x11,0x3f,0x94,0xe9,0x55,0xf6,0x7a,0x0e,0x62,0xaa,0xbd,0xe9,0xa2};
			char ipek[16] = {0x6a ,0xc2 ,0x92 ,0xfa ,0xa1 ,0x31 ,0x5b ,0x4d ,0x85 ,0x8a ,0xb3 ,0xa3 ,0xd7 ,0xd5 ,0x93 ,0x3a };


			unsigned char  encrypted[4098]={};
			int encrypted_length= public_encrypt(ipek,ipek_len,buf+9+5,encrypted);  // skip 9 bytes for msg type, version and bdk id. skip 5 bytes for header
			if(encrypted_length == -1)
			{
				printLastError("Public Encrypt failed ");
				exit(0);
			}

			printf("Encrypted length =%d\n",encrypted_length);

			memcpy(send_buffer+5+5,encrypted,encrypted_length);
			len = encrypted_length+3+5;
			temp_byte1 = (len) & 0x00ff;
			temp_byte2 = (len) & 0xff00;
			temp_byte2 = temp_byte2 >> 8;

			ipek_r.msg_len[0] = send_buffer[0] = temp_byte2 ;
			ipek_r.msg_len[1] = send_buffer[1] = temp_byte1;
			//setting tpdu
			send_buffer[2] = buf[2], send_buffer[3] = buf[5], send_buffer[4] = buf[6] , send_buffer[5] = buf[3], send_buffer[6] = buf[4];
			//setting msg type, version and ACK
			send_buffer[7] = 0x2, send_buffer[8]=0x1, send_buffer[9] = 0x6;

			//n = write(childfd, send_buffer, len+2);
			n = send(childfd, send_buffer, len+2, 0);
			if (n < 0) 
				error("ERROR writing to socket");

			printf("ip header : %02X %02X\n", send_buffer[0], send_buffer[1]);
			printf("\n %d bytes sent", n);

			close(childfd);

		}

	}
}
