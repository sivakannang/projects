// gcc rsa_client.c rsa.c -o rsa_client -lcrypto
// ./rsa_client <port>
//
// Generates a new RSA key pair on every run, so no private key is stored in the source,
// sends the public key to rsa_server, and decrypts the IPEK that comes back.

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include "rsa.h"

#define RCV_TIMEOUT_SECS 10

static const unsigned char KEY_SET_ID[KEY_SET_ID_LEN] = {0xFF, 0xFF, 0x98, 0x76, 0x54};

static int connect_to_server(unsigned short port)
{
	struct sockaddr_in servaddr;
	struct timeval timeout;
	int fd = socket(AF_INET, SOCK_STREAM, 0);

	if ( fd < 0 )
	{
		perror("socket");
		return -1;
	}

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	servaddr.sin_port = htons(port);

	if ( connect(fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 )
	{
		perror("connect");
		close(fd);
		return -1;
	}

	timeout.tv_sec = RCV_TIMEOUT_SECS;
	timeout.tv_usec = 0;
	setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

	return fd;
}

// Returns 0 if the IPEK was received and decrypted
static int request_ipek(int fd, EVP_PKEY *key)
{
	unsigned char request[MAX_PAYLOAD];
	unsigned char response[MAX_PAYLOAD];
	unsigned char decrypted[MAX_PAYLOAD];
	int offset = 0, pemLen, len, decryptedLen, i;

	// Request: type, version, key set id, public key
	request[offset++] = MSG_IPEK_REQUEST;
	request[offset++] = MSG_VERSION;
	memcpy(request + offset, KEY_SET_ID, KEY_SET_ID_LEN);
	offset += KEY_SET_ID_LEN;

	pemLen = rsa_public_key_pem(key, (char *)request + offset, sizeof(request) - offset);
	if ( pemLen == -1 )
	{
		printLastError("Public key export failed");
		return -1;
	}

	if ( write_frame(fd, request, offset + pemLen) == -1 )
	{
		perror("send");
		return -1;
	}

	len = read_frame(fd, response, sizeof(response));
	if ( len == -1 )
	{
		fprintf(stderr, "No valid response from the server\n");
		return -1;
	}

	printf("Received %d bytes from server\n", len);

	// Response: type, version, ACK, encrypted IPEK
	if ( len <= 3 || response[0] != MSG_IPEK_RESPONSE || response[1] != MSG_VERSION || response[2] != MSG_ACK )
	{
		fprintf(stderr, "Not a valid IPEK response\n");
		return -1;
	}

	decryptedLen = rsa_private_decrypt(response + 3, len - 3, key, decrypted, sizeof(decrypted));
	if ( decryptedLen == -1 )
	{
		printLastError("Private Decrypt failed");
		return -1;
	}

	printf("IPEK (%d bytes):", decryptedLen);
	for ( i = 0; i < decryptedLen; i++ )
		printf(" %02X", decrypted[i]);
	printf("\n");

	return (decryptedLen == IPEK_LEN) ? 0 : -1;
}

int main(int argc, char **argv)
{
	int fd, ret;
	long port;
	EVP_PKEY *key;

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

	key = rsa_generate_key(MIN_RSA_BITS);
	if ( key == NULL )
	{
		printLastError("Key generation failed");
		return 1;
	}

	fd = connect_to_server((unsigned short)port);
	if ( fd == -1 )
	{
		EVP_PKEY_free(key);
		return 1;
	}

	ret = request_ipek(fd, key);

	close(fd);
	EVP_PKEY_free(key);

	return (ret == 0) ? 0 : 1;
}
