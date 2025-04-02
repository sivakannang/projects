/*
      g++ server.cpp -lssl -lpthread -o server
*/

/****************************************************************************
 *
 * int socket(int domain, int type, int protocol);
 *
 * int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
 * int listen(int sockfd, int backlog);
 * int accept(int sockfd, struct sockaddr *addr, socklen_t addrlen);
 *
 * int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
 *
 * ssize_t send(int sockfd, const void buf[.size], size_t size, int flags);
 * ssize_t recv(int sockfd, void buf[.size], size_t size, int flags);
 *
 * ssize_t read(int fd, void buf[.count], size_t count);
 * ssize_t write(int fd, const void buf[.count], size_t count);
 *
 * int close(int fd);
 *
 * ssize_t sendto(int sockfd, const void buf[.size], size_t size, int flags, const struct sockaddr *dest_addr, socklen_t addrlen);
 * ssize_t recvfrom(int sockfd, void buf[restrict .size], size_t size, int flags, struct sockaddr *src_addr, socklen_t *addrlen);
 *
 * int setsockopt(int socket, int level, int option_name, const void *option_value, socklen_t option_len);
 * int getsockopt(int socket, int level, int option_name, void *option_value, socklen_t *option_len);
 * int fcntl(int fd, int op, ... );
 * int ioctl(int fd, unsigned long op, ...);
 *
 * int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *excepfds, struct timeval *timeout);
 * void FD_CLR(int fd, fd_set *set);
 * int  FD_ISSET(int fd, fd_set *set);
 * void FD_SET(int fd, fd_set *set);
 * void FD_ZERO(fd_set *set);
 *
 * int poll(struct pollfd *fds, nfds_t nfds, int timeout);
 * struct pollfd {
 * 	int   fd;         // file descriptor
 * 	short events;     // requested events
 * 	short revents;    // returned events
 * };
 *
 *
 * int epoll_create(int size);
 * int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
 * int epoll_wait(int epfd, struct epoll_event events[.maxevents], int maxevents, int timeout);
 *
 *
 *
 *
 *
 *
 * *************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include <signal.h>
#include <sys/wait.h>

// OpenSSL
#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/x509v3.h>

// Socket
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <poll.h>
#include <sys/epoll.h>

// POSIX Threads
#include <pthread.h>

#define LINUX

#ifdef WINDOWS
	#include "windows.h"
#else
	#include "unistd.h"
#endif

#define SOCKET_ERROR   -1

typedef int SOCKET;


typedef enum
{
	_FALSE,
	_TRUE
}BOOLEAN;

typedef struct SOCKET_HANDLER
{
	SOCKET sock;
	SSL_CTX *ctx;
	SSL *ssl;
	unsigned short isConnected;
}SocketHandle;

typedef struct SOCKET_INFO
{
	char hostIP[25];
	unsigned short hostPort;
	short isSSL;
	char certificate[25];
	char keyfile[25];
	char password[25];
	char caCert[25];
	char commonName[25];
}SocketInfo;

typedef struct
{
	SOCKET clientSock;
	SocketHandle *server;
	SocketInfo *sockInfo;
}ClientContext;


BIO *bio_err=0;

SOCKET Socket_Open( const char *host, unsigned short port );
int Socket_Write(SOCKET sock, SSL *ssl, unsigned char *buffer, int len);
int Socket_Read(SOCKET sock, SSL *ssl, unsigned char *buffer, int maxLen);
void Socket_Close(SOCKET s);
SocketInfo getSocketInfo(void);
void Socket_Destroy(SocketHandle *Socket);

SSL_CTX *initialize_ctx (char *certfile, char *keyfile, char *CA, char *password);
void destroy_ctx(SSL_CTX *ctx);
void check_cert_chain (SSL *ssl, char *host);
int verify_callback (int ok, X509_STORE_CTX *store);
static int password_cb(char *buf,int num,int rwflag,void *userdata);
void generate_eph_rsa_key(SSL_CTX *ctx);
void load_dh_params(SSL_CTX *ctx, char *file);

int Time_Get(char *outField);
int File_Log(const char* format, ...);
BOOLEAN set_socket_nonblock(SOCKET sd);
BOOLEAN set_socket_rcv_timeout(SOCKET sock, int secs);

SocketHandle Socket_ServerInit(void);
SOCKET Socket_Init(const char *host, unsigned short port, unsigned short maxQueue);
void *handleClient_mThread(void *args);
void printEventPoll(unsigned long event);
void printEventEpoll(unsigned long event);

short server();
short server_multiProcess();
short server_multiThread(void);
short server_select(void);
short server_select_with_client_array();
short server_poll();
short server_epoll();

pthread_mutex_t lock;
static char *pass;


/*****************************************    Socket and SSL Communication     *************************************************/

SocketInfo getSocketInfo(void)
{

	// Fill the Socket Information here

	SocketInfo sockInfo = {0};

	strcpy(sockInfo.hostIP, "10.200.3.45");
	sockInfo.hostPort = 13001; 
	sockInfo.isSSL = _FALSE; 
	strcpy(sockInfo.certificate, "server.crt");
	strcpy(sockInfo.keyfile, "server_key.pem");
	strcpy(sockInfo.caCert, "ca.crt");
	strcpy(sockInfo.commonName, "unipay");
	strcpy(sockInfo.password, "");

	return sockInfo;
}


