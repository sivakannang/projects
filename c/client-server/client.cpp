/*
      g++ client.cpp -lssl -lcrypto -lpthread -o client
*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


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

int Time_Get(char *outField);
int File_Log(const char* format, ...);
BOOLEAN set_socket_nonblock(SOCKET sd);
BOOLEAN set_socket_rcv_timeout(SOCKET sock, int secs);

SocketHandle Socket_ClientInit(void);
void *client(void *args);

short client_server();
short client_server_multiThread();
short client_server_polling();

static char *pass;


/*****************************************    Socket and SSL Communication     *************************************************/

SocketInfo getSocketInfo(void)
{

	// Fill the Socket Information here

	SocketInfo sockInfo = {0};

	strcpy(sockInfo.hostIP, "127.0.0.1");
	sockInfo.hostPort = 8888; 
	sockInfo.isSSL = _FALSE; 
	strcpy(sockInfo.certificate, "client.crt");
	strcpy(sockInfo.keyfile, "client_key.pem");
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
	
	set_socket_rcv_timeout(sock, 30); // 30 secs

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

	sprintf(logfile, "log_c_%02d.txt", month);

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



SocketHandle Socket_ClientInit(void)
{
	BIO *sbio;

	SocketHandle Client= {0};
	SocketInfo sockInfo = getSocketInfo();


	Client.sock = Socket_Open(sockInfo.hostIP, sockInfo.hostPort);
	if ( Client.sock == SOCKET_ERROR)
		return Client;


	if ( sockInfo.isSSL)
	{

		Client.ctx = initialize_ctx(sockInfo.certificate, sockInfo.keyfile, sockInfo.caCert, sockInfo.password);
		if ( Client.ctx == NULL)
		{
			Socket_Close(Client.sock);
			return Client;
		}

		File_Log("SSL Connecting");

		Client.ssl = SSL_new (Client.ctx);
		sbio = BIO_new_socket (Client.sock, BIO_NOCLOSE);
		SSL_set_bio (Client.ssl, sbio, sbio);

		if ( SSL_connect (Client.ssl) < 0){
			Socket_Close(Client.sock);
			destroy_ctx(Client.ctx);
			File_Log("SSL Connection Error");
			return Client;
		}

		check_cert_chain (Client.ssl, sockInfo.commonName);
	}

	Client.isConnected = _TRUE;

	return Client;

}


void *client(void *args)
{
	
	short ret = 0;
	unsigned char buffer[101] = {0};
	SocketHandle Client= {0};

	sprintf((char *)buffer, "Hello Server.... I'm from thread number %d", *(int *)args);

	Client = Socket_ClientInit();
	if ( !Client.isConnected){
		File_Log("Thread %d : Socket Connect Failed", *(int *)args);
		return (void *)"FAIL";
	}


	File_Log("Thread %d => SOCKET CONNECTED", *(int *)args);
	ret = Socket_Write(Client.sock, Client.ssl, buffer, strlen((char *)buffer));
	File_Log("Thread %d => SND => %d => %s", *(int *)args, ret, (ret > 0) ? (char *)buffer : " ");

	memset(buffer, 0, sizeof(buffer));
	ret = Socket_Read (Client.sock, Client.ssl, buffer, sizeof(buffer));
	File_Log("Thread %d => RCV => %d => %s", *(int *)args, ret, (char *)buffer);

	Socket_Destroy(&Client);
	File_Log("Thread %d => SOCKET DISCONNECT", *(int *)args);

	return (void *)"SUCCESS";
	
}




short client_server()
{

	#define MAX_ATTEMPT 1024

	int ret = 0, i = 0;
	SOCKET sock = 0;
	SocketInfo sockInfo = getSocketInfo();
	unsigned char buffer[1024] = {0};

	for ( i = 0; i < MAX_ATTEMPT; i++)
	{
		memset(buffer, 0, sizeof(buffer));
		sprintf((char *)buffer, "I'm from client %d", i+1);

		sock = Socket_Open(sockInfo.hostIP, sockInfo.hostPort);
		if ( sock == SOCKET_ERROR )
		{
			File_Log("ERROR : Socket Open");
			exit(EXIT_FAILURE);
		}

		ret = Socket_Write(sock, NULL, buffer, strlen((char *)buffer));
                File_Log("CLIENT %d => SND => %d => %s", i, ret, buffer);

		memset(buffer, 0, sizeof(buffer));
		ret = Socket_Read(sock, NULL, buffer, sizeof(buffer));
                File_Log("CLIENT %d => RCV => %d => %s", i, ret, buffer);

		Socket_Close(sock);

		if ( ret < 0)
			break;
	}

	return 0;

	
}

short client_server_multiThread()
{

	#define MAX_THREADS 128 
	short ret = 0;
	void *status;
	int i = 0;
	int count = MAX_THREADS;
	pthread_t threadID[MAX_THREADS] = {0}; 
	int id[MAX_THREADS] = {0};
	unsigned char buffer[1024] = {0};

//start:

	for ( i = 0; i < count ; i++){
		
		id[i] = i;	
		
		ret = pthread_create(&threadID[i], NULL, client, (void *)&id[i]);
		
		if ( ret != 0){
			File_Log("Thread %d : Thread creation failed", i);
			count = i;
		}
		else{
			File_Log("Thread %d : Thread created", i);
		}
	}
	
	
	for ( i = 0; i < count ; i++){
	
		File_Log("Thread %d : thread joining", i);

		ret = pthread_join(threadID[i], NULL);
		if ( ret != 0)
			File_Log("Thread %d : thread join failed", i);
		else
			File_Log("Thread %d : thread joined", i);

	}

	//goto start;

	return 0;
}

short client_server_polling()
{

	#define MAX_CLIENTS 1000 

	int ret = 0, i = 0, count = MAX_CLIENTS;
	SocketInfo sockInfo = getSocketInfo();
	unsigned char buffer[1024] = {0};

	SOCKET sock[MAX_CLIENTS];

	for ( i = 0; i < count; i++)
	{
		sock[i] = Socket_Open(sockInfo.hostIP, sockInfo.hostPort);
		if ( sock[i] == SOCKET_ERROR )
		{
			File_Log("ERROR : Socket Open");
			break;	
		}
	}

	count = i;
	for ( i = 0; i < count; i++)
	{
		memset(buffer, 0, sizeof(buffer));
		sprintf((char *)buffer, "I'm from client %d", i+1);

		ret = Socket_Write(sock[i], NULL, buffer, strlen((char *)buffer));
                File_Log("CLIENT %d => SND => %d => %s", i, ret, buffer);
	
		memset(buffer, 0, sizeof(buffer));
		ret = Socket_Read(sock[i], NULL, buffer, sizeof(buffer));
                File_Log("CLIENT %d => RCV => %d => %s", i, ret, buffer);
	}
	
	for ( i = 0; i < count; i++)
	{
		Socket_Close(sock[i]);
	}

	return 0;

}


int main()
{

        int choice = 0;

        printf("\n\n\n---------------- Choose the client you want to run-------------------------\n");
        printf("\n 1. Client-Server");
        printf("\n 2. Client-Server-Multi-Process");
        printf("\n 3. Client-Server-Multi-Thread");
        printf("\n 4. Client-Server-Select");
        printf("\n 5. Client-Server-Poll");
        printf("\n 6. Client-Server-Epoll");

        printf("\n\n Enter your choice : ");
        scanf("%d", &choice);

        switch(choice)
        {
                case 1: client_server(); break;
                case 2: client_server_multiThread(); break;
                case 3: client_server_multiThread(); break;
                case 4: client_server_polling(); break;
                case 5: client_server_polling(); break;
                case 6: client_server_polling(); break;
                case 7: break;
                default: printf("\n Sorry !!!! , You have entered in invalid choice\n"); break;
        }

        return 0;

}


