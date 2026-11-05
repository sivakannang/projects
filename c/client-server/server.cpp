/*
      g++ server.cpp -lssl -lcrypto -lpthread -o server      (Linux only: uses epoll)

      TLS (isSSL in getSocketInfo) works in the iterative, multi-process and multi-thread servers.
      The select, poll and epoll servers are plain TCP and refuse to start with TLS turned on.
*/

/****************************************************************************
 * TCP server : socket -> setsockopt -> bind -> listen -> accept -> recv/send -> close
 * TCP client : socket -> connect -> send/recv -> close
 * UDP        : socket -> bind (server only) -> sendto/recvfrom -> close
 *
 * int socket(int domain, int type, int protocol);
 * int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
 * int listen(int sockfd, int backlog);
 * int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
 * int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
 * int close(int fd);
 *
 * ssize_t send(int sockfd, const void *buf, size_t len, int flags);
 * ssize_t recv(int sockfd, void *buf, size_t len, int flags);
 * ssize_t sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen);
 * ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen);
 *   recv returns 0 when the peer closed, and -1 on error. It does not NUL-terminate.
 *   send and recv may transfer fewer bytes than requested.
 *
 * int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);
 * int getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen);
 * int fcntl(int fd, int op, ...);                 // F_GETFL / F_SETFL with O_NONBLOCK
 *   SO_REUSEADDR  : rebind the port while old connections are in TIME_WAIT
 *   backlog       : pending-connection queue, capped by somaxconn. Not a client limit.
 *
 * I/O multiplexing
 * int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
 *   FD_ZERO, FD_SET, FD_CLR, FD_ISSET. nfds = highest fd + 1. select modifies the sets.
 *   Every fd value must be below FD_SETSIZE (usually 1024). Scans all fds on each call.
 *
 * int poll(struct pollfd *fds, nfds_t nfds, int timeout);
 *   struct pollfd { int fd; short events; short revents; };
 *   No FD_SETSIZE limit, and negative fds are ignored. Still scans the whole array.
 *
 * int epoll_create1(int flags);                   // epoll_create(size) ignores size
 * int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
 * int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);
 *   Linux only. Returns only the ready fds. Level-triggered by default.
 *   EPOLLET reports changes only: use nonblocking fds and read until EAGAIN.
 ****************************************************************************/
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <fcntl.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// OpenSSL
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/x509v3.h>

// Socket
#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/epoll.h>
#include <sys/socket.h>

// POSIX Threads
#include <pthread.h>

#define BUFFER_SIZE       1024
#define LISTEN_BACKLOG    1024
#define RCV_TIMEOUT_SECS  30
#define SEND_WAIT_MS      5000
#define POLL_SIZE         2048
#define MAX_EVENTS        1024
#define PEER_NAME_SIZE    (INET_ADDRSTRLEN + 8)

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

typedef struct
{
	int clientSock;
	SSL_CTX *ctx;
	int id;
}ClientContext;


int Socket_Write(int sock, SSL *ssl, const unsigned char *buffer, int len);
int Socket_Read(int sock, SSL *ssl, unsigned char *buffer, int maxLen);
void Socket_Close(int s);
SocketInfo getSocketInfo(void);
void Socket_Destroy(SocketHandle *Socket);

SSL_CTX *initialize_ctx(const char *certfile, const char *keyfile, const char *CA, const char *password);
int verify_callback(int ok, X509_STORE_CTX *store);

int File_Log(const char *format, ...) __attribute__((format(printf, 1, 2)));
BOOLEAN set_socket_nonblock(int sd);
BOOLEAN set_socket_rcv_timeout(int sock, int secs);

SocketHandle Socket_ServerInit(void);
int Socket_Init(const char *host, unsigned short port, int maxQueue);
void *handleClient_mThread(void *args);

int server(void);
int server_multiProcess(void);
int server_multiThread(void);
int server_select(void);
int server_select_with_client_array(void);
int server_poll(void);
int server_epoll(void);


/*****************************************    Socket and SSL Communication     *************************************************/

