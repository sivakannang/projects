// g++ -std=c++17 forwarder.cpp -o forwarder      (Linux only: uses epoll)
//
// A TCP port forwarder: ./forwarder [listen_port target_ip target_port]
// The default is 8080 -> 127.0.0.1:9090. It listens on all network interfaces, so anyone who
// can reach this machine can reach the target through it.
//
// One thread, using level-triggered epoll. Each socket only asks for the events it can act on:
//   EPOLLIN   while the buffer for the other side has room. A full buffer stops the reads, so TCP
//             flow control slows down the sender, and a slow reader can't use up all the memory.
//   EPOLLOUT  while there is data waiting, or while the connection to the target is in progress.
//             Asking for it all the time would wake the loop constantly, as sockets are usually writable.

#include <arpa/inet.h>
#include <cerrno>
#include <csignal>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <memory>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <string>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <unordered_map>
#include <vector>

namespace
{

constexpr size_t MAX_BUFFERED = 1024 * 1024;   // Per direction
constexpr size_t READ_SIZE = 64 * 1024;

// The data going one way, and how far that direction has got
struct Pipe
{
    std::string data;
    size_t offset = 0;        // Bytes already written. Erasing after each write would mean moving all the rest.
    bool source_eof = false;  // The source finished sending
    bool shut_down = false;   // Everything was delivered, and the destination has been told

    size_t pending() const { return data.size() - offset; }
    bool has_room() const { return pending() < MAX_BUFFERED; }
    bool finished() const { return source_eof && pending() == 0 && shut_down; }

    void consume(size_t count)
    {
        offset += count;

        if (offset == data.size())
        {
            data.clear();
            offset = 0;
        }
        else if (offset >= MAX_BUFFERED)
        {
            data.erase(0, offset);
            offset = 0;
        }
    }
};

struct Connection
{
    int client_fd = -1;
    int target_fd = -1;
    bool connecting = true;
    bool closed = false;
    uint32_t client_events = 0;   // What each socket is currently registered for. 0 means not registered.
    uint32_t target_events = 0;
    Pipe client_to_target;
    Pipe target_to_client;
};

using ConnectionPtr = std::shared_ptr<Connection>;

class Forwarder
{
public:
    Forwarder(int listen_fd, int epoll_fd, const sockaddr_in& target)
        : listen_fd_(listen_fd), epoll_fd_(epoll_fd), target_(target) {}

    int run();

private:
    void accept_clients();
    void pause_listener();
    void resume_listener();
    void handle_event(const ConnectionPtr& conn, int fd, uint32_t events);
    void finish_connecting(const ConnectionPtr& conn);
    void read_from(const ConnectionPtr& conn, int fd);
    void write_to(const ConnectionPtr& conn, int fd);
    void update_events(const ConnectionPtr& conn);
    void set_events(int fd, uint32_t& current, uint32_t wanted);
    void close_connection(const ConnectionPtr& conn, const char* reason);
    void release_closed();

    int listen_fd_;
    int epoll_fd_;
    sockaddr_in target_;
    bool listener_paused_ = false;
    std::unordered_map<int, ConnectionPtr> fd_map_;
    std::vector<ConnectionPtr> closed_;
};

bool set_non_blocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);

    return flags != -1 && fcntl(fd, F_SETFL, flags | O_NONBLOCK) != -1;
}

// Sends small writes at once, instead of holding them back to combine with later ones
void set_no_delay(int fd)
{
    int opt = 1;
    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt));
}

int create_listener(int port)
{
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0)
    {
        perror("socket");
        return -1;
    }

    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(static_cast<uint16_t>(port));
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listen_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0)
    {
        perror("bind");
        close(listen_fd);
        return -1;
    }

    if (listen(listen_fd, 128) < 0 || !set_non_blocking(listen_fd))
    {
        perror("listen");
        close(listen_fd);
        return -1;
    }

    return listen_fd;
}

// The socket must be nonblocking before connect, so a slow target can't stall every other
// connection. Sets connecting to true if the result will come later, through EPOLLOUT.
int connect_to_target(const sockaddr_in& target, bool& connecting)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        perror("socket");
        return -1;
    }

    if (!set_non_blocking(fd))
    {
        perror("fcntl");
        close(fd);
        return -1;
    }

    connecting = false;

    if (connect(fd, reinterpret_cast<const sockaddr*>(&target), sizeof(target)) < 0)
    {
        if (errno != EINPROGRESS)
        {
            perror("connect");
            close(fd);
            return -1;
        }

        connecting = true;
    }

    return fd;
}

