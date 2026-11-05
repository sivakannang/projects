/*
      g++ client.cpp -lssl -lcrypto -lpthread -o client      (Linux)
*/

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

// OpenSSL
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/x509v3.h>

// Socket
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>

// POSIX Threads
#include <pthread.h>

#define BUFFER_SIZE       1024
#define RCV_TIMEOUT_SECS  30
#define MAX_ATTEMPT       1024
#define MAX_THREADS       128
#define MAX_CLIENTS       1000

typedef enum
{
	_FALSE,
	_TRUE
}BOOLEAN;

typedef struct SOCKET_HANDLER
{
	int sock;
	SSL_CTX *ctx;
	SSL *ssl;
	unsigned short isConnected;
}SocketHandle;

typedef struct SOCKET_INFO
{
	const char *hostIP;
	unsigned short hostPort;
	short isSSL;
	const char *certificate;
	const char *keyfile;
	const char *password;
	const char *caCert;
	const char *commonName;
}SocketInfo;


int Socket_Open(const char *host, unsigned short port);
int Socket_Write(int sock, SSL *ssl, const unsigned char *buffer, int len);
int Socket_Read(int sock, SSL *ssl, unsigned char *buffer, int maxLen);
void Socket_Close(int s);
SocketInfo getSocketInfo(void);
void Socket_Destroy(SocketHandle *Socket);

SSL_CTX *initialize_ctx(const char *certfile, const char *keyfile, const char *CA, const char *password);
int verify_callback(int ok, X509_STORE_CTX *store);

int File_Log(const char *format, ...) __attribute__((format(printf, 1, 2)));
BOOLEAN set_socket_rcv_timeout(int sock, int secs);

SocketHandle Socket_ClientInit(void);
void *client(void *args);

int client_sequential(void);
int client_multiThread(void);
int client_many_open_connections(void);


/*****************************************    Socket and SSL Communication     *************************************************/

SocketInfo getSocketInfo(void)
{
	// Fill the Socket Information here

	SocketInfo sockInfo;

	sockInfo.hostIP = "127.0.0.1";
	sockInfo.hostPort = 13001;
	sockInfo.isSSL = _FALSE;
	sockInfo.certificate = "client.crt";
	sockInfo.keyfile = "client_key.pem";
	sockInfo.caCert = "ca.crt";
	sockInfo.commonName = "unipay";
	sockInfo.password = "";

	return sockInfo;
}


// host can be an IP address or a domain name. Returns the connected socket, or -1.
int Socket_Open(const char *host, unsigned short port)
{
	struct addrinfo hints, *result, *addr;
	char service[16];
	int ret, sock = -1;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	snprintf(service, sizeof(service), "%d", port);

	ret = getaddrinfo(host, service, &hints, &result);
	if ( ret != 0 )
	{
		File_Log("ERROR : getaddrinfo() %s:%d : %s", host, port, gai_strerror(ret));
		return -1;
	}

	for ( addr = result; addr != NULL; addr = addr->ai_next )
	{
		sock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
		if ( sock == -1 )
			continue;

		if ( connect(sock, addr->ai_addr, addr->ai_addrlen) == 0 )
			break;

		Socket_Close(sock);
		sock = -1;
	}

	freeaddrinfo(result);

	if ( sock == -1 )
		File_Log("ERROR : connect() %s:%d failed : %s", host, port, strerror(errno));

	return sock;
}

void Socket_Close(int s)
{
	close(s);
}

BOOLEAN set_socket_rcv_timeout(int sock, int secs)
{
	struct timeval tv;
	tv.tv_sec = secs;
	tv.tv_usec = 0;

	if ( setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) == -1 )
	{
		File_Log("ERROR : SET SOCKET RECEIVE TIMEOUT : %s", strerror(errno));
		return _FALSE;
	}

	return _TRUE;
}

// Returns bytes read, 0 if the peer closed, or a negative value on error
int Socket_Read(int sock, SSL *ssl, unsigned char *buffer, int maxLen)
{
	int bytesRead;

	if ( ssl )
		return SSL_read(ssl, buffer, maxLen);

	do
	{
		bytesRead = recv(sock, buffer, maxLen, 0);
	} while ( bytesRead == -1 && errno == EINTR );

	return bytesRead;
}

// Writes the whole buffer, handling partial writes. Returns len, or -1 on error.
int Socket_Write(int sock, SSL *ssl, const unsigned char *buffer, int len)
{
	int total = 0;

	while ( total < len )
	{
		int written = ssl ? SSL_write(ssl, buffer + total, len - total) : send(sock, buffer + total, len - total, 0);

		if ( written > 0 )
			total += written;
		else if ( ssl || written == 0 || errno != EINTR )
			return -1;
	}

	return total;
}

void Socket_Destroy(SocketHandle *Socket)
{
	if ( Socket->ssl )
	{
		SSL_shutdown(Socket->ssl);
		SSL_free(Socket->ssl);
	}

	Socket_Close(Socket->sock);

	if ( Socket->ctx )
		SSL_CTX_free(Socket->ctx);
}