SocketInfo getSocketInfo(void)
{
	// Fill the Socket Information here

	SocketInfo sockInfo;

	sockInfo.hostIP = "127.0.0.1";
	sockInfo.hostPort = 13001;
	sockInfo.isSSL = _FALSE;
	sockInfo.certificate = "server.crt";
	sockInfo.keyfile = "server_key.pem";
	sockInfo.caCert = "ca.crt";
	sockInfo.commonName = "unipay";
	sockInfo.password = "";

	return sockInfo;
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

BOOLEAN set_socket_nonblock(int sd)
{
	int flags = fcntl(sd, F_GETFL, 0);
	if ( flags == -1 )
	{
		File_Log("ERROR : fcntl F_GETFL : %s", strerror(errno));
		return _FALSE;
	}

	if ( fcntl(sd, F_SETFL, flags | O_NONBLOCK) == -1 )
	{
		File_Log("ERROR : fcntl F_SETFL : %s", strerror(errno));
		return _FALSE;
	}

	return _TRUE;
}

// Returns bytes read, 0 if the peer closed, or a negative value on error (check errno for plain sockets)
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
// A nonblocking socket with a full send buffer waits up to SEND_WAIT_MS. This is simple,
// but one slow reader stalls an event loop. A production server would queue the data and wait for POLLOUT.
int Socket_Write(int sock, SSL *ssl, const unsigned char *buffer, int len)
{
	int total = 0;

	while ( total < len )
	{
		int written = ssl ? SSL_write(ssl, buffer + total, len - total) : send(sock, buffer + total, len - total, 0);

		if ( written > 0 )
		{
			total += written;
			continue;
		}

		if ( ssl || written == 0 )
			return -1;

		if ( errno == EINTR )
			continue;

		if ( errno == EAGAIN || errno == EWOULDBLOCK )
		{
			struct pollfd pfd;
			int ready;

			pfd.fd = sock;
			pfd.events = POLLOUT;
			pfd.revents = 0;

			ready = poll(&pfd, 1, SEND_WAIT_MS);
			if ( ready > 0 || (ready == -1 && errno == EINTR) )
				continue;
		}

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
	SSL_CTX *ctx = SSL_CTX_new(TLS_server_method());

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

	/* Mutual TLS: a client without a valid certificate fails the handshake */
	SSL_CTX_set_verify_depth(ctx, 2);
	SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, verify_callback);

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


// Returns a connected SSL object, or NULL if the handshake or certificate check failed
static SSL *tls_accept(int sock, SSL_CTX *ctx)
{
	SSL *ssl = SSL_new(ctx);

	if ( ssl == NULL )
	{
		log_ssl_error("SSL_new() FAIL");
		return NULL;
	}

	// SSL_set1_host makes the handshake fail unless the peer certificate matches this name
	if ( SSL_set_fd(ssl, sock) != 1 || SSL_set1_host(ssl, getSocketInfo().commonName) != 1 || SSL_accept(ssl) <= 0 )
	{
		log_ssl_error("SSL_accept() FAIL");
		SSL_free(ssl);
		return NULL;
	}

	return ssl;
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

	snprintf(logfile, sizeof(logfile), "log_s_%02d.txt", lt.tm_mon + 1);

	fd = open(logfile, O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR);
	if ( fd < 0 )
		return -1;

	ret = write(fd, buffer, len);

	close(fd);

	return ret;
}


// inet_ntoa returns a shared static buffer, so it is unsafe in the multi-thread server
static void get_peer_name(int sock, char *out, size_t size)
{
	struct sockaddr_in address;
	socklen_t addr_len = sizeof(address);
	char ip[INET_ADDRSTRLEN];

	if ( getpeername(sock, (struct sockaddr *)&address, &addr_len) == -1 || inet_ntop(AF_INET, &address.sin_addr, ip, sizeof(ip)) == NULL )
	{
		snprintf(out, size, "unknown");
		return;
	}

	snprintf(out, size, "%s:%d", ip, ntohs(address.sin_port));
}


// Returns _FALSE if the listener itself is broken and the server must stop
static BOOLEAN accept_error_is_transient(void)
{
	int err = errno;

	switch ( err )
	{
		case EBADF:
		case EINVAL:
		case ENOTSOCK:
			File_Log("ERROR : accept() : %s", strerror(err));
			return _FALSE;

		case EMFILE:
		case ENFILE:
		case ENOBUFS:
		case ENOMEM:
			File_Log("ERROR : accept() : %s", strerror(err));
			sleep(1);  // Out of resources: the pending connection stays queued, so avoid a busy loop
			return _TRUE;

		default:       // EINTR, ECONNABORTED and network errors affect one connection only
			return _TRUE;
	}
}


int Socket_Init(const char *host, unsigned short port, int maxQueue)
{
	int server_sock;
	int reuse = 1;
	struct sockaddr_in server_address;

	memset(&server_address, 0, sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(port);

	if ( inet_pton(AF_INET, host, &server_address.sin_addr) != 1 )
	{
		File_Log("ERROR : invalid IP address %s", host);
		return -1;
	}

	server_sock = socket(AF_INET, SOCK_STREAM, 0);
	if ( server_sock == -1 )
	{
		File_Log("ERROR : socket() : %s", strerror(errno));
		return -1;
	}

	// Allow rebinding the port while old connections are in TIME_WAIT
	if ( setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1 )
		File_Log("ERROR : SO_REUSEADDR : %s", strerror(errno));

	if ( bind(server_sock, (struct sockaddr *)&server_address, sizeof(server_address)) == -1 )
	{
		File_Log("ERROR : bind() %s:%d : %s", host, port, strerror(errno));
		Socket_Close(server_sock);
		return -1;
	}

	// maxQueue is the pending-connection backlog, not a client limit
	if ( listen(server_sock, maxQueue) == -1 )
	{
		File_Log("ERROR : listen() : %s", strerror(errno));
		Socket_Close(server_sock);
		return -1;
	}

	File_Log("Listening on %s:%d, SD %d", host, port, server_sock);

	return server_sock;
}


SocketHandle Socket_ServerInit(void)
{
	SocketHandle Server;
	SocketInfo sockInfo = getSocketInfo();

	memset(&Server, 0, sizeof(Server));

	Server.sock = Socket_Init(sockInfo.hostIP, sockInfo.hostPort, LISTEN_BACKLOG);
	if ( Server.sock == -1 )
		return Server;

	if ( sockInfo.isSSL )
	{
		Server.ctx = initialize_ctx(sockInfo.certificate, sockInfo.keyfile, sockInfo.caCert, sockInfo.password);
		if ( Server.ctx == NULL )
		{
			Socket_Close(Server.sock);
			Server.sock = -1;
			return Server;
		}
	}

	Server.isConnected = _TRUE;

	return Server;
}


// Nonblocking listener for the select, poll and epoll servers. Returns -1 on failure.
static int open_event_loop_listener(void)
{
	SocketInfo sockInfo = getSocketInfo();
	int listener_sock;

	if ( sockInfo.isSSL )
	{
		File_Log("ERROR : TLS is not supported by the select, poll and epoll servers. Use option 1, 2 or 3.");
		return -1;
	}

	listener_sock = Socket_Init(sockInfo.hostIP, sockInfo.hostPort, LISTEN_BACKLOG);
	if ( listener_sock == -1 )
		return -1;

	// If a client aborts between the readiness event and accept(), a blocking accept() would hang the whole loop
	if ( !set_socket_nonblock(listener_sock) )
	{
		Socket_Close(listener_sock);
		return -1;
	}

	return listener_sock;
}


// Echo loop for one blocking connection: used by the iterative, multi-process and multi-thread servers
static void serve_client(int sock, SSL_CTX *ctx, int id)
{
	char peer[PEER_NAME_SIZE];
	unsigned char buffer[BUFFER_SIZE];
	SSL *ssl = NULL;
	int bytesRead, bytesSent = 0;

	get_peer_name(sock, peer, sizeof(peer));
	File_Log("SD %4d => %s => Client %d => CONNECT", sock, peer, id);

	// Also limits how long a silent client can hold the TLS handshake open
	set_socket_rcv_timeout(sock, RCV_TIMEOUT_SECS);

	if ( ctx )
	{
		ssl = tls_accept(sock, ctx);
		if ( ssl == NULL )
		{
			Socket_Close(sock);
			File_Log("SD %4d => %s => Client %d => DISCONNECT, TLS HANDSHAKE FAILED", sock, peer, id);
			return;
		}
	}

	// Leave room for the NUL, because recv doesn't add one and the data gets logged with %s
	while ( (bytesRead = Socket_Read(sock, ssl, buffer, sizeof(buffer) - 1)) > 0 )
	{
		buffer[bytesRead] = '\0';
		File_Log("SD %4d => %s => Client %d => RCV => %d => %s", sock, peer, id, bytesRead, buffer);

		bytesSent = Socket_Write(sock, ssl, buffer, bytesRead);
		File_Log("SD %4d => %s => Client %d => SND => %d", sock, peer, id, bytesSent);

		if ( bytesSent != bytesRead )
			break;
	}

	if ( ssl )
	{
		SSL_shutdown(ssl);
		SSL_free(ssl);
	}

	Socket_Close(sock);

	File_Log("SD %4d => %s => Client %d => DISCONNECT%s", sock, peer, id, (bytesRead == 0) ? "" : " ON ERROR OR TIMEOUT");
}


// Echoes everything readable on a nonblocking socket, until EAGAIN. Returns _FALSE if the caller must close it.
// Reading until EAGAIN is required with EPOLLET. It also means a client's last message is
// echoed before its disconnect is handled, because recv returns the data first and then 0.
static BOOLEAN echo_until_eagain(int sd)
{
	char peer[PEER_NAME_SIZE];
	unsigned char buffer[BUFFER_SIZE];

	get_peer_name(sd, peer, sizeof(peer));

	while ( _TRUE )
	{
		int bytesRead = Socket_Read(sd, NULL, buffer, sizeof(buffer) - 1);

		if ( bytesRead > 0 )
		{
			int bytesSent;

			buffer[bytesRead] = '\0';
			File_Log("SD %4d => %s => RCV => %d => %s", sd, peer, bytesRead, buffer);

			bytesSent = Socket_Write(sd, NULL, buffer, bytesRead);
			File_Log("SD %4d => %s => SND => %d", sd, peer, bytesSent);

			if ( bytesSent != bytesRead )
			{
				File_Log("SD %4d => %s => DISCONNECT ON SEND ERROR", sd, peer);
				return _FALSE;
			}
		}
		else if ( bytesRead == -1 && (errno == EAGAIN || errno == EWOULDBLOCK) )
		{
			return _TRUE;
		}
		else
		{
			File_Log("SD %4d => %s => DISCONNECT%s", sd, peer, (bytesRead == 0) ? "" : " ON RCV ERROR");
			return _FALSE;
		}
	}
}


void *handleClient_mThread(void *args)
{
	ClientContext *context = (ClientContext *)args;

	serve_client(context->clientSock, context->ctx, context->id);

	free(context);

	return NULL;
}


/* Signal handler to reap zombie processes */
static void wait_for_child(int)
{
	int saved_errno = errno;   // waitpid can change errno, which the interrupted code may be about to read

	while ( waitpid(-1, NULL, WNOHANG) > 0 );

	errno = saved_errno;
}


int server(void)
{
	int peer_sock, id = 0;
	SocketHandle Server = Socket_ServerInit();

	if ( !Server.isConnected )
		return 1;

	while ( _TRUE )
	{
		peer_sock = accept(Server.sock, NULL, NULL);
		if ( peer_sock == -1 )
		{
			if ( accept_error_is_transient() )
				continue;
			break;
		}

		serve_client(peer_sock, Server.ctx, ++id);
	}

	Socket_Destroy(&Server);

	return 1;
}


int server_multiProcess(void)
{
	int peer_sock, id = 0;
	pid_t pid;
	struct sigaction sa;
	SocketHandle Server = Socket_ServerInit();

	if ( !Server.isConnected )
		return 1;

	/* Set up the signal handler */
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = wait_for_child;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if ( sigaction(SIGCHLD, &sa, NULL) == -1 )
	{
		File_Log("ERROR : sigaction : %s", strerror(errno));
		Socket_Destroy(&Server);
		return 1;
	}

	while ( _TRUE )
	{
		peer_sock = accept(Server.sock, NULL, NULL);
		if ( peer_sock == -1 )
		{
			if ( accept_error_is_transient() )
				continue;
			break;
		}

		id++;

		fflush(stdout);   // Otherwise the child inherits and prints a second copy of buffered output

		pid = fork();

		if ( pid == 0 )   // Child: close the duplicate listener and serve this client
		{
			Socket_Close(Server.sock);
			serve_client(peer_sock, Server.ctx, id);
			exit(EXIT_SUCCESS);
		}

		if ( pid == -1 )
			File_Log("ERROR : fork() : %s", strerror(errno));

		Socket_Close(peer_sock);   // Parent: the child owns the connection now
	}

	Socket_Destroy(&Server);

	return 1;
}


int server_multiThread(void)
{
	int peer_sock, ret, id = 0;
	pthread_t tid;
	SocketHandle Server = Socket_ServerInit();

	if ( !Server.isConnected )
		return 1;

	while ( _TRUE )
	{
		ClientContext *context;

		peer_sock = accept(Server.sock, NULL, NULL);
		if ( peer_sock == -1 )
		{
			if ( accept_error_is_transient() )
				continue;
			break;
		}

		// Each thread owns its context, so the main thread can't overwrite it during the next accept
		context = (ClientContext *)malloc(sizeof(*context));
		if ( context == NULL )
		{
			File_Log("ERROR : malloc() failed");
			Socket_Close(peer_sock);
			continue;
		}

		context->clientSock = peer_sock;
		context->ctx = Server.ctx;
		context->id = ++id;

		ret = pthread_create(&tid, NULL, handleClient_mThread, context);
		if ( ret != 0 )
		{
			File_Log("ERROR : pthread_create() : %s", strerror(ret));
			Socket_Close(peer_sock);
			free(context);
			continue;
		}

		pthread_detach(tid);   // Nobody joins these threads, so release their resources on exit
	}

	Socket_Destroy(&Server);

	return 1;
}


int server_select_with_client_array(void)
{
	int i, ret, peer_sock, max_sd;
	int client_sock[FD_SETSIZE];
	fd_set readfds;
	int listener_sock = open_event_loop_listener();

	if ( listener_sock == -1 )
		return 1;

	for ( i = 0; i < FD_SETSIZE; i++ )
		client_sock[i] = -1;

	while ( _TRUE )
	{
		// select modifies the set, so rebuild it from the array on every pass
		FD_ZERO(&readfds);
		FD_SET(listener_sock, &readfds);
		max_sd = listener_sock;

		for ( i = 0; i < FD_SETSIZE; i++ )
		{
			if ( client_sock[i] != -1 )
			{
				FD_SET(client_sock[i], &readfds);
				if ( client_sock[i] > max_sd )
					max_sd = client_sock[i];
			}
		}

		ret = select(max_sd + 1, &readfds, NULL, NULL, NULL);
		if ( ret == -1 )
		{
			if ( errno == EINTR )    // The sets are undefined after an error, so don't read them
				continue;

			File_Log("ERROR : select() : %s", strerror(errno));
			break;
		}

		// Check the client sockets for data or a disconnect
		for ( i = 0; i < FD_SETSIZE; i++ )
		{
			if ( client_sock[i] != -1 && FD_ISSET(client_sock[i], &readfds) && !echo_until_eagain(client_sock[i]) )
			{
				Socket_Close(client_sock[i]);
				client_sock[i] = -1;
			}
		}

		// If something happened on the listener socket, then it's an incoming connection
		if ( FD_ISSET(listener_sock, &readfds) )
		{
			// Accept every queued connection before going back to select
			while ( _TRUE )
			{
				peer_sock = accept(listener_sock, NULL, NULL);
				if ( peer_sock == -1 )
				{
					if ( errno != EAGAIN && errno != EWOULDBLOCK && !accept_error_is_transient() )
						goto END;
					break;
				}

				for ( i = 0; i < FD_SETSIZE && client_sock[i] != -1; i++ );

				// FD_SET on an fd at or above FD_SETSIZE would corrupt memory
				if ( peer_sock >= FD_SETSIZE || i == FD_SETSIZE || !set_socket_nonblock(peer_sock) )
				{
					File_Log("SD %4d => REJECTED, NO CAPACITY", peer_sock);
					Socket_Close(peer_sock);
					continue;
				}

				client_sock[i] = peer_sock;
				File_Log("SD %4d => CONNECT", peer_sock);
			}
		}
	}

END:
	for ( i = 0; i < FD_SETSIZE; i++ )
	{
		if ( client_sock[i] != -1 )
			Socket_Close(client_sock[i]);
	}

	Socket_Close(listener_sock);

	return 1;
}


int server_select(void)
{
	int ret, sd, peer_sock, max_sd;
	fd_set masterfds, tempfds;
	int listener_sock = open_event_loop_listener();

	if ( listener_sock == -1 )
		return 1;

	FD_ZERO(&masterfds);
	FD_SET(listener_sock, &masterfds);
	max_sd = listener_sock;

	while ( _TRUE )
	{
		tempfds = masterfds;   // select modifies the set it is given, so pass a copy

		ret = select(max_sd + 1, &tempfds, NULL, NULL, NULL);
		if ( ret == -1 )
		{
			if ( errno == EINTR )    // The sets are undefined after an error, so don't read them
				continue;

			File_Log("ERROR : select() : %s", strerror(errno));
			break;
		}

		// If something happened on the listener socket, then it's an incoming connection
		if ( FD_ISSET(listener_sock, &tempfds) )
		{
			// Accept every queued connection before going back to select
			while ( _TRUE )
			{
				peer_sock = accept(listener_sock, NULL, NULL);
				if ( peer_sock == -1 )
				{
					if ( errno != EAGAIN && errno != EWOULDBLOCK && !accept_error_is_transient() )
						goto END;
					break;
				}

				// FD_SET on an fd at or above FD_SETSIZE would corrupt memory
				if ( peer_sock >= FD_SETSIZE || !set_socket_nonblock(peer_sock) )
				{
					File_Log("SD %4d => REJECTED, NO CAPACITY", peer_sock);
					Socket_Close(peer_sock);
					continue;
				}

				File_Log("SD %4d => CONNECT", peer_sock);
				FD_SET(peer_sock, &masterfds);
				if ( peer_sock > max_sd )
					max_sd = peer_sock;
			}
		}

		// Check the client sockets for data or a disconnect
		for ( sd = 0; sd <= max_sd; sd++ )
		{
			if ( sd != listener_sock && FD_ISSET(sd, &tempfds) && !echo_until_eagain(sd) )
			{
				FD_CLR(sd, &masterfds);
				Socket_Close(sd);
			}
		}
	}

END:
	for ( sd = 0; sd <= max_sd; sd++ )
	{
		if ( FD_ISSET(sd, &masterfds) )
			Socket_Close(sd);
	}

	return 1;
}


int server_poll(void)
{
	int i, j, ret, peer_sock, current_size, nfds = 1;
	BOOLEAN compress_fd_array;
	struct pollfd fds[POLL_SIZE];
	int listener_sock = open_event_loop_listener();

	if ( listener_sock == -1 )
		return 1;

	memset(fds, 0, sizeof(fds));
	fds[0].fd = listener_sock;
	fds[0].events = POLLIN;

	while ( _TRUE )
	{
		ret = poll(fds, nfds, -1);
		if ( ret == -1 )
		{
			if ( errno == EINTR )
				continue;

			File_Log("ERROR : poll() : %s", strerror(errno));
			break;
		}

		current_size = nfds;
		compress_fd_array = _FALSE;

		if ( fds[0].revents )
		{
			// Accept every queued connection before going back to poll
			while ( _TRUE )
			{
				peer_sock = accept(listener_sock, NULL, NULL);
				if ( peer_sock == -1 )
				{
					if ( errno != EAGAIN && errno != EWOULDBLOCK && !accept_error_is_transient() )
						goto END;
					break;
				}

				if ( nfds >= POLL_SIZE || !set_socket_nonblock(peer_sock) )
				{
					File_Log("SD %4d => REJECTED, NO CAPACITY", peer_sock);
					Socket_Close(peer_sock);
					continue;
				}

				File_Log("SD %4d => CONNECT", peer_sock);

				fds[nfds].fd = peer_sock;
				fds[nfds].events = POLLIN;
				fds[nfds].revents = 0;
				nfds++;
			}
		}

		for ( i = 1; i < current_size; i++ )
		{
			// A disconnect or an error makes the socket readable, and recv reports it after any remaining data
			if ( fds[i].revents && !echo_until_eagain(fds[i].fd) )
			{
				Socket_Close(fds[i].fd);
				fds[i].fd = -1;
				compress_fd_array = _TRUE;
			}
		}

		if ( compress_fd_array )
		{
			// Slide the remaining entries down in one pass
			for ( i = 1, j = 1; i < nfds; i++ )
			{
				if ( fds[i].fd != -1 )
					fds[j++] = fds[i];
			}
			nfds = j;
		}
	}

END:
	for ( i = 0; i < nfds; i++ )
	{
		if ( fds[i].fd != -1 )
			Socket_Close(fds[i].fd);
	}

	return 1;
}


int server_epoll(void)
{
	int i, event_count, peer_sock, epoll_fd;
	struct epoll_event ev;
	struct epoll_event events[MAX_EVENTS];
	int listener_sock = open_event_loop_listener();

	if ( listener_sock == -1 )
		return 1;

	epoll_fd = epoll_create1(EPOLL_CLOEXEC);
	if ( epoll_fd == -1 )
	{
		File_Log("ERROR : epoll_create1() : %s", strerror(errno));
		Socket_Close(listener_sock);
		return 1;
	}

	memset(&ev, 0, sizeof(ev));
	ev.events = EPOLLIN;
	ev.data.fd = listener_sock;

	if ( epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listener_sock, &ev) == -1 )
	{
		File_Log("ERROR : EPOLL CTL ADD : %s", strerror(errno));
		goto END;
	}

	while ( _TRUE )
	{
		event_count = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
		if ( event_count == -1 )
		{
			if ( errno == EINTR )
				continue;

			File_Log("ERROR : epoll_wait() : %s", strerror(errno));
			goto END;
		}

		for ( i = 0; i < event_count; i++ )
		{
			if ( events[i].data.fd == listener_sock )
			{
				// Accept every queued connection before going back to epoll_wait
				while ( _TRUE )
				{
					peer_sock = accept(listener_sock, NULL, NULL);
					if ( peer_sock == -1 )
					{
						if ( errno != EAGAIN && errno != EWOULDBLOCK && !accept_error_is_transient() )
							goto END;
						break;
					}

					if ( !set_socket_nonblock(peer_sock) )
					{
						Socket_Close(peer_sock);
						continue;
					}

					// Edge-triggered: echo_until_eagain must empty the socket on each event.
					// EPOLLHUP and EPOLLERR are always reported, so they aren't requested here.
					ev.events = EPOLLIN | EPOLLET;
					ev.data.fd = peer_sock;

					if ( epoll_ctl(epoll_fd, EPOLL_CTL_ADD, peer_sock, &ev) == -1 )
					{
						File_Log("ERROR : EPOLL CTL ADD : %s", strerror(errno));
						Socket_Close(peer_sock);
						continue;
					}

					File_Log("SD %4d => CONNECT", peer_sock);
				}
			}
			else if ( !echo_until_eagain(events[i].data.fd) )
			{
				Socket_Close(events[i].data.fd);   // close also removes the fd from the epoll set
			}
		}
	}

END:
	Socket_Close(epoll_fd);
	Socket_Close(listener_sock);

	return 1;
}


int main(void)
{
	int choice = 0;

	signal(SIGPIPE, SIG_IGN);   // A write to a closed client must return an error, not kill the server

	printf("\n\n\n---------------- Choose the server you want to run -------------------------\n");
	printf("\n 1. Server (one client at a time)");
	printf("\n 2. Server-Multi-Process");
	printf("\n 3. Server-Multi-Thread");
	printf("\n 4. Server-Select");
	printf("\n 5. Server-Select-With-Client-Array");
	printf("\n 6. Server-Poll");
	printf("\n 7. Server-Epoll");

	printf("\n\n Enter your choice : ");
	if ( scanf("%d", &choice) != 1 )
		choice = 0;

	switch ( choice )
	{
		case 1: return server();
		case 2: return server_multiProcess();
		case 3: return server_multiThread();
		case 4: return server_select();
		case 5: return server_select_with_client_array();
		case 6: return server_poll();
		case 7: return server_epoll();
		default: printf("\n Sorry !!!! , You have entered an invalid choice\n"); return 1;
	}
}
