#include "Socket.h"
#include <iostream>
#include <cstring>
#include "Util.h"

void setReusePort(int fd) {
    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const void *)&opt, sizeof(opt));
}


ClientSocket::ClientSocket() {
    fd = -1;
}
ClientSocket::~ClientSocket() {
    close();
}
void ClientSocket::close() {
    if (fd >= 0) {
        std::cout << "Log: ClientSocket closed. fd: " << fd << std::endl;
        ::close(fd);
        fd = -1;
    }
}

ServerSocket::ServerSocket(int port, const char *ip) : mPort(port), mIp(ip) {
    bzero(&mAddr, sizeof(mAddr));
    mAddr.sin_family = AF_INET;
    mAddr.sin_port = htons(port);
    if (ip != nullptr) {
        inet_pton(AF_INET, ip, &mAddr.sin_addr);
    } else {
        mAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == -1) {
        std::cout << "Log: Error! Create socket error in file!" << std::endl;
        exit(0);
    }
    setReusePort(listen_fd);
    setnonblocking(listen_fd);
}
void ServerSocket::bind() {
    int ret = ::bind(listen_fd, (struct sockaddr*)&mAddr, sizeof(mAddr));
    if (ret == -1) {
        std::cout << "Log: Error! Bind socket error in file!" << std::endl;
        exit(0);
    }
}
void ServerSocket::listen() {
    int ret = ::listen(listen_fd, 1024);
    if (ret == -1) {
        std::cout << "Log: Error! Listen socket error in file!" << std::endl;
        exit(0);
    }
}
int ServerSocket::accept(ClientSocket &ClientSocket) const {
    int client_fd = ::accept(listen_fd, NULL, NULL);
    if (client_fd < 0) {
        if ((errno == EWOULDBLOCK) || (errno == EAGAIN)) return client_fd;
        std::cout << "Log: Error! Accept socket error in file!" << std::endl;
        std::cout << "     clientfd: " << client_fd << std::endl;
    }
    ClientSocket.fd = client_fd;
    return client_fd;
}
ServerSocket::~ServerSocket() {
    close();
}
void ServerSocket::close() {
    if (listen_fd >= 0) {
        std::cout << "Log: ServerSocket closed. fd: " << listen_fd << std::endl;
        ::close(listen_fd);
        listen_fd = -1;
    }
}