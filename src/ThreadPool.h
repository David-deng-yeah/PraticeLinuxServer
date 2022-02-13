#pragma once

#include <functional>
#include <memory>
#include <list>
#include <vector>
#include "noncopyable.h"
#include "MutexLock.h"
#include "Condition.h"

const int MAX_THREAD_SIZE = 1024;
const int MAX_QUEUE_SIZE = 10000;

struct ThreadTask {
    std::function<void(std::shared_ptr<void>)> process;
    std::shared_ptr<void> arg;
};

class ThreadPool {
public:
    ThreadPool(int thread_s, int max_queue_s);
    ~ThreadPool();
    bool append(std::shared_ptr<void> arg, std::function<void(std::shared_ptr<void>)> fun);
    void shutdown();
private:
    static void* worker(void *args);
    void run();

    MutexLock mutex_;
    Condition condition_;

    int thread_size;
    int max_queue_size;
    int started;
    int shutdown_;
    std::vector<pthread_t> threads;
    std::list<ThreadTask> request_queue;
};