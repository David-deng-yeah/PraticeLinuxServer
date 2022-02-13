#pragma once

#include "Socket.h"
#include <memory>

class Data : public std::enable_shared_from_this<Data> {

public: 
    Data();
    
    std::shared_ptr<ClientSocket> ClientSocket_;
    int epoll_fd;
};