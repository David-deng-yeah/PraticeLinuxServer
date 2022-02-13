#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

class ClientSocket {

public:
    ClientSocket();
    void close();
    ~ClientSocket();

    int fd;
    socklen_t mLen;
    sockaddr_in mAddr;
};

class ServerSocket {

public:
    ServerSocket(int port = 8080, const char *ip = nullptr);
    ~ServerSocket();
    void bind();
    void listen();
    void close();
    int accept(ClientSocket &) const;

    sockaddr_in mAddr;
    int listen_fd;
    int epoll_fd;
    int mPort;
    const char *mIp;
};