static void log_ssl_error(const char *what)
{
	char details[256];

	ERR_error_string_n(ERR_get_error(), details, sizeof(details));
	File_Log("%s : %s", what, details);
}

static int password_cb(char *buf, int size, int, void *userdata)
{
	const char *password = (const char *)userdata;
	int len = strlen(password);

	if ( len >= size )
		return 0;

	memcpy(buf, password, len + 1);

	return len;
}

SSL_CTX *initialize_ctx(const char *certfile, const char *keyfile, const char *CA, const char *password)
{
	SSL_CTX *ctx = SSL_CTX_new(TLS_client_method());

	if ( ctx == NULL )
	{
		log_ssl_error("SSL CTX CREATE FAIL");
		return NULL;
	}

	SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION);
	SSL_CTX_set_default_passwd_cb(ctx, password_cb);
	SSL_CTX_set_default_passwd_cb_userdata(ctx, (void *)password);

	/* Load our keys and certificates */
	if ( SSL_CTX_use_certificate_file(ctx, certfile, SSL_FILETYPE_PEM) != 1 )
	{
		log_ssl_error("CERTIFICATE FILE LOAD FAIL");
		SSL_CTX_free(ctx);
		return NULL;
	}

	if ( SSL_CTX_use_PrivateKey_file(ctx, keyfile, SSL_FILETYPE_PEM) != 1 )
	{
		log_ssl_error("KEY FILE LOAD FAIL");
		SSL_CTX_free(ctx);
		return NULL;
	}

	/* Load the CAs we trust */
	if ( SSL_CTX_load_verify_locations(ctx, CA, NULL) != 1 )
	{
		log_ssl_error("CA LOAD FAIL");
		SSL_CTX_free(ctx);
		return NULL;
	}

	SSL_CTX_set_verify_depth(ctx, 2);
	SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, verify_callback);

	return ctx;
}


int verify_callback(int ok, X509_STORE_CTX *store)
{
	if ( !ok )
	{
		char data[256];
		X509 *cert = X509_STORE_CTX_get_current_cert(store);
		int err = X509_STORE_CTX_get_error(store);

		File_Log("Error with certificate at depth: %d", X509_STORE_CTX_get_error_depth(store));
		if ( cert )
		{
			X509_NAME_oneline(X509_get_issuer_name(cert), data, sizeof(data));
			File_Log("Issuer = %s", data);
			X509_NAME_oneline(X509_get_subject_name(cert), data, sizeof(data));
			File_Log("Subject = %s", data);
		}
		File_Log("Err %d:%s", err, X509_verify_cert_error_string(err));
	}

	return ok;
}


int File_Log(const char *format, ...)
{
	char buffer[4096];
	char logfile[30];
	time_t t = time(NULL);
	struct tm lt;
	va_list args;
	int fd, ret, len;

	localtime_r(&t, &lt);

	len = snprintf(buffer, sizeof(buffer), "\n%02d%02d%02d: %02d/%02d/%02d: => ", lt.tm_hour, lt.tm_min, lt.tm_sec, lt.tm_mon + 1, lt.tm_mday, lt.tm_year - 100);

	va_start(args, format);
	ret = vsnprintf(buffer + len, sizeof(buffer) - len, format, args);
	va_end(args);

	if ( ret > 0 )
		len = ((size_t)(len + ret) < sizeof(buffer)) ? len + ret : (int)sizeof(buffer) - 1;

	printf("%s", buffer);

	snprintf(logfile, sizeof(logfile), "log_c_%02d.txt", lt.tm_mon + 1);

	fd = open(logfile, O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR);
	if ( fd < 0 )
		return -1;

	ret = write(fd, buffer, len);

	close(fd);

	return ret;
}


SocketHandle Socket_ClientInit(void)
{
	SocketHandle Client;
	SocketInfo sockInfo = getSocketInfo();

	memset(&Client, 0, sizeof(Client));

	Client.sock = Socket_Open(sockInfo.hostIP, sockInfo.hostPort);
	if ( Client.sock == -1 )
		return Client;

	// Also limits how long a silent server can hold the TLS handshake open
	set_socket_rcv_timeout(Client.sock, RCV_TIMEOUT_SECS);

	if ( sockInfo.isSSL )
	{
		Client.ctx = initialize_ctx(sockInfo.certificate, sockInfo.keyfile, sockInfo.caCert, sockInfo.password);
		if ( Client.ctx == NULL )
		{
			Socket_Close(Client.sock);
			return Client;
		}

		Client.ssl = SSL_new(Client.ctx);

		// SSL_set1_host makes the handshake fail unless the server certificate matches this name
		if ( Client.ssl == NULL || SSL_set_fd(Client.ssl, Client.sock) != 1 || SSL_set1_host(Client.ssl, sockInfo.commonName) != 1 || SSL_connect(Client.ssl) <= 0 )
		{
			log_ssl_error("SSL_connect() FAIL");
			if ( Client.ssl )
				SSL_free(Client.ssl);
			SSL_CTX_free(Client.ctx);
			Socket_Close(Client.sock);
			Client.ssl = NULL;
			Client.ctx = NULL;
			return Client;
		}
	}

	Client.isConnected = _TRUE;

	return Client;
}