SOCKET Socket_Open( const char *host, unsigned short port )
{
	
	SOCKET sock;
	struct sockaddr_in host_address;
	char hostIP[51] = {'\0'};

	strcpy(hostIP, host);

#ifdef WINDOWS

	WSADATA WinSockInitInfo;
	WSAStartup(MAKEWORD(2,0),&WinSockInitInfo);

#endif

	File_Log("Socket_Open() : %s : %d", host, port);

	host_address.sin_family = AF_INET;
	host_address.sin_addr.s_addr = inet_addr(host); 
	host_address.sin_port = htons(port);

	if ( host_address.sin_addr.s_addr == INADDR_NONE)
	{
		// If host address is Domain Name instead of IP address

		struct hostent *tempAddr = gethostbyname(host);
		if ( tempAddr == NULL)
		{
			File_Log("gethostbyname() fail, %s : %c", host, port);
			goto ERRORHANDLER;
		}
		else
		{
			strcpy(hostIP, (char *)inet_ntoa( *( struct in_addr*)( tempAddr -> h_addr_list[0])));	
			host_address.sin_addr.s_addr = inet_addr(hostIP);
		}	
	}

	File_Log("Socket Connecting --> %s:%d", hostIP, port);


	sock = socket( AF_INET, SOCK_STREAM, 0 );

	if ( sock == SOCKET_ERROR)
	{
		File_Log("Socket Initialize Error");
		goto ERRORHANDLER;
	}


	if ( connect( sock, (struct sockaddr *)&host_address, sizeof(host_address)) == SOCKET_ERROR) 
	{
		
		File_Log("Socket Connection Failed"); 
		goto ERRORHANDLER;
	}

	File_Log("Socket Connected --> %s:%d", hostIP, port);

	return sock;

ERRORHANDLER:

#ifdef WINDOWS
	WSACleanup();
#endif

	return SOCKET_ERROR;
}

void Socket_Close(SOCKET s)
{

	//File_Log("Socket Closing");

#ifdef WINDOWS
	closesocket(s);
	WSACleanup();
#else
	close(s);
#endif
	
	//File_Log("Socket Closed");

}

BOOLEAN set_socket_rcv_timeout(SOCKET sock, int secs)
{
	short ret = 0;
	struct timeval tv;
	tv.tv_sec = secs;     // 30 sec timeout
	tv.tv_usec = 0;

	ret = setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *) &tv, sizeof(tv));

	if ( ret )
		File_Log("ERROR : SET SOCKET RECEIVE TIMEOUT");

	return ret ? _FALSE : _TRUE;

}

BOOLEAN set_socket_nonblock(SOCKET sd)
{
	int flags;

	flags = fcntl( sd, F_GETFL, 0);
	if ( flags == -1)
	{
		File_Log("ERROR : fcntl F_GETFL");
		return _FALSE;
	}

	flags |= O_NONBLOCK;
	
	flags = fcntl( sd, F_SETFL, flags);
	if ( flags == -1)
	{
		File_Log("ERROR : fcntl F_SETFL");
		return _FALSE;
	}

	return _TRUE;

}

int Socket_Read(SOCKET sock, SSL *ssl, unsigned char *buffer, int maxLen)
{
	int bytesRead = 0;
	
	//File_Log("Socket Receiving...");

	bytesRead = ssl ?  SSL_read(ssl, buffer, maxLen) : recv(sock, (char *)buffer, maxLen, 0) ;

	return bytesRead;
}

int Socket_Write(SOCKET sock, SSL *ssl, unsigned char *buffer, int len)
{
	int bytesWritten = 0;

	//File_Log("Socket Sending...");

	bytesWritten = ssl ?  SSL_write(ssl, buffer, len) : send(sock, (char *)buffer, len, 0) ;

	return bytesWritten;
}

void Socket_Destroy(SocketHandle *Socket)
{

	//File_Log("Socket_Destroy() -> Destroying Socket...");

	if ( Socket->ssl)
		SSL_free(Socket->ssl);

	Socket_Close(Socket->sock);

	if ( Socket->ctx)
		destroy_ctx(Socket->ctx);
	
	//File_Log("Socket_Destroy() -> Destroyed");

}


SSL_CTX *initialize_ctx (char *certfile, char *keyfile, char *CA, char *password)
{
	int ret = 0;
	SSL_METHOD *meth;
	SSL_CTX *ctx;



	//File_Log("SSL CTX Initializing");

	if (!bio_err) 
	{
		/* Global system initialization*/
		ret = SSL_library_init ();
		if ( ret != 1){
			File_Log("SSL LIB INIT FAIL");
			return NULL;
		}
		SSL_load_error_strings ();
		/* An error write context */
		bio_err = BIO_new_fp (stderr, BIO_NOCLOSE);
	}

	/* Create our context*/
	meth = (SSL_METHOD *)SSLv23_method ();
	//meth = (SSL_METHOD *)TLS_method ();
	ctx = SSL_CTX_new (meth);
	
	
	/* Load our keys and certificates*/
	ret = SSL_CTX_use_certificate_file ( ctx, certfile, SSL_FILETYPE_PEM);
	if ( ret != 1)
	{
		File_Log("CERTIFICATE FILE LOAD FAIL");
		destroy_ctx(ctx);
		return NULL;
	}
	pass = password;
	SSL_CTX_set_default_passwd_cb (ctx, password_cb);
	
	
	ret = SSL_CTX_use_PrivateKey_file (ctx, keyfile, SSL_FILETYPE_PEM);
	if ( ret != 1)
	{
		File_Log("KEY FILE LOAD FAIL");
		destroy_ctx(ctx);
		return NULL;
	}
	
	/* Load the CAs we trust*/
	SSL_CTX_load_verify_locations (ctx, CA,0);
	if ( ret != 1)
	{
		File_Log("CA LOAD FAIL");
		destroy_ctx(ctx);
		return NULL;
	}

	SSL_CTX_set_verify_depth (ctx,2);
	SSL_CTX_set_verify (ctx, SSL_VERIFY_PEER, verify_callback);

	//File_Log("SSL CTX Initialization OK");

	return ctx;
}

void destroy_ctx(SSL_CTX *ctx)
{
	SSL_CTX_free(ctx);
}


int verify_callback (int ok, X509_STORE_CTX *store)
{
	char data[256];
	
	if (!ok)
	{
		
		X509 *cert = X509_STORE_CTX_get_current_cert(store);
		int depth = X509_STORE_CTX_get_error_depth(store);
		int err = X509_STORE_CTX_get_error(store);
		File_Log("Error with certificate at depth: %i", depth);
		X509_NAME_oneline(X509_get_issuer_name(cert), data, 256);
		File_Log("Issuer = %s", data);
		X509_NAME_oneline(X509_get_subject_name(cert), data, 256);
		File_Log("Subject = %s", data);
		File_Log("Err %i:%s", err, X509_verify_cert_error_string(err) );
	}

	return ok;
}


