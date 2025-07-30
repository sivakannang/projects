#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/select.h>

#define SERVER_PORT 5000
#define RCVBUF_SIZE (10 * 1024 * 1024)    // 10 MB
#define READ_SIZE 516
#define SELECT_TIMEOUT_USEC 2000000
#define RCVTIMEO_SEC 2
#define RCVTIMEO_USEC 0
#define MAX_READ_ATTEMPTS 10

int main() {
    int listen_fd, client_fd = -1;
    struct sockaddr_in servaddr, cliaddr;
    socklen_t clilen;
    int optval;
    char buffer[READ_SIZE + 1];  // +1 for null termination
    int counter = 0;
    int ret;

    // Ignore SIGPIPE to prevent crashing on broken pipes
    signal(SIGPIPE, SIG_IGN);

    // Create listening socket
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Enable SO_REUSEADDR
    optval = 1;
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        perror("setsockopt SO_REUSEADDR");
        exit(EXIT_FAILURE);
    }

    // Bind address
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERVER_PORT);

    if (bind(listen_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(listen_fd, 5) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", SERVER_PORT);

    while (1) {
        // Prepare for select()
        fd_set readfds;
        struct timeval tv;
        FD_ZERO(&readfds);
        FD_SET(listen_fd, &readfds);
        tv.tv_sec = SELECT_TIMEOUT_USEC / 1000000;
        tv.tv_usec = SELECT_TIMEOUT_USEC % 1000000;

        ret = select(listen_fd + 1, &readfds, NULL, NULL, &tv);
        if (ret < 0) {
            perror("select");
            continue;
        } else if (ret == 0) {
            printf("Waiting for connection...\n");
            continue;
        }

        if (FD_ISSET(listen_fd, &readfds)) {
            clilen = sizeof(cliaddr);
            client_fd = accept(listen_fd, (struct sockaddr *)&cliaddr, &clilen);
            if (client_fd < 0) {
                perror("accept");
                continue;
            }
            printf("Accepted new client.\n");

            // Set SO_RCVBUF
            optval = RCVBUF_SIZE;
            if (setsockopt(client_fd, SOL_SOCKET, SO_RCVBUF, &optval, sizeof(optval)) < 0) {
                perror("setsockopt SO_RCVBUF");
            }

            // Set SO_RCVTIMEO
            struct timeval rcv_timeout;
            rcv_timeout.tv_sec = RCVTIMEO_SEC;
            rcv_timeout.tv_usec = RCVTIMEO_USEC;
            if (setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, &rcv_timeout, sizeof(rcv_timeout)) < 0) {
                perror("setsockopt SO_RCVTIMEO");
            }

            // Enable TCP keepalive
            optval = 1;
            if (setsockopt(client_fd, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval)) < 0) {
                perror("setsockopt SO_KEEPALIVE");
            }

            optval = 5;
            if (setsockopt(client_fd, IPPROTO_TCP, TCP_KEEPIDLE, &optval, sizeof(optval)) < 0) {
                perror("setsockopt TCP_KEEPIDLE");
            }

            optval = 2;
            if (setsockopt(client_fd, IPPROTO_TCP, TCP_KEEPINTVL, &optval, sizeof(optval)) < 0) {
                perror("setsockopt TCP_KEEPINTVL");
            }

            optval = 5;
            if (setsockopt(client_fd, IPPROTO_TCP, TCP_KEEPCNT, &optval, sizeof(optval)) < 0) {
                perror("setsockopt TCP_KEEPCNT");
            }

            counter = 0;

            while (1) {
                size_t total_read = 0;
                int attempts = 0;

                while (total_read < READ_SIZE && attempts < MAX_READ_ATTEMPTS) {
                    ssize_t n = recv(client_fd, buffer + total_read,
                                     READ_SIZE - total_read, 0);
                    if (n > 0) {
                        total_read += n;
                    } else if (n == 0) {
                        printf("Client disconnected normally.\n");
                        close(client_fd);
                        client_fd = -1;
                        goto accept_next;
                    } else {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            printf("Recv timeout, retrying...\n");
                            attempts++;
                            continue;
                        } else {
                            perror("recv");
                            close(client_fd);
                            client_fd = -1;
                            goto accept_next;
                        }
                    }
                }

                if (total_read == READ_SIZE) {
                    counter++;
                    buffer[READ_SIZE] = '\0'; // Null terminate for safety if data is text
                    printf("[#%d] bytes received , data = %s\n", counter, buffer);
                    // Optional: print content
                    // printf("%.*s\n", READ_SIZE, buffer);
                } else {
                    printf("Partial read incomplete after %d attempts, closing client.\n", attempts);
                    close(client_fd);
                    client_fd = -1;
                    goto accept_next;
                }
            }

        accept_next:
            // Loop back to accept a new client
            continue;
        }
    }

    close(listen_fd);
    return 0;
}