int Forwarder::run()
{
    std::vector<epoll_event> events(64);

    while (true)
    {
        int n = epoll_wait(epoll_fd_, events.data(), static_cast<int>(events.size()), 1000);
        if (n < 0)
        {
            if (errno == EINTR)
                continue;

            perror("epoll_wait");
            return 1;
        }

        if (listener_paused_)
            resume_listener();

        for (int i = 0; i < n; ++i)
        {
            int fd = events[i].data.fd;

            if (fd == listen_fd_)
            {
                if (!listener_paused_)
                    accept_clients();
                continue;
            }

            auto found = fd_map_.find(fd);
            if (found != fd_map_.end())
            {
                ConnectionPtr conn = found->second;
                if (!conn->closed)
                    handle_event(conn, fd, events[i].events);
            }
        }

        // Only now is it safe to close. A closed number could be reused by accept during
        // the batch, and a later event for the old socket would then be applied to the new one.
        release_closed();
    }
}

void Forwarder::accept_clients()
{
    while (true)
    {
        int client_fd = accept(listen_fd_, nullptr, nullptr);
        if (client_fd < 0)
        {
            if (errno == EMFILE || errno == ENFILE || errno == ENOBUFS || errno == ENOMEM)
            {
                // Out of resources. The client stays queued, so level-triggered epoll
                // would report it again immediately, and the loop would spin.
                perror("accept");
                pause_listener();
            }
            else if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR && errno != ECONNABORTED)
            {
                perror("accept");
            }

            return;
        }

        if (!set_non_blocking(client_fd))
        {
            perror("fcntl");
            close(client_fd);
            continue;
        }

        bool connecting = false;
        int target_fd = connect_to_target(target_, connecting);
        if (target_fd < 0)
        {
            close(client_fd);
            continue;
        }

        set_no_delay(client_fd);
        set_no_delay(target_fd);

        auto conn = std::make_shared<Connection>();
        conn->client_fd = client_fd;
        conn->target_fd = target_fd;
        conn->connecting = connecting;

        fd_map_[client_fd] = conn;
        fd_map_[target_fd] = conn;

        update_events(conn);

        std::cout << "[+] New client connected." << std::endl;   // endl flushes, so a redirected log stays current
    }
}

void Forwarder::pause_listener()
{
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, listen_fd_, nullptr) == 0)
        listener_paused_ = true;
}

void Forwarder::resume_listener()
{
    epoll_event ev{};
    ev.data.fd = listen_fd_;
    ev.events = EPOLLIN;

    if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, listen_fd_, &ev) == 0)
        listener_paused_ = false;
}

void Forwarder::handle_event(const ConnectionPtr& conn, int fd, uint32_t events)
{
    const bool is_target = (fd == conn->target_fd);

    if (is_target && conn->connecting)
    {
        finish_connecting(conn);
        return;
    }

    // EPOLLERR and EPOLLHUP don't need asking for. The read or write then reports what happened.
    if (events & (EPOLLIN | EPOLLERR | EPOLLHUP))
        read_from(conn, fd);

    if (!conn->closed && (events & (EPOLLOUT | EPOLLERR | EPOLLHUP)))
        write_to(conn, fd);

    if (!conn->closed)
        update_events(conn);
}

void Forwarder::finish_connecting(const ConnectionPtr& conn)
{
    int error = 0;
    socklen_t len = sizeof(error);

    if (getsockopt(conn->target_fd, SOL_SOCKET, SO_ERROR, &error, &len) < 0)
        error = errno;

    if (error != 0)
    {
        std::cerr << "connect to target: " << std::strerror(error) << "\n";
        close_connection(conn, "target connect failed");
        return;
    }

    conn->connecting = false;

    write_to(conn, conn->target_fd);   // Anything the client has sent so far

    if (!conn->closed)
        update_events(conn);
}

void Forwarder::read_from(const ConnectionPtr& conn, int fd)
{
    const bool from_client = (fd == conn->client_fd);
    Pipe& pipe = from_client ? conn->client_to_target : conn->target_to_client;
    const int peer_fd = from_client ? conn->target_fd : conn->client_fd;

    if (pipe.source_eof || !pipe.has_room())
        return;

    char buf[READ_SIZE];
    ssize_t nread = read(fd, buf, sizeof(buf));

    if (nread > 0)
    {
        pipe.data.append(buf, static_cast<size_t>(nread));
    }
    else if (nread == 0)
    {
        pipe.source_eof = true;
    }
    else if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
    {
        return;
    }
    else
    {
        close_connection(conn, "read error");
        return;
    }

    // Try to pass it on now. Waiting for EPOLLOUT would never end, because it isn't requested
    // until there is data, and the data would otherwise only sit in the buffer.
    if (!(peer_fd == conn->target_fd && conn->connecting))
        write_to(conn, peer_fd);
}

