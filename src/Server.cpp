#include <cstring>
#include <iostream>
#include <string>

#include "Server.h"
#include "ThreadPool.h"
#include "Epoll.h"

ExeServer::ExeServer(int port, const char* ip) : serverSocket(port, ip) {
    serverSocket.bind();
    serverSocket.listen();
}
void ExeServer::run(int thread_num, int max_queue_size) {
    ThreadPool threadpool(thread_num, max_queue_size);

    int epoll_fd = Epoll::init(1024);
    serverSocket.epoll_fd = epoll_fd;

    std::shared_ptr<Data> data(new Data);
    data->epoll_fd = epoll_fd;

    __uint32_t event = (EPOLLIN | EPOLLET);
    Epoll::addfd(epoll_fd, serverSocket.listen_fd, event, data);

    while (true) {
        std::vector<std::shared_ptr<Data>> events = Epoll::poll(serverSocket, 1024, -1);

        for (auto &req : events) {
            threadpool.append(req, std::bind(&ExeServer::handleRequest, this, std::placeholders::_1));
        }
    }
}
void ExeServer::handleRequest(std::shared_ptr<void> arg) {
    std::shared_ptr<Data> sharedData = std::static_pointer_cast<Data>(arg);

    // pocess main
    char buffer[BUFFERSIZE];
    bzero(buffer, BUFFERSIZE);

    ssize_t recv_data;
    int read_index = 0;

    while (true) {
        //std::cout << "Log: Loop" << std::endl;
        recv_data = recv(sharedData->ClientSocket_->fd, buffer + read_index, BUFFERSIZE - read_index, 0);
        if (recv_data == -1) {
            if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                //std::cout << "Log: read later" << std::endl;
                return;
            }
            std::cout << "Log: Error! reading faild!" << std::endl;
            return;
        }
        if (recv_data == 0) {
            //std::cout << "Server: " << sharedData.use_count() << std::endl;
            std::cout << "Log: connection closed by peer" << std::endl;
            return;
        }

        std::string header_s = "user[fd" + std::to_string(sharedData->ClientSocket_->fd) + "]: ";
        const char* header = header_s.c_str();

        //std::cout << header << std::endl;

        send(sharedData->ClientSocket_->fd, header, strlen(header), 0);
        send(sharedData->ClientSocket_->fd, buffer + read_index, strlen(buffer + read_index), 0);

        auto it = Epoll::dataMap.begin();
        while (it != Epoll::dataMap.end()) {

            if (it->second->ClientSocket_) {
                send(it->second->ClientSocket_->fd, header, strlen(header), 0);
                send(it->second->ClientSocket_->fd, buffer + read_index, strlen(buffer + read_index), 0);
            }
            
            it++;
        }

        //FIXME EPOLLONESHOT reregister
        Epoll::setfd(sharedData->epoll_fd, sharedData->ClientSocket_->fd, Epoll::DEFAULT_EVENTS, sharedData);
    
        read_index += recv_data;
    }

}