/* Check that the common name matches the host name*/
void check_cert_chain (SSL *ssl, char *host)
{
	X509 *peer;
	char peer_CN[256];

	
	if ( SSL_get_verify_result (ssl) != X509_V_OK )
		File_Log("Certificate doesn't verify");
	/*Check the common name*/
	peer = SSL_get_peer_certificate (ssl);
	X509_NAME_get_text_by_NID (X509_get_subject_name (peer), NID_commonName, peer_CN, 256);
	if (strcmp(peer_CN, host)){
		File_Log("Common name not matched");
		File_Log("%s : %s", peer_CN, host);
	}
	
}

static int password_cb(char *buf,int num,int rwflag,void *userdata)
{
	if(num < (signed int)strlen(pass)+1)
	{
		return(0);
	}

	strcpy(buf, pass);

	return(strlen(pass));
}




void load_dh_params(SSL_CTX *ctx, char *file)
{
	DH *ret = 0;
	BIO *bio;

	bio = BIO_new_file(file, "r");
	ret = PEM_read_bio_DHparams(bio, NULL, NULL, NULL);
	BIO_free(bio);
	SSL_CTX_set_tmp_dh(ctx, ret);
}

void generate_eph_rsa_key(SSL_CTX *ctx)
{
	RSA *rsa;
	//RSA *rsa = RSA_new();
	//BIGNUM *bn = BN_new();
    //BN_set_word(bn, RSA_F4);

	rsa = RSA_generate_key(512, RSA_F4, NULL, NULL);
	//RSA_generate_key_ex(rsa, 2048, bn, NULL );
	SSL_CTX_set_tmp_rsa(ctx, rsa);
	RSA_free(rsa);
}

int File_Log(const char* format, ...)
{
	int ret=0;
	int month = 0;
	int fp = 0;
	char buffer[15001] = {'\0'};
	char logfile[30] = {'\0'};	

	va_list args;

	month = Time_Get(buffer);

	va_start(args, format);
	vsprintf (&buffer[strlen(buffer)], format, args );
	va_end(args);	

	sprintf(logfile, "log_s_%02d.txt", month);

	printf("%s", buffer);


	fp = open (logfile, O_WRONLY|O_APPEND|O_CREAT, S_IWRITE | S_IREAD); // S_IWRITE - Open or Create with write permission 
	
	if(fp<0)
	{
		switch(errno)
		{
			case EACCES: //return _ERR_FILE_ACCESS; 
			case EMFILE: //return _ERR_FILE_TOO_MANY_HANDLERS;
			default: //return _ERR_FILE_UNKNOWN;
				return -1;
		}
	}

	ret = write(fp, (const char *)buffer,strlen(buffer));

	close(fp);

	return ret;
	

}


int Time_Get(char *outField)
{

	time_t t = time(0);
	struct tm* lt = localtime(&t);
 	char time_str[128] = {0};
	
	sprintf(time_str, "\n%02d%02d%02d: %02d/%02d/%02d: => ", lt->tm_hour, lt->tm_min, lt->tm_sec, lt->tm_mon + 1, lt->tm_mday, lt->tm_year - 100);

	strncat(outField, time_str, strlen(time_str));
	return lt->tm_mon + 1;
}



void printEventPoll(unsigned long event)
{
	if ( event & POLLIN )
		File_Log("EVENT : POLLIN");
	
	if ( event & POLLOUT)
		File_Log("EVENT : POLLOUT");

	if ( event & POLLRDHUP )
		File_Log("EVENT : POLLRDHUP");

	if ( event & POLLHUP )
		File_Log("EVENT : POLLHUP");

	if ( event & POLLERR )
		File_Log("EVENT : POLLERR");

	if ( event & POLLPRI )
		File_Log("EVENT & POLLPRI");

	if ( event & POLLNVAL )
		File_Log("EVENT : POLLNVAL");

	if ( event & POLLRDNORM )
		File_Log("EVENT : POLLRDNORM");

	if ( event & POLLRDBAND )
		File_Log("EVENT & POLLRDBAND");

	if ( event & POLLWRNORM )
		File_Log("EVENT : POLLWRNORM");

	if ( event & POLLWRBAND)
		File_Log("EVENT : POLLWRBAND");

	if ( event == 0)
		File_Log("EVENT : NOTHING OCCURED");
}


void printEventEpoll(unsigned long event)
{
	if ( event & EPOLLIN )
		File_Log("EVENT : EPOLLIN");
	
	if ( event & EPOLLOUT)
		File_Log("EVENT : EPOLLOUT");

	if ( event & EPOLLRDHUP )
		File_Log("EVENT : EPOLLRDHUP");

	if ( event & EPOLLHUP )
		File_Log("EVENT : EPOLLHUP");

	if ( event & EPOLLERR )
		File_Log("EVENT : EPOLLERR");

	if ( event & EPOLLPRI )
		File_Log("EVENT & EPOLLPRI");

	if ( event & EPOLLONESHOT )
		File_Log("EVENT : EPOLLONESHOT");
}


SOCKET Socket_Init(const char *host, unsigned short port, unsigned short maxQueue)
{

	int ret = 0;
 	SOCKET server = 0;
	struct sockaddr_in server_address;
	int reuse = _TRUE;
	
#ifdef WINDOWS

	WSADATA WinSockInitInfo;
	WSAStartup(MAKEWORD(2,0),&WinSockInitInfo);

#endif


	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = inet_addr(host); 
	server_address.sin_port = htons(port);

	File_Log("Server Socket Initializing");


	server = socket( AF_INET, SOCK_STREAM, 0 );
	if ( server == SOCKET_ERROR)
	{
		File_Log("Socket Initialize Error");
		goto ERRORHANDLER;
	}

	setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

	
	// bind server address to socket

	ret = bind(server, (struct sockaddr *)&server_address, sizeof(server_address));
	if ( ret == SOCKET_ERROR)
	{
		File_Log("bind() failed %d", ret);
		goto ERRORHANDLER;
	}
	File_Log("bind OK");



	// listen on the socket, with 5 max connection requests queued

	ret = listen(server, maxQueue);
	if ( ret == SOCKET_ERROR)
	{
		File_Log("listen() failed %d", ret);
		goto ERRORHANDLER;
	}
	File_Log("listen OK");

	return server;

ERRORHANDLER:

#ifdef WINDOWS
	WSACleanup();
#endif

	return SOCKET_ERROR;

}


