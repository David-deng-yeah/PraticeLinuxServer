#pragma once

#include <unordered_map>
#include <sys/epoll.h>
#include "Data.h"

class Epoll {

public:
    static int init(int max_events);
    static int addfd(int epoll_fd, int fd, __uint32_t events, std::shared_ptr<Data>);
    static int delfd(int epoll_fd, int fd, __uint32_t events);
    static int setfd(int epoll_fd, int fd, __uint32_t events, std::shared_ptr<Data>);
    static void connectToClient(const ServerSocket &);
    static std::vector<std::shared_ptr<Data>> poll(const ServerSocket &, int max_events, int timeout);

    static std::unordered_map<int, std::shared_ptr<Data>> dataMap;
    static const int MAX_EVENTS;
    static epoll_event *events;
    static const __uint32_t DEFAULT_EVENTS;
};