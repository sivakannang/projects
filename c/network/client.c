/*
 *
   

    cat /proc/sys/net/core/wmem_max
    4194304
    max ( kernal/system wide memory max )

    cat /proc/sys/net/core/wmem_default
    212992
    default ( kernal/system wide memory default )

    cat /proc/sys/net/ipv4/tcp_wmem
    4096    16384   4194304
    min     default max
    ( tcp autotuning / auto scale - tcp wide min, default, max memory )

    ss -nto
    netstat -tn

    siva@sivakannan:/mnt/d/repo/projects/c/network$ ss -nto
    State          Recv-Q          Send-Q                    Local Address:Port                    Peer Address:Port          Process
    ESTAB          0               3104640                       127.0.0.1:55536                      127.0.0.1:5000           timer:(persist,37sec,0)
    ESTAB          50224           0                             127.0.0.1:5000                       127.0.0.1:55536
    siva@sivakannan:/mnt/d/repo/projects/c/network$


    siva@sivakannan:/mnt/d/repo/projects/c/network$ netstat -tn
    Active Internet connections (w/o servers)
    Proto Recv-Q Send-Q Local Address           Foreign Address         State
    tcp        0 3104640 127.0.0.1:55536         127.0.0.1:5000          ESTABLISHED
    tcp    48160      0 127.0.0.1:5000          127.0.0.1:55536         ESTABLISHED
    siva@sivakannan:/mnt/d/repo/projects/c/network$

*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>

volatile sig_atomic_t stop = 0;

void handle_sigint(int sig) {
    stop = 1;
}

int main() {
    // No SIGPIPE handling - crashes if it happens!

    signal(SIGINT, handle_sigint);
    // Ignore SIGPIPE to prevent crashing on broken pipes
    signal(SIGPIPE, SIG_IGN);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    // Enable TCP_NODELAY
    int optval = 1;
    if (setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval)) < 0) {
        perror("setsockopt TCP_NODELAY");
    }

    // Enable TCP keepalive
    optval = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval)) < 0) {
        perror("setsockopt SO_KEEPALIVE");
    }

    // Set keepalive idle time
    optval = 5;
    if (setsockopt(sockfd, IPPROTO_TCP, TCP_KEEPIDLE, &optval, sizeof(optval)) < 0) {
        perror("setsockopt TCP_KEEPIDLE");
    }

    // Set keepalive interval
    optval = 2;
    if (setsockopt(sockfd, IPPROTO_TCP, TCP_KEEPINTVL, &optval, sizeof(optval)) < 0) {
        perror("setsockopt TCP_KEEPINTVL");
    }

    // Set keepalive probe count
    optval = 9;
    if (setsockopt(sockfd, IPPROTO_TCP, TCP_KEEPCNT, &optval, sizeof(optval)) < 0) {
        perror("setsockopt TCP_KEEPCNT");
    }

    // Set SO_SNDTIMEO = 10 sec
    struct timeval timeout;
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0) {
        perror("setsockopt SO_SNDTIMEO");
    }

    // Set SO_SNDBUF = 4 MB
    /*optval = 5 * 1024 * 1024;
    if (setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &optval, sizeof(optval)) < 0) {
        perror("setsockopt SO_SNDBUF");
    }*/

    int user_timeout = 0;
    socklen_t len = sizeof(user_timeout);
    if (getsockopt(sockfd, IPPROTO_TCP, TCP_USER_TIMEOUT, &user_timeout, &len) == 0) {
	    printf("TCP_USER_TIMEOUT = %d ms\n", user_timeout);
    }

    user_timeout = 10000; // 5 seconds
    setsockopt(sockfd, IPPROTO_TCP, TCP_USER_TIMEOUT, &user_timeout, sizeof(user_timeout));
    user_timeout = 0;
    if (getsockopt(sockfd, IPPROTO_TCP, TCP_USER_TIMEOUT, &user_timeout, &len) == 0) {
	    printf("TCP_USER_TIMEOUT = %d ms\n", user_timeout);
    }


    // Prepare server address
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(5000);
    inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);
    inet_pton(AF_INET, "192.168.1.9", &servaddr.sin_addr);

    // connect() first in blocking mode
    int ret = connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    if (ret < 0) {
        perror("connect");
        close(sockfd);
        return 1;
    }
    printf("Connected successfully.\n");

    // Now set O_NONBLOCK after connect
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags < 0) {
        perror("fcntl get");
        close(sockfd);
        return 1;
    }
    if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) < 0) {
        perror("fcntl set");
        close(sockfd);
        return 1;
    }
    printf("Socket set to non-blocking mode.\n");

    int counter = 1;
    char buffer[516];
    int prev_sndbuf_size = 0;

    while (!stop) {
        int sndbuf_size = 0;
        socklen_t optlen = sizeof(sndbuf_size);
        if (getsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &sndbuf_size, &optlen) == 0) {
            if (sndbuf_size != prev_sndbuf_size) {
                printf("SO_SNDBUF = %d bytes (counter = %d)\n", sndbuf_size, counter);
                prev_sndbuf_size = sndbuf_size;
            }
        } else {
            perror("getsockopt SO_SNDBUF");
        }

        // Prepare payload
        memset(buffer, 0, sizeof(buffer));
        snprintf(buffer, sizeof(buffer), "%d", counter);

        size_t total_sent = 0;
        while (total_sent < sizeof(buffer)) {
            ssize_t n = send(sockfd, buffer + total_sent, sizeof(buffer) - total_sent, MSG_DONTWAIT);
            //ssize_t n = send(sockfd, buffer + total_sent, sizeof(buffer) - total_sent, MSG_DONTWAIT | MSG_NOSIGNAL);
	     
            if (n < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
		    getsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &sndbuf_size, &optlen);
                    printf("Send buffer full, try again later < counter : %d >, <SO_SNDBUF = %d> \n", counter, sndbuf_size);
		    //usleep(10 * 1000);
		    sleep(1);
                    break;
                } else if ( errno == EPIPE ) {
                    printf("broken pipe detected < counter : %d>\n", counter);
		    close(sockfd);
		    return 1;
		}else {
                    printf("send() error ret = %ld , errno = %d , errstr = %s , <counter : %d> \n", n, errno, strerror(errno), counter);
                    close(sockfd);
                    return 1;
                }
            } else {
                total_sent += n;
            }
        }

        if (total_sent == sizeof(buffer)) {
            //printf("Sent %zu bytes (counter = %d)\n", total_sent, counter);
            counter++;
        } else {
            printf("Partial send, sent only %zu bytes\n", total_sent);
        }

	//if ( counter % 10000 == 0 ) {
		printf("counter is %d\n", counter );
	//}
	
	if ( counter % 10 == 0 )
	{
		ssize_t nRcvdBytes = recv(sockfd, buffer , sizeof(buffer), 0);
		printf("recv() ret = %ld , errno = %d , errstr = %s", nRcvdBytes, errno, strerror(errno));
	}

        //usleep(10 * 1000); // sleep 1 ms
	sleep(1);
    }

    printf("Exiting client. <counter : %d>\n", counter);
    close(sockfd);
    return 0;
}
 
