#include <iostream>
#include <sys/prctl.h>
#include "ThreadPool.h"
#include "Data.h"

ThreadPool::ThreadPool(int thread_s, int max_queue_s) : 
    thread_size(thread_s), max_queue_size(max_queue_s),
    condition_(mutex_), shutdown_(0) {
    
    threads.resize(thread_size);

    for (int i = 0; i < thread_size; ++i) {
        if (pthread_create(&threads[i], NULL, worker, this) != 0) {
            std::cout << "Log: Error! ThreadPool init error" << std::endl;
            throw std::exception();
        }
    }
}
ThreadPool::~ThreadPool() {
    shutdown();
}
bool ThreadPool::append(std::shared_ptr<void> arg, std::function<void(std::shared_ptr<void>)> fun) {
    if (shutdown_) {
        std::cout << "Log: already shutdown" << std::endl;
        return false;
    }

    MutexLockGuard guard(this->mutex_);
    if (request_queue.size() > max_queue_size) {
        std::cout << "Log: too many requests" << std::endl;
        return false;
    }
    
    ThreadTask threadTask;
    threadTask.arg = arg;
    threadTask.process = fun;
    
    request_queue.push_back(threadTask);
    condition_.notify();
    return true;
}
void ThreadPool::shutdown() {
    {
        MutexLockGuard guard(this->mutex_);
        if (shutdown_) std::cout << "Log: already shutdown" << std::endl;
        shutdown_ = true;
        condition_.notifyAll();
    }
    for (int i = 0; i < thread_size; ++i) {
        if (pthread_join(threads[i], NULL) != 0) {
            std::cout << "Log: Error! pthread_join error" << std::endl;
        }
    }
}
void* ThreadPool::worker(void *args) {
    ThreadPool *pool = static_cast<ThreadPool*>(args);
    if (pool == nullptr) return NULL;

    prctl(PR_SET_NAME,"FlyThread");
    pool->run();
    return NULL;
}
void ThreadPool::run() {
    while (true) {
        ThreadTask requestTask;
        {
            MutexLockGuard guard(this->mutex_);

            while (request_queue.empty() && !shutdown_) {
                condition_.wait();
            }

            if (shutdown_) break;

            requestTask = request_queue.front();
            request_queue.pop_front();
        }
        //std::cout << "Threadpool: " << std::static_pointer_cast<Data>(requestTask.arg).use_count() << std::endl;
        requestTask.process(requestTask.arg);
    }
}