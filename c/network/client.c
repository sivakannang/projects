/*
    gcc client.c -o client         ./client [server ip, default 127.0.0.1]

    Experiment: send fixed 516-byte messages on a nonblocking socket, to a server
    that reads slowly, and watch the TCP send buffer fill up.

    Socket buffer limits (example values, in bytes)
    cat /proc/sys/net/core/wmem_max        4194304               largest value setsockopt(SO_SNDBUF) can request
    cat /proc/sys/net/core/wmem_default    212992                default for non-TCP sockets
    cat /proc/sys/net/ipv4/tcp_wmem        4096 16384 4194304    TCP autotuning: min, default, max

    Setting SO_SNDBUF yourself turns autotuning off for that socket.

    Watch the queues while this runs. In this example the receiver's window is full,
    so the sender keeps probing it (the persist timer):

    $ ss -nto
    State   Recv-Q   Send-Q    Local Address:Port    Peer Address:Port
    ESTAB   0        3104640   127.0.0.1:55536       127.0.0.1:5000     timer:(persist,37sec,0)
    ESTAB   50224    0         127.0.0.1:5000        127.0.0.1:55536
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
    (void)sig;
    stop = 1;
}

int main(int argc, char *argv[]) {
    const char *server_ip = (argc > 1) ? argv[1] : "127.0.0.1";

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

    int user_timeout = 0;
    socklen_t len = sizeof(user_timeout);
    if (getsockopt(sockfd, IPPROTO_TCP, TCP_USER_TIMEOUT, &user_timeout, &len) == 0) {
	    printf("TCP_USER_TIMEOUT = %d ms\n", user_timeout);
    }

    // Fail the connection if sent data stays unacknowledged for this long
    user_timeout = 10000; // milliseconds
    if (setsockopt(sockfd, IPPROTO_TCP, TCP_USER_TIMEOUT, &user_timeout, sizeof(user_timeout)) < 0) {
        perror("setsockopt TCP_USER_TIMEOUT");
    }
    user_timeout = 0;
    if (getsockopt(sockfd, IPPROTO_TCP, TCP_USER_TIMEOUT, &user_timeout, &len) == 0) {
	    printf("TCP_USER_TIMEOUT = %d ms\n", user_timeout);
    }


    // Prepare server address
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(5000);
    if (inet_pton(AF_INET, server_ip, &servaddr.sin_addr) != 1) {
        fprintf(stderr, "invalid server ip: %s\n", server_ip);
        close(sockfd);
        return 1;
    }

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
    char reply[516];
    int prev_sndbuf_size = 0;
    size_t total_sent = 0;   // Survives across loops, so a partly sent message is resumed

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

        // A partial send must resume from the same byte. Starting the message
        // again would shift every later message, because TCP has no message boundaries.
        while (total_sent < sizeof(buffer)) {
            ssize_t n = send(sockfd, buffer + total_sent, sizeof(buffer) - total_sent, 0);

            if (n >= 0) {
                total_sent += n;
            } else if (errno == EINTR) {
                continue;
            } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
                printf("Send buffer full, try again later < counter : %d >, <sent %zu of %zu>\n", counter, total_sent, sizeof(buffer));
                break;
            } else {
                printf("send() error ret = %zd , errno = %d , errstr = %s , <counter : %d>\n", n, errno, strerror(errno), counter);
                close(sockfd);
                return 1;
            }
        }

        if (total_sent == sizeof(buffer)) {
            total_sent = 0;
            counter++;
        }

        printf("counter is %d\n", counter);

        // The server never replies, so 0 is how a closed connection shows up here
        if (counter % 10 == 0) {
            ssize_t nRcvdBytes = recv(sockfd, reply, sizeof(reply), 0);
            if (nRcvdBytes == 0) {
                printf("server closed the connection <counter : %d>\n", counter);
                close(sockfd);
                return 1;
            } else if (nRcvdBytes < 0 && errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR) {
                printf("recv() ret = %zd , errno = %d , errstr = %s\n", nRcvdBytes, errno, strerror(errno));
                close(sockfd);
                return 1;
            }
        }

        // Use usleep(10 * 1000) here (10 ms) to fill the send buffer faster
        sleep(1);
    }

    printf("Exiting client. <counter : %d>\n", counter);
    close(sockfd);
    return 0;
}
 