// Sends message and waits for the full echo, which TCP may deliver in several pieces. Returns _FALSE on failure.
static BOOLEAN send_and_check_echo(SocketHandle *Client, int id, const char *message)
{
	unsigned char buffer[BUFFER_SIZE];
	int len = strlen(message);
	int received = 0;
	int ret;

	ret = Socket_Write(Client->sock, Client->ssl, (const unsigned char *)message, len);
	File_Log("CLIENT %d => SND => %d => %s", id, ret, message);

	if ( ret != len )
		return _FALSE;

	while ( received < len && received < (int)sizeof(buffer) - 1 )
	{
		ret = Socket_Read(Client->sock, Client->ssl, buffer + received, sizeof(buffer) - 1 - received);
		if ( ret <= 0 )
			break;

		received += ret;
	}

	buffer[received] = '\0';
	File_Log("CLIENT %d => RCV => %d => %s", id, received, buffer);

	return (received == len) ? _TRUE : _FALSE;
}


void *client(void *args)
{
	int id = *(int *)args;
	char message[BUFFER_SIZE];
	SocketHandle Client = Socket_ClientInit();

	if ( !Client.isConnected )
	{
		File_Log("CLIENT %d => CONNECT FAILED", id);
		return NULL;
	}

	snprintf(message, sizeof(message), "Hello Server.... I'm from thread number %d", id);

	send_and_check_echo(&Client, id, message);

	Socket_Destroy(&Client);

	return NULL;
}


// Connects, exchanges one message and disconnects, MAX_ATTEMPT times. Works with every server.
int client_sequential(void)
{
	int i;
	char message[BUFFER_SIZE];

	for ( i = 0; i < MAX_ATTEMPT; i++ )
	{
		BOOLEAN ok;
		SocketHandle Client = Socket_ClientInit();

		if ( !Client.isConnected )
			return 1;

		snprintf(message, sizeof(message), "I'm from client %d", i + 1);

		ok = send_and_check_echo(&Client, i, message);

		Socket_Destroy(&Client);

		if ( !ok )
			return 1;
	}

	return 0;
}

// MAX_THREADS clients at the same time, each in its own thread
int client_multiThread(void)
{
	int i, ret;
	int count = 0;
	pthread_t threadID[MAX_THREADS];
	int id[MAX_THREADS];

	for ( i = 0; i < MAX_THREADS; i++ )
	{
		id[i] = i;

		ret = pthread_create(&threadID[i], NULL, client, &id[i]);
		if ( ret != 0 )
		{
			File_Log("Thread %d : pthread_create() : %s", i, strerror(ret));
			break;
		}

		count++;
	}

	for ( i = 0; i < count; i++ )
	{
		ret = pthread_join(threadID[i], NULL);
		if ( ret != 0 )
			File_Log("Thread %d : pthread_join() : %s", i, strerror(ret));
	}

	return 0;
}

// Opens MAX_CLIENTS connections first, and then uses each in turn, keeping them all open.
// Needs a concurrent server. The one-client-at-a-time server would stay blocked on the first connection.
int client_many_open_connections(void)
{
	int i, count = 0;
	char message[BUFFER_SIZE];
	static SocketHandle Clients[MAX_CLIENTS];

	for ( i = 0; i < MAX_CLIENTS; i++ )
	{
		Clients[i] = Socket_ClientInit();
		if ( !Clients[i].isConnected )
			break;

		count++;
	}

	File_Log("Opened %d connections", count);

	for ( i = 0; i < count; i++ )
	{
		snprintf(message, sizeof(message), "I'm from client %d", i + 1);

		// One failed connection shouldn't stop the others
		send_and_check_echo(&Clients[i], i, message);
	}

	for ( i = 0; i < count; i++ )
		Socket_Destroy(&Clients[i]);

	return 0;
}


int main(void)
{
	int choice = 0;

	signal(SIGPIPE, SIG_IGN);   // A write to a closed server must return an error, not kill the client

	printf("\n\n\n---------------- Choose the client you want to run -------------------------\n");
	printf("\n 1. Sequential clients (works with any server)");
	printf("\n 2. Multi-thread clients (works with any server)");
	printf("\n 3. Many open connections (needs server option 2 to 7)");

	printf("\n\n Enter your choice : ");
	if ( scanf("%d", &choice) != 1 )
		choice = 0;

	switch ( choice )
	{
		case 1: return client_sequential();
		case 2: return client_multiThread();
		case 3: return client_many_open_connections();
		default: printf("\n Sorry !!!! , You have entered an invalid choice\n"); return 1;
	}
}
