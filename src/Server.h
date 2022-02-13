#pragma once

#include <memory>
#include "Socket.h"

class ExeServer {
public:
    explicit ExeServer(int port = 80, const char* ip = nullptr);
    void run(int thread_num, int max_queue_size);
    void handleRequest(std::shared_ptr<void> arg);
private:
    ServerSocket serverSocket;
    const int BUFFERSIZE = 2048;
};