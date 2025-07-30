#include <iostream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <memory>
#include <vector>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>

struct Connection
{
    int client_fd;
    int target_fd;
    std::string buffer_client_to_target;
    std::string buffer_target_to_client;
};

void set_non_blocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int create_listener(int port)
{
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(listen_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
    listen(listen_fd, 128);
    set_non_blocking(listen_fd);
    return listen_fd;
}

int connect_to_target(const std::string& ip, int port)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return -1;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);

    if (connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0)
    {
        if (errno != EINPROGRESS)
        {
            close(fd);
            return -1;
        }
    }
    set_non_blocking(fd);
    return fd;
}

void add_fd(int epoll_fd, int fd, uint32_t events)
{
    epoll_event ev{};
    ev.data.fd = fd;
    ev.events = events;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev);
}

void modify_fd(int epoll_fd, int fd, uint32_t events)
{
    epoll_event ev{};
    ev.data.fd = fd;
    ev.events = events;
    epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev);
}

int main()
{
    constexpr int LISTEN_PORT = 8080;
    constexpr std::string_view TARGET_IP = "127.0.0.1";
    constexpr int TARGET_PORT = 9090;

    int listen_fd = create_listener(LISTEN_PORT);
    std::cout << "Listening on port " << LISTEN_PORT << "\n";

    int epoll_fd = epoll_create1(0);
    add_fd(epoll_fd, listen_fd, EPOLLIN);

    std::unordered_map<int, std::shared_ptr<Connection>> fd_map;

    std::vector<epoll_event> events(64);

    while (true)
    {
        int n = epoll_wait(epoll_fd, events.data(), events.size(), 1000);
        if (n < 0)
        {
            perror("epoll_wait");
            break;
        }

        for (int i = 0; i < n; ++i)
        {
            int fd = events[i].data.fd;
            uint32_t ev = events[i].events;

            if (fd == listen_fd && (ev & EPOLLIN))
            {
                sockaddr_in client_addr{};
                socklen_t len = sizeof(client_addr);
                int client_fd = accept(listen_fd, reinterpret_cast<sockaddr*>(&client_addr), &len);
                if (client_fd < 0)
                {
                    perror("accept");
                    continue;
                }
                set_non_blocking(client_fd);

                int target_fd = connect_to_target(std::string(TARGET_IP), TARGET_PORT);
                if (target_fd < 0)
                {
                    close(client_fd);
                    continue;
                }

                auto conn = std::make_shared<Connection>();
                conn->client_fd = client_fd;
                conn->target_fd = target_fd;

                fd_map[client_fd] = conn;
                fd_map[target_fd] = conn;

                add_fd(epoll_fd, client_fd, EPOLLIN | EPOLLOUT | EPOLLET);
                add_fd(epoll_fd, target_fd, EPOLLIN | EPOLLOUT | EPOLLET);

                std::cout << "[+] New client connected.\n";
                continue;
            }

            if (fd_map.find(fd) != fd_map.end())
            {
                auto conn = fd_map[fd];

                int peer_fd = (fd == conn->client_fd) ? conn->target_fd : conn->client_fd;
                bool is_client = (fd == conn->client_fd);

                if (ev & EPOLLIN)
                {
                    char buf[4096];
                    while (true)
                    {
                        ssize_t nread = read(fd, buf, sizeof(buf));
                        if (nread > 0)
                        {
                            if (is_client)
                                conn->buffer_client_to_target.append(buf, nread);
                            else
                                conn->buffer_target_to_client.append(buf, nread);
                        }
                        else if (nread == -1 && errno == EAGAIN)
                        {
                            break;
                        }
                        else
                        {
                            close(conn->client_fd);
                            close(conn->target_fd);
                            fd_map.erase(conn->client_fd);
                            fd_map.erase(conn->target_fd);
                            std::cout << "[-] Connection closed.\n";
                            break;
                        }
                    }
                }

                if (ev & EPOLLOUT)
                {
                    std::string& outbuf =
                        is_client ? conn->buffer_target_to_client : conn->buffer_client_to_target;

                    while (!outbuf.empty())
                    {
                        ssize_t nwritten = write(fd, outbuf.data(), outbuf.size());
                        if (nwritten > 0)
                        {
                            outbuf.erase(0, nwritten);
                        }
                        else if (nwritten == -1 && errno == EAGAIN)
                        {
                            break;
                        }
                        else
                        {
                            close(conn->client_fd);
                            close(conn->target_fd);
                            fd_map.erase(conn->client_fd);
                            fd_map.erase(conn->target_fd);
                            std::cout << "[-] Connection closed during write.\n";
                            break;
                        }
                    }
                }
            }
        }
    }

    close(listen_fd);
    close(epoll_fd);
    return 0;
}

