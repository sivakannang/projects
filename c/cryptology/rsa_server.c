// gcc rsa_server.c rsa.c -o rsa_server -lcrypto
// ./rsa_server <port>
//
// Demo of remote key injection: a client sends its RSA public key, and the server
// returns an IPEK encrypted with that key, so only that client can read the IPEK.
//
// Not safe for production: nothing proves who the client is, so anyone can request the
// key. A real system authenticates the device, for example with a certificate.

#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include "rsa.h"

#define HEADER_LEN        (1 + 1 + KEY_SET_ID_LEN)   // type, version, key set id
#define RCV_TIMEOUT_SECS  10

// Public ANSI X9.24 test vector: the IPEK for BDK 0123456789ABCDEFFEDCBA9876543210, KSN FFFF9876543210E00000
static const unsigned char IPEK[IPEK_LEN] = {0x6A, 0xC2, 0x92, 0xFA, 0xA1, 0x31, 0x5B, 0x4D, 0x85, 0x8A, 0xB3, 0xA3, 0xD7, 0xD5, 0x93, 0x3A};

static void handle_client(int fd)
{
	unsigned char request[MAX_PAYLOAD];
	unsigned char response[MAX_PAYLOAD];
	struct timeval timeout;
	int len, encryptedLen;

	timeout.tv_sec = RCV_TIMEOUT_SECS;
	timeout.tv_usec = 0;
	setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

	len = read_frame(fd, request, sizeof(request));
	if ( len == -1 )
	{
		printf("Bad frame, or the client disconnected\n");
		return;
	}

	printf("Received %d bytes\n", len);

	if ( len <= HEADER_LEN || request[0] != MSG_IPEK_REQUEST || request[1] != MSG_VERSION )
	{
		printf("Not a valid IPEK request\n");
		return;
	}

	// Response: type, version, ACK, encrypted IPEK
	response[0] = MSG_IPEK_RESPONSE;
	response[1] = MSG_VERSION;
	response[2] = MSG_ACK;

	encryptedLen = rsa_public_encrypt(IPEK, sizeof(IPEK), (const char *)request + HEADER_LEN, len - HEADER_LEN, response + 3, sizeof(response) - 3);
	if ( encryptedLen == -1 )
	{
		printLastError("Public Encrypt failed");
		return;
	}

	printf("Encrypted length = %d\n", encryptedLen);

	if ( write_frame(fd, response, 3 + encryptedLen) == -1 )
		perror("send");
}

int main(int argc, char **argv)
{
	int listen_fd;
	int optval = 1;
	long port;
	struct sockaddr_in serveraddr;

	if ( argc != 2 )
	{
		fprintf(stderr, "usage: %s <port>\n", argv[0]);
		return 1;
	}

	port = strtol(argv[1], NULL, 10);
	if ( port < 1 || port > 65535 )
	{
		fprintf(stderr, "invalid port: %s\n", argv[1]);
		return 1;
	}

	signal(SIGPIPE, SIG_IGN);   // A client that disconnects must not kill the server

	listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if ( listen_fd < 0 )
	{
		perror("socket");
		return 1;
	}

	setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	serveraddr.sin_port = htons((unsigned short)port);

	if ( bind(listen_fd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0 )
	{
		perror("bind");
		return 1;
	}

	if ( listen(listen_fd, 16) < 0 )
	{
		perror("listen");
		return 1;
	}

	printf("Listening on 127.0.0.1:%ld\n", port);

	while ( 1 )
	{
		int client_fd = accept(listen_fd, NULL, NULL);
		if ( client_fd < 0 )
		{
			perror("accept");
			continue;
		}

		handle_client(client_fd);

		close(client_fd);
		fflush(stdout);
	}
}