void Forwarder::write_to(const ConnectionPtr& conn, int fd)
{
    const bool to_client = (fd == conn->client_fd);
    Pipe& pipe = to_client ? conn->target_to_client : conn->client_to_target;

    while (pipe.pending() > 0)
    {
        ssize_t nwritten = write(fd, pipe.data.data() + pipe.offset, pipe.pending());

        if (nwritten > 0)
        {
            pipe.consume(static_cast<size_t>(nwritten));
        }
        else if (nwritten < 0 && errno == EINTR)
        {
            continue;
        }
        else if (nwritten < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
        {
            return;
        }
        else
        {
            close_connection(conn, "write error");
            return;
        }
    }

    // TCP can close one direction at a time. A client may finish sending and still be waiting
    // for the reply, so only pass on the end of this direction, and only after all its data.
    if (pipe.source_eof && !pipe.shut_down)
    {
        shutdown(fd, SHUT_WR);
        pipe.shut_down = true;
    }

    if (conn->client_to_target.finished() && conn->target_to_client.finished())
        close_connection(conn, "closed");
}

void Forwarder::update_events(const ConnectionPtr& conn)
{
    uint32_t client = 0;
    uint32_t target = 0;

    if (!conn->client_to_target.source_eof && conn->client_to_target.has_room())
        client |= EPOLLIN;

    if (conn->target_to_client.pending() > 0)
        client |= EPOLLOUT;

    if (conn->connecting)
    {
        target = EPOLLOUT;
    }
    else
    {
        if (!conn->target_to_client.source_eof && conn->target_to_client.has_room())
            target |= EPOLLIN;

        if (conn->client_to_target.pending() > 0)
            target |= EPOLLOUT;
    }

    set_events(conn->client_fd, conn->client_events, client);
    set_events(conn->target_fd, conn->target_events, target);
}

// With nothing to wait for, the socket leaves epoll altogether. Staying registered with no
// events would still report EPOLLHUP constantly, and the loop would spin.
void Forwarder::set_events(int fd, uint32_t& current, uint32_t wanted)
{
    if (current == wanted)
        return;

    epoll_event ev{};
    ev.data.fd = fd;
    ev.events = wanted;

    int op = (current == 0) ? EPOLL_CTL_ADD : (wanted == 0) ? EPOLL_CTL_DEL : EPOLL_CTL_MOD;

    if (epoll_ctl(epoll_fd_, op, fd, &ev) < 0)
        perror("epoll_ctl");
    else
        current = wanted;
}

void Forwarder::close_connection(const ConnectionPtr& conn, const char* reason)
{
    if (conn->closed)
        return;

    conn->closed = true;
    closed_.push_back(conn);

    std::cout << "[-] Connection " << reason << "." << std::endl;
}

void Forwarder::release_closed()
{
    for (const ConnectionPtr& conn : closed_)
    {
        fd_map_.erase(conn->client_fd);
        fd_map_.erase(conn->target_fd);

        // close also removes each socket from epoll
        close(conn->client_fd);
        close(conn->target_fd);
    }

    closed_.clear();
}

bool parse_port(const char* text, int& port)
{
    char* end = nullptr;
    long value = std::strtol(text, &end, 10);

    if (end == text || *end != '\0' || value < 1 || value > 65535)
        return false;

    port = static_cast<int>(value);

    return true;
}

}  // namespace

int main(int argc, char* argv[])
{
    int listen_port = 8080;
    int target_port = 9090;
    const char* target_ip = "127.0.0.1";

    if (argc != 1 && argc != 4)
    {
        std::cerr << "usage: " << argv[0] << " [listen_port target_ip target_port]\n";
        return 2;
    }

    if (argc == 4)
    {
        target_ip = argv[2];

        if (!parse_port(argv[1], listen_port) || !parse_port(argv[3], target_port))
        {
            std::cerr << "ports must be 1 to 65535\n";
            return 2;
        }
    }

    sockaddr_in target{};
    target.sin_family = AF_INET;
    target.sin_port = htons(static_cast<uint16_t>(target_port));

    if (inet_pton(AF_INET, target_ip, &target.sin_addr) != 1)
    {
        std::cerr << "invalid target ip: " << target_ip << "\n";
        return 2;
    }

    // A client that disconnects must make write fail with EPIPE, not kill the whole forwarder
    signal(SIGPIPE, SIG_IGN);

    int listen_fd = create_listener(listen_port);
    if (listen_fd < 0)
        return 1;

    int epoll_fd = epoll_create1(EPOLL_CLOEXEC);
    if (epoll_fd < 0)
    {
        perror("epoll_create1");
        close(listen_fd);
        return 1;
    }

    epoll_event ev{};
    ev.data.fd = listen_fd;
    ev.events = EPOLLIN;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &ev) < 0)
    {
        perror("epoll_ctl");
        close(epoll_fd);
        close(listen_fd);
        return 1;
    }

    std::cout << "Listening on port " << listen_port << ", forwarding to " << target_ip << ":" << target_port << "\n" << std::flush;

    Forwarder forwarder(listen_fd, epoll_fd, target);
    int status = forwarder.run();

    close(epoll_fd);
    close(listen_fd);

    return status;
}
