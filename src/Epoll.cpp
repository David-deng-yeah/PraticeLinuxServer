#include <iostream>
#include <vector>
#include <cstring>
#include "Epoll.h"
#include "Util.h"

std::unordered_map<int, std::shared_ptr<Data>> Epoll::dataMap;
const int Epoll::MAX_EVENTS = 10000;
epoll_event *Epoll::events;
const __uint32_t Epoll::DEFAULT_EVENTS = (EPOLLIN | EPOLLET | EPOLLONESHOT);

int Epoll::init(int max_events) {
    int epoll_fd = ::epoll_create(max_events);
    if (epoll_fd == -1) {
        std::cout << "Log: Error! epoll create error" << std::endl;
        exit(-1);
    }
    events = new epoll_event[max_events];
    return epoll_fd;
}

int Epoll::addfd(int epoll_fd, int fd, __uint32_t events, std::shared_ptr<Data> data) {
    epoll_event event;
    event.events = events;
    event.data.fd = fd;
    dataMap[fd] = data;
    int ret = ::epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);
    if (ret < 0) {
        std::cout << "Log: Error! epoll add error" << std::endl;
        dataMap[fd].reset();
        return -1;
    }
    return 0;
}
int Epoll::setfd(int epoll_fd, int fd, __uint32_t events, std::shared_ptr<Data> data) {
    epoll_event event;
    event.events = events;
    event.data.fd = fd;
    dataMap[fd] = data;
    int ret = ::epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &event);
    if (ret < 0) {
        std::cout << "Log: Error! epoll set error" << std::endl;
        dataMap[fd].reset();
        return -1;
    }
    return 0;
}
int Epoll::delfd(int epoll_fd, int fd, __uint32_t events) {
    epoll_event event;
    event.events = events;
    event.data.fd = fd;
    int ret = ::epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, &event);
    if (ret < 0) {
        std::cout << "Log: Error! epoll del error" << std::endl;
        return -1;
    }
    auto it = dataMap.find(fd);
    if (it != dataMap.end()) {
        dataMap.erase(it);
    }
    return 0;
}
void Epoll::connectToClient(const ServerSocket &serverSocket) {

    std::shared_ptr<ClientSocket> tempClient(new ClientSocket);

    while (serverSocket.accept(*tempClient) > 0) {
        int ret = setnonblocking(tempClient->fd);
        if (ret < 0) {
            std::cout << "Log: Error! setnonblocking error" << std::endl;
            tempClient->close();
            continue ;
        }
    }

    std::shared_ptr<Data> sharedData(new Data);

    std::shared_ptr<ClientSocket> sharedClientSocket(new ClientSocket);
    sharedClientSocket.swap(tempClient);

    sharedData->ClientSocket_ = sharedClientSocket;
    sharedData->epoll_fd = serverSocket.epoll_fd;

    addfd(serverSocket.epoll_fd, sharedClientSocket->fd, DEFAULT_EVENTS, sharedData);

    std::string s_t = "Welcome to exeserver! You are fd" + std::to_string(sharedClientSocket->fd) + "\n";
    
    const char* s = s_t.c_str();

    ::send(sharedData->ClientSocket_->fd, s, strlen(s), 0);

    std::cout << "Log: user fd" << sharedData->ClientSocket_->fd << " join the server" << std::endl;

}

std::vector<std::shared_ptr<Data>> Epoll::poll(const ServerSocket &serverSocket, int max_events, int timeout) {
    int event_num = epoll_wait(serverSocket.epoll_fd, events, max_events, timeout);
    if (event_num < 0) {
        std::cout << "Log: Error! epoll_wait error" << std::endl;
        std::cout << "     " << errno << std::endl;
        exit(-1);
    }

    std::vector<std::shared_ptr<Data>> Datas;
    for (int i = 0; i < event_num; ++i) {
        int fd = events[i].data.fd;

        if (fd == serverSocket.listen_fd) {
            connectToClient(serverSocket);
        } else {
            if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLRDHUP) || (events[i].events & EPOLLHUP)) {
                auto it = dataMap.find(fd);
                if (it != dataMap.end()) {
                    dataMap.erase(it);
                }
                continue ;
            }

            auto it = dataMap.find(fd);
            if (it != dataMap.end()) {
                //std::cout << "Epoll: " << it->second.use_count() << std::endl;
                if ((events[i].events & EPOLLIN) || (events[i].events & EPOLLPRI)) {
                    Datas.push_back(it->second);
                    dataMap.erase(it);
                }
            } else continue ;
        }
    }
    return Datas;
}