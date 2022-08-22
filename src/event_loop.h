#pragma once

#include "event_data.h"
#include "event_handler.h"

#include <memory>
#include <sys/epoll.h>
#include <thread>

#define MAX_EVENTS 11000
#define THREAD_POOL_SIZE 5

namespace http_server {
class EventLoop {
public:
    static EventLoop& getInstance() { 
        static EventLoop instance; 
        return instance; 
    }

    ~EventLoop() = default;
    EventLoop(const EventLoop &other) = delete;
    EventLoop& operator=(const EventLoop &other) = delete;

    void start();
    void setTcpFd(const int fd);
    void setToNonBlocking(const int fd);
    void setEventHandler(EventHandler* handler);
    int getFd() { return eventLoopFd_; }
private:
    EventLoop() {
        eventLoopFd_ = epoll_create1(0);
    };
    int eventLoopFd_;
    int tcpFd_;
    EventHandler* eventHandler;
    std::thread workers[THREAD_POOL_SIZE];
    int workersEpoll[THREAD_POOL_SIZE];
    epoll_event events[THREAD_POOL_SIZE][MAX_EVENTS];
};
}