SocketHandle Socket_ServerInit(void)
{
	SocketHandle Server= {0};
	
	SocketInfo sockInfo = getSocketInfo();

	Server.sock = Socket_Init(sockInfo.hostIP, sockInfo.hostPort, 1024);
	if ( Server.sock == SOCKET_ERROR )
		return Server;


	if ( sockInfo.isSSL )
	{
		Server.ctx = initialize_ctx(sockInfo.certificate, sockInfo.keyfile, sockInfo.caCert, sockInfo.password);
		if ( Server.ctx == NULL)
		{
			Socket_Close(Server.sock);
			return Server;
		}

		//load_dh_params(Server.ctx, "DHFILE");
		generate_eph_rsa_key(Server.ctx);

	}

	Server.isConnected = _TRUE;

	return Server;
	
}


void *handleClient_mThread(void *args)
{

	int ret = 0;
	ClientContext clientContext = *(ClientContext *)args;
	
	ret = pthread_mutex_unlock(&lock);
	if ( ret != 0){
		File_Log("ERROR : Mutex Unlock Failed");
	}
	
	struct sockaddr_in address;
	socklen_t addr_len = sizeof(address);

	static int counter ;
	BIO *sbio = NULL;
	SSL *ssl = NULL;
	unsigned char buffer[128] = {0};
	int count = ++counter;
	SOCKET peer_sock = clientContext.clientSock;

	getpeername(peer_sock, (struct sockaddr*)&address, (socklen_t *)&addr_len);
	File_Log("SD %4d => %s:%d => Client %d => CONNECT", peer_sock, inet_ntoa(address.sin_addr), ntohs(address.sin_port), count);


	if ( clientContext.sockInfo->isSSL )
	{
		sbio = BIO_new_socket(peer_sock, BIO_NOCLOSE);
		ssl = SSL_new(clientContext.server->ctx);
		if ( ssl == NULL)
		{
			Socket_Close(peer_sock);
			File_Log("SD %4d => %s:%d => Client %d => DISCONNECT ERROR SSL_new()", peer_sock, inet_ntoa(address.sin_addr), ntohs(address.sin_port), count);
			return (void *)"FAIL";
		}

		SSL_set_bio(ssl, sbio, sbio);
		ret = SSL_accept(ssl);
		check_cert_chain(ssl, clientContext.sockInfo->commonName);
	    	if ( ret < 0)
		{
			SSL_free(ssl);
			Socket_Close(peer_sock);
			File_Log("SD %4d => %s:%d => Client %d => DISCONNECT ERROR SSL_accept()", peer_sock, inet_ntoa(address.sin_addr), ntohs(address.sin_port), count);
			return (void *)"FAIL";
		}	
	}

	
	ret = Socket_Read (peer_sock, ssl, buffer, sizeof(buffer));
	File_Log("SD %4d => %s:%d => Client %d => RCV => %d => %s", peer_sock, inet_ntoa(address.sin_addr), ntohs(address.sin_port), count, ret, buffer);

	if ( ret > 0 )
	{
		ret = Socket_Write(peer_sock, ssl, buffer, ret); 
		File_Log("SD %4d => %s:%d => Client %d => SND => %d => %s", peer_sock, inet_ntoa(address.sin_addr), ntohs(address.sin_port), count, ret, (ret > 0) ? (char *)buffer : " ");
	}



	if ( ssl)
		SSL_free(ssl);

	Socket_Close(peer_sock);
	
	File_Log("SD %4d => %s:%d => Client %d => DISCONNECT", peer_sock, inet_ntoa(address.sin_addr), ntohs(address.sin_port), count);

	return (void *)"OK";


}


/* Signal handler to reap zombie processes */
static void wait_for_child(int sig)
{
    while (waitpid(-1, NULL, WNOHANG) > 0);
}


short server()
{
	#define MAX_QUEUE 1024 

	int i, ret, reuseAddr = _TRUE;
	SOCKET listener_sock, peer_sock;
	struct sockaddr_in address;
	socklen_t addr_len = sizeof(address);
	unsigned char buffer[1024] = {0};

	SocketInfo sockInfo = getSocketInfo();
	
	listener_sock = socket(AF_INET, SOCK_STREAM, 0);
	if ( listener_sock == SOCKET_ERROR)
	{
		File_Log("Socket Initialize Error");
		exit(EXIT_FAILURE);
	}

	// Set listener socket to allow multiple connections	
	ret = setsockopt(listener_sock, SOL_SOCKET, SO_REUSEADDR, &reuseAddr, sizeof(reuseAddr));

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr(sockInfo.hostIP); 
	address.sin_port = htons(sockInfo.hostPort);

	ret = bind(listener_sock, (struct sockaddr *)&address, sizeof(address));
	if ( ret < 0)
	{
		File_Log("Bind failed");
		exit(EXIT_FAILURE);
	}

	ret = listen(listener_sock, MAX_QUEUE);   
	if ( ret < 0 )
	{
		File_Log("listen() failed");
		exit(EXIT_FAILURE);
	}

	File_Log("SOCK ID : %d", listener_sock);



	while(_TRUE)
	{

		File_Log("--------------------------- Listening ------------------");

		peer_sock = accept(listener_sock, (struct sockaddr *)&address, (socklen_t *)&addr_len);
		if ( peer_sock < 0)
		{
			File_Log("ERROR : accept()");
			exit(EXIT_FAILURE);
		}

		getpeername(peer_sock, (struct sockaddr*)&address, (socklen_t *)&addr_len);
		File_Log("SD %4d => %s:%d => CONNECT", peer_sock, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

		memset(buffer, 0, sizeof(buffer));
		set_socket_rcv_timeout(peer_sock, 30);

		ret = Socket_Read(peer_sock, NULL, buffer, sizeof(buffer));
		
		File_Log("SD %4d => %s:%d => RCV => %d => %s", peer_sock, inet_ntoa(address.sin_addr), ntohs(address.sin_port), ret, buffer);
		
		if ( ret > 0 )
		{
			ret = Socket_Write(peer_sock, NULL, buffer, strlen((char *)buffer));
			File_Log("SD %4d => %s:%d => SND => %d => %s", peer_sock, inet_ntoa(address.sin_addr), ntohs(address.sin_port), ret, buffer);
		}

		Socket_Close(peer_sock);

		File_Log("SD %4d => %s:%d => DISCONNECT", peer_sock, inet_ntoa(address.sin_addr), ntohs(address.sin_port));
	}

	return 0;

}	


short server_multiProcess()
{
	int ret = 0;
	SOCKET listener_sock, peer_sock ;
	SocketHandle Server = {0};
	unsigned char buffer[128] = {0};
	int pid;
	struct sockaddr_in address;
        socklen_t addr_len = sizeof(address);
	struct sigaction sa;

	Server = Socket_ServerInit();
	if ( !Server.isConnected)
		return 0;

	/* Set up the signal handler */
	sa.sa_handler = wait_for_child;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		File_Log("ERROR : sigaction");
		return 1;
	}

	listener_sock = Server.sock;


	
	while ( _TRUE)
	{

		File_Log("----------------- Listening -------------------------");
		peer_sock  = accept(listener_sock, NULL, NULL);
		if ( peer_sock == SOCKET_ERROR)
		{
			File_Log("ERROR : accept() failed %d");
			exit(EXIT_FAILURE);
		}
		

		pid = fork();

		if ( pid == 0) // Child Process
		{
			Socket_Close(listener_sock);  // close the duplicate listener_sock


			getpeername(peer_sock, (struct sockaddr*)&address, (socklen_t *)&addr_len);
			File_Log("SD %4d => %s:%d => CONNECT", peer_sock, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

			memset(buffer, 0, sizeof(buffer));
			set_socket_rcv_timeout(peer_sock, 25);

			ret = Socket_Read(peer_sock, NULL, buffer, sizeof(buffer));

			File_Log("SD %4d => %s:%d => RCV => %d => %s", peer_sock, inet_ntoa(address.sin_addr), ntohs(address.sin_port), ret, buffer);

			if ( ret > 0 )
			{
				ret = Socket_Write(peer_sock, NULL, buffer, strlen((char *)buffer));
				File_Log("SD %4d => %s:%d => SND => %d => %s", peer_sock, inet_ntoa(address.sin_addr), ntohs(address.sin_port), ret, ( ret > 0 ) ? (char *)buffer : " ");
			}

			Socket_Close(peer_sock);

			File_Log("SD %4d => %s:%d => DISCONNECT", peer_sock, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

			return 0;

		}
		else           // Parent process , close the duplicated peer_sock descriptor, go for accept() again for accept the next client
		{
			if ( pid == -1)
			{
				File_Log("ERROR : fork()");
				return -1;
			}

			Socket_Close(peer_sock);
		} 
	}

	return 0;
}


short server_multiThread(void)
{

	SocketHandle Server= {0};
	SOCKET client = 0;
	SocketInfo sockInfo = getSocketInfo();
	struct sockaddr client_address = {0};
	socklen_t client_address_len = sizeof(client_address);
	ClientContext clientContext = {0};
	pthread_t tid;
	short ret = 0;

	ret = pthread_mutex_init(&lock, NULL);
	if ( ret != 0)
	{
		File_Log("ERROR : Mutex Init Failed");
		return -1;
	}



	Server = Socket_ServerInit();
	if ( !Server.isConnected)
		return 0;


	clientContext.server = &Server;
	clientContext.sockInfo = &sockInfo;


	while(1)
	{
		ret = pthread_mutex_lock(&lock);
		if ( ret != 0){
			File_Log("ERROR : Mutex Lock Failed");
		}

		File_Log("----------------- Listening -------------------------");
		client = accept(Server.sock, &client_address, &client_address_len);
		if ( client == SOCKET_ERROR)
		{
			File_Log("ERROR : accept() failed %d");
			break;
		}

		clientContext.clientSock = client;


		ret = pthread_create(&tid, NULL, handleClient_mThread, (void *)&clientContext);
		if ( ret != 0){
			printf("ERROR : pthread_create() failed");
			break;
		}

	}

	
	ret = pthread_mutex_unlock(&lock);
	if ( ret != 0){
		File_Log("ERROR : Mutex Unlock Failed");
	}
	
	Socket_Destroy(&Server);

	return 0;
	
}



short server_select_with_client_array()
{
	int i, ret, addr_len, reuseAddr = _TRUE, max_clients = 1024;
	SOCKET listener_sock, peer_sock, max_sd, sd, client_sock[1024];
	struct sockaddr_in address;
	char buffer[1024] = {0};

	fd_set readfds;           // Set of socket descriptors

	SocketInfo sockInfo = getSocketInfo();

	listener_sock = socket(AF_INET, SOCK_STREAM, 0);
	if ( listener_sock == SOCKET_ERROR)
	{
		File_Log("Socket Initialize Error");
		exit(EXIT_FAILURE);
	}

	// Set listener socket to allow multiple connections	
	ret = setsockopt(listener_sock, SOL_SOCKET, SO_REUSEADDR, &reuseAddr, sizeof(reuseAddr));

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr(sockInfo.hostIP); 
	address.sin_port = htons(sockInfo.hostPort);

	ret = bind(listener_sock, (struct sockaddr *)&address, sizeof(address));
	if ( ret < 0)
	{
		File_Log("Bind failed");
		exit(EXIT_FAILURE);
	}

	ret = listen(listener_sock, 1024);
	if ( ret < 0 )
	{
		File_Log("listen() failed");
		exit(EXIT_FAILURE);
	}

	addr_len = sizeof(address);
	for( i = 0; i < max_clients; i++)
	{
		client_sock[i] = 0;
	}

	File_Log("SOCK ID : %d", listener_sock);

	while(_TRUE)
	{
		// Clear the socket set
		FD_ZERO(&readfds);               

		// Add listener socket to set
		FD_SET(listener_sock, &readfds);   
		max_sd = listener_sock;

		// add child sockets to set
		for( i = 0; i < max_clients; i++)
		{
			// socket descriptor
			sd = client_sock[i];     

			// If valid socket descriptor then add to read list
			if ( sd > 0 )
			{
				FD_SET(sd, &readfds);
				// highest file descriptor number, need it for the select function
				if ( sd > max_sd )
					max_sd = sd;
			}


		}

		ret = select(max_sd + 1, &readfds, NULL, NULL, NULL);
		if ( (ret < 0) && (errno != EINTR) )
		{
			File_Log("ERROR : select()");
		}


		// If something happened on the listener socket, then it's an incoming connection
		if ( FD_ISSET(listener_sock, &readfds) )
		{
			peer_sock = accept(listener_sock, (struct sockaddr *)&address, (socklen_t *)&addr_len);
			if ( peer_sock < 0)
			{
				File_Log("ERROR : accept()");
			}
			else
			{
				// Somebody connected, get its details and print
				getpeername(sd, (struct sockaddr*)&address, (socklen_t *)&addr_len);
				File_Log("%s:%d => CONNECT", inet_ntoa(address.sin_addr), ntohs(address.sin_port));
				
				
				File_Log("SOCK ID : %d", peer_sock);
				// add new socket to array of sockets
				for ( i = 0; i < max_clients; i++)
				{
					if ( client_sock[i] == 0)
					{
						client_sock[i] = peer_sock;
						break;
					}

				}
			}
		}


		// else its some IO operation on some other socket
		for ( i = 0; i < max_clients; i++)
		{
			sd = client_sock[i];

			if ( FD_ISSET(sd, &readfds) )
			{
				getpeername(sd, (struct sockaddr*)&address, (socklen_t *)&addr_len);

				memset(buffer, 0, sizeof(buffer));
				ret = recv(sd, buffer, 1024, 0);
				if ( ret == 0)  // check if it was for closing
				{
					// Somebody disconnected, get its details and print
					File_Log("%s:%d => DISCONNECT", inet_ntoa(address.sin_addr), ntohs(address.sin_port));

					// Close the socket and mark as 0 in the list for reuse
					close(sd);
					client_sock[i] = 0;
				}
				else  // Received something, echo it
				{
					File_Log("%s:%d => RCV : %d %s", inet_ntoa(address.sin_addr), ntohs(address.sin_port), ret, buffer);
					ret = (ret > 0) ? ret : 1;
					buffer[ret] = '\0';
					ret = send(sd, buffer, ret, 0);
					File_Log("%s:%d => SND : %d %s", inet_ntoa(address.sin_addr), ntohs(address.sin_port), ret, buffer);
				}
			}
		}
	}

	return 0;

}

short server_select()
{
	int i, ret, reuseAddr = _TRUE;
	SOCKET listener_sock, peer_sock, max_sd;
	struct sockaddr_in address;
	socklen_t addr_len = sizeof(address);
	unsigned char buffer[1024] = {0};

	fd_set readfds,tempfds;           // Set of socket descriptors

	SocketInfo sockInfo = getSocketInfo();

	listener_sock = socket(AF_INET, SOCK_STREAM, 0);
	if ( listener_sock == SOCKET_ERROR)
	{
		File_Log("Socket Initialize Error");
		exit(EXIT_FAILURE);
	}

	// Set listener socket to allow multiple connections	
	ret = setsockopt(listener_sock, SOL_SOCKET, SO_REUSEADDR, &reuseAddr, sizeof(reuseAddr));

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr(sockInfo.hostIP); 
	address.sin_port = htons(sockInfo.hostPort);

	ret = bind(listener_sock, (struct sockaddr *)&address, sizeof(address));
	if ( ret < 0)
	{
		File_Log("Bind failed");
		exit(EXIT_FAILURE);
	}

	ret = listen(listener_sock, FD_SETSIZE);   // select allows only 1024 connections which is defined as FD_SETSIZE
	if ( ret < 0 )
	{
		File_Log("listen() failed");
		exit(EXIT_FAILURE);
	}

	File_Log("SOCK ID : %d", listener_sock);
		
	// Clear the socket set
	FD_ZERO(&readfds);               

	// Add listener socket to set
	FD_SET(listener_sock, &readfds);   
	max_sd = listener_sock;

	while(_TRUE)
	{


		tempfds = readfds;   // copy file descriptors to a temp file descriptors, because select modifies the passed file descriptors

		ret = select(max_sd + 1, &tempfds, NULL, NULL, NULL);
		if ( (ret < 0) && (errno != EINTR) )
		{
			File_Log("ERROR : select()");
		}


		// If something happened on the listener socket, then it's an incoming connection
		if ( FD_ISSET(listener_sock, &tempfds) )
		{
			peer_sock = accept(listener_sock, (struct sockaddr *)&address, (socklen_t *)&addr_len);
			if ( peer_sock < 0)
			{
				File_Log("ERROR : accept()");
			}
			else
			{
				// Somebody connected, get its details and print
				getpeername(peer_sock, (struct sockaddr*)&address, (socklen_t *)&addr_len);
				set_socket_nonblock(peer_sock);

				File_Log("SD %4d => %s:%d => CONNECT", peer_sock, inet_ntoa(address.sin_addr), ntohs(address.sin_port));	
				
				// add new socket to array of sockets
				FD_SET(peer_sock, &readfds);
				FD_CLR(listener_sock, &tempfds);
				max_sd = (peer_sock > max_sd) ? peer_sock : max_sd;

			}
		}


		// else its some IO operation on some other socket
		for ( peer_sock = 0; peer_sock < max_sd+1; peer_sock++)
		{

			if ( FD_ISSET(peer_sock, &tempfds) )
			{
				getpeername(peer_sock, (struct sockaddr*)&address, (socklen_t *)&addr_len);

				memset(buffer, 0, sizeof(buffer));
				ret = Socket_Read(peer_sock, NULL, buffer, sizeof(buffer));
				if ( ret <= 0)  // check if it was for closing
				{
					// Somebody disconnected, get its details and print
					File_Log("SD %4d => %s:%d => DISCONNECT %s", peer_sock, inet_ntoa(address.sin_addr), ntohs(address.sin_port), (ret == 0) ? " " : "FOR RCV ERROR" );

					// Close the socket and mark as 0 in the list for reuse
					Socket_Close(peer_sock);
					FD_CLR(peer_sock, &readfds);
				}
				
				

				if ( ret > 0 )
				{
					File_Log("SD %4d => %s:%d => RCV => %d => %s", peer_sock, inet_ntoa(address.sin_addr), ntohs(address.sin_port), ret, buffer);
					ret = Socket_Write(peer_sock, NULL, buffer, strlen((char *)buffer));
					File_Log("SD %4d => %s:%d => SND => %d => %s", peer_sock, inet_ntoa(address.sin_addr), ntohs(address.sin_port), ret, buffer);
				}
			}
		}
	}

	return 0;

}

short server_poll()
{

	#define POLL_SIZE 2048

	int i, ret, reuseAddr = _TRUE, current_size, nfds = 1, on = 1 ;
	SOCKET listener_sock, peer_sock;
	struct sockaddr_in address;
	unsigned char buffer[1024] = {0};
	socklen_t addr_len = sizeof(address);
	BOOLEAN compress_fd_array;

	SocketInfo sockInfo = getSocketInfo();
	
	struct pollfd fds[POLL_SIZE];

	listener_sock = socket(AF_INET, SOCK_STREAM, 0);
	if ( listener_sock == SOCKET_ERROR)
	{
		File_Log("Socket Initialize Error");
		exit(EXIT_FAILURE);
	}

	// Set listener socket to allow multiple connections	
	ret = setsockopt(listener_sock, SOL_SOCKET, SO_REUSEADDR, &reuseAddr, sizeof(reuseAddr));

	ret = ioctl(listener_sock, FIONBIO, (char *)&on);
	if (ret < 0)
	{
		File_Log("ioctl() failed");
		Socket_Close(listener_sock);
		exit(EXIT_FAILURE);
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr(sockInfo.hostIP); 
	address.sin_port = htons(sockInfo.hostPort);

	ret = bind(listener_sock, (struct sockaddr *)&address, sizeof(address));
	if ( ret < 0)
	{
		File_Log("Bind failed");
		exit(EXIT_FAILURE);
	}

	ret = listen(listener_sock, POLL_SIZE);
	if ( ret < 0 )
	{
		File_Log("listen() failed");
		exit(EXIT_FAILURE);
	}

	File_Log("SOCK ID : %d", listener_sock);


	
	// Setup initial listening socket
	
	memset(fds, 0, sizeof(fds));
	fds[0].fd = listener_sock;
	fds[0].events = POLLIN;	

		


	while(_TRUE)
	{

		//File_Log("--------------------------- Listening ------------------");

		ret = poll(fds, nfds, -1);
		if ( ret < 0 )
		{
			File_Log("ERROR : POLL");
			exit(EXIT_FAILURE);
		}


		current_size = nfds;
		compress_fd_array = _FALSE;



		if ( fds[0].revents )                // bits set on listener socket's returned event
		{
			//Accept all incoming connections that are queued up on the listening socket before we loop back and call poll again

		       do
		       {
			       peer_sock = accept(listener_sock, (struct sockaddr *)&address, (socklen_t *)&addr_len);
			       if ( peer_sock < 0)
			       {
				       if ( current_size == nfds )
					       File_Log("ERROR : accept()");
			       }
			       else
			       {
				       File_Log("SD %4d => %s:%d => CONNECT", peer_sock, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

				       set_socket_nonblock(peer_sock);

				       fds[nfds].events  = POLLIN | POLLRDHUP ;
				       fds[nfds].fd      = peer_sock;
				       nfds++;

				}

		       }while( peer_sock > 0);	       
		}
		fds[0].revents = 0;


		
		

		for ( i = 1; i < current_size; i++)
		{

			if ( fds[i].revents == 0)
				continue;

			peer_sock = fds[i].fd;
			getpeername(peer_sock, (struct sockaddr*)&address, (socklen_t *)&addr_len);


			if ( fds[i].revents & POLLRDHUP )      // Remote host closed connection
			{

				fds[i].fd = -1;
				fds[i].revents = 0;
				compress_fd_array = _TRUE;
				Socket_Close(peer_sock);
				File_Log("SD %4d => %s:%d => DISCONNECT", peer_sock, inet_ntoa(address.sin_addr), ntohs(address.sin_port));
				
			}
			else if ( fds[i].revents & POLLIN )
			{
				memset(buffer, 0, sizeof(buffer));
				ret = Socket_Read(peer_sock, NULL, buffer, sizeof(buffer));
				File_Log("SD %4d => %s:%d => RCV => %d => %s", peer_sock, inet_ntoa(address.sin_addr), ntohs(address.sin_port), ret, buffer);
				if ( ret > 0 )
				{
					ret = Socket_Write(peer_sock, NULL, buffer, strlen((char *)buffer));
					File_Log("SD %4d => %s:%d => SND => %d => %s", peer_sock, inet_ntoa(address.sin_addr), ntohs(address.sin_port), ret, buffer);
				}
				else
				{
					fds[i].fd = -1;
					compress_fd_array = _TRUE;
					Socket_Close(peer_sock);
					File_Log("SD %4d => %s:%d => DISCONNECT %s", peer_sock, inet_ntoa(address.sin_addr), ntohs(address.sin_port), (ret == 0) ? " " : "FOR RCV ERROR" );
				}
				fds[i].revents = 0;

			}
			else         				// Error Case
			{
				printEventPoll(fds[i].revents);
				fds[i].fd = -1;
				fds[i].revents = 0;
				compress_fd_array = _TRUE;
				Socket_Close(peer_sock);
				File_Log("SD %4d => %s:%d => DISCONNECT FOR UNKNOWN ERROR", peer_sock, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

			}

		}



		if ( compress_fd_array )
		{
			int j = 0;

			for (i = 0; i < nfds; i++)
			{
				if (fds[i].fd == -1)
				{
					for(j = i; j < nfds; j++)
					{
						fds[j].fd = fds[j+1].fd;
					}
					nfds--;
				}
			}
		}

	}

	return 0;

}

short server_epoll()
{
	#define EPOLL_SIZE 2048
	#define MAX_EVENTS EPOLL_SIZE

	int i, ret, reuseAddr = _TRUE, event_count;
	SOCKET listener_sock, peer_sock;
	struct sockaddr_in address;
	unsigned char buffer[1024] = {0};
	socklen_t addr_len = sizeof(address);


	SocketInfo sockInfo = getSocketInfo();
	
	SOCKET epoll_fd;
	struct epoll_event ev;
	struct epoll_event events[EPOLL_SIZE];
	unsigned long event = 0;

	listener_sock = socket(AF_INET, SOCK_STREAM, 0);
	if ( listener_sock == SOCKET_ERROR)
	{
		File_Log("Socket Initialize Error");
		exit(EXIT_FAILURE);
	}

	// Set listener socket to allow multiple connections	
	ret = setsockopt(listener_sock, SOL_SOCKET, SO_REUSEADDR, &reuseAddr, sizeof(reuseAddr));

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr(sockInfo.hostIP); 
	address.sin_port = htons(sockInfo.hostPort);

	ret = bind(listener_sock, (struct sockaddr *)&address, sizeof(address));
	if ( ret < 0)
	{
		File_Log("Bind failed");
		exit(EXIT_FAILURE);
	}

	ret = listen(listener_sock, EPOLL_SIZE);
	if ( ret < 0 )
	{
		File_Log("listen() failed");
		exit(EXIT_FAILURE);
	}

	File_Log("SOCK ID : %d", listener_sock);


	
	epoll_fd = epoll_create(EPOLL_SIZE);
	if ( epoll_fd == SOCKET_ERROR)
	{
		File_Log("ERROR : EPOLL CREATE");
		exit(EXIT_FAILURE);
	}

	ev.events = EPOLLIN;
	ev.data.fd = listener_sock;

	ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listener_sock, &ev);
	if ( ret == SOCKET_ERROR )
	{
		File_Log("ERROR : EPOLL CTL ADD");
		exit(EXIT_FAILURE);
	}

		


	while(_TRUE)
	{

		//File_Log("--------------------------- Listening ------------------");

		event_count = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
		if ( event_count == -1 )
		{
			File_Log("ERROR : EPOLL WAIT");
			exit(EXIT_FAILURE);
		}

		for ( i =0; i < event_count; i++)
		{
			event = events[i].events;
			peer_sock = events[i].data.fd;
			getpeername(peer_sock, (struct sockaddr*)&address, (socklen_t *)&addr_len);
			//printEventEpoll(event);

			if ( peer_sock == listener_sock )
			{
			
				peer_sock = accept(listener_sock, (struct sockaddr *)&address, (socklen_t *)&addr_len);
				if ( peer_sock < 0)
				{
					File_Log("ERROR : accept()");
				}
				else
				{
					File_Log("SD %4d => %s:%d => CONNECT", peer_sock, inet_ntoa(address.sin_addr), ntohs(address.sin_port));
					
					set_socket_nonblock(peer_sock);

					ev.events  = EPOLLIN | EPOLLET | EPOLLRDHUP | EPOLLHUP ;
					ev.data.fd = peer_sock;
	
					ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, peer_sock, &ev);
					if ( ret == SOCKET_ERROR )
					{
						File_Log("ERROR : EPOLL CTL ADD");
					}

				}

			}
			else if ( (event & EPOLLHUP) || (event & EPOLLRDHUP) || (event & EPOLLERR) )
			{
					File_Log("SD %4d => %s:%d => DISCONNECT", peer_sock, inet_ntoa(address.sin_addr), ntohs(address.sin_port));
					Socket_Close(peer_sock);

			}
			else if ( event & EPOLLIN )
			{
				memset(buffer, 0, sizeof(buffer));
				ret = Socket_Read(peer_sock, NULL, buffer, sizeof(buffer));
				File_Log("SD %4d => %s:%d => RCV => %d => %s", peer_sock, inet_ntoa(address.sin_addr), ntohs(address.sin_port), ret, buffer);
				if ( ret > 0 )
				{
					ret = Socket_Write(peer_sock, NULL, buffer, strlen((char *)buffer));
					File_Log("SD %4d => %s:%d => SND => %d => %s", peer_sock, inet_ntoa(address.sin_addr), ntohs(address.sin_port), ret, buffer);
				}

			}
			else
			{
					File_Log("%s:%d => UNCAPTURED EVENT", inet_ntoa(address.sin_addr), ntohs(address.sin_port));
			}
		}

	}


	return 0;

}	


int main()
{
	int choice = 0;
	
	printf("\n\n\n---------------- Choose the server you want to run -------------------------\n");
	printf("\n 1. Server");
	printf("\n 2. Server-Multi-Process");
	printf("\n 3. Server-Multi-Thread");
	printf("\n 4. Server-Select");
	printf("\n 5. Server-Poll");
	printf("\n 6. Server-Epoll");
	
	printf("\n\n Enter your choice : ");
	scanf("%d", &choice);

	switch(choice)
	{
		case 1: server(); break;
		case 2: server_multiProcess(); break;
		case 3: server_multiThread(); break;
		case 4: server_select(); break;
		case 5: server_poll(); break;
		case 6: server_epoll(); break;
		default: printf("\n Sorry !!!! , You have entered in invalid choice\n"); break;
	}	
	
	return 0;
	
}



