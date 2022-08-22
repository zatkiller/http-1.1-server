#include "event_loop.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <nlohmann/json.hpp>
#include <sys/epoll.h>
#include <unistd.h>

namespace http_server {

    void EventLoop::start() {
        for (int i = 0; i < THREAD_POOL_SIZE; i++)
            workersEpoll[i] = epoll_create1(0);        

        auto eventLoopImpl = [&](int workerIdx) {
            for (;;) {
                int nfds = epoll_wait(workersEpoll[workerIdx], events[workerIdx], MAX_EVENTS, -1);
                for (int i = 0; i < nfds; ++i) {
                    auto &event = events[workerIdx][i];

                    if (event.events & EPOLLHUP) {
                        EventData* ePtr = reinterpret_cast<EventData*>(event.data.ptr);
                        int fd = ePtr->fd;
                        // std::cout << "DELETE: " << fd << std::endl;
                        if (epoll_ctl(workersEpoll[workerIdx], EPOLL_CTL_DEL, fd, NULL) < 0) {
                            std::cout << "ERROR OCCURED WHEN DELETING: " << strerror(errno) << std::endl;
                            std::cout << "FD: " << fd << std::endl;
                            continue;
                        }
                        delete ePtr;
                        close(fd);
                    } else if (event.events == EPOLLIN) {
                        eventHandler->handleRead(event);
                        epoll_event newEvent;
                        newEvent.events = EPOLLOUT;  // | EPOLLET | EPOLLONESHOT;
                        newEvent.data.ptr = event.data.ptr;
                        EventData* ePtr = reinterpret_cast<EventData*>(newEvent.data.ptr);
                        int fd = ePtr->fd;
                        if (epoll_ctl(workersEpoll[workerIdx], EPOLL_CTL_MOD, fd, &newEvent) < 0) {
                            std::cout << "(1) ERROR OCCURED WHEN MODIFYING: " << strerror(errno) << std::endl;
                            std::cout << "FD: " << fd << std::endl;
                            continue;
                        }
                    } else if (event.events == EPOLLOUT) {
                        eventHandler->handleWrite(event);

                        epoll_event newEvent;
                        newEvent.events = EPOLLIN; // | EPOLLET | EPOLLONESHOT;
                        newEvent.data.ptr = event.data.ptr;
                        EventData* ePtr = reinterpret_cast<EventData*>(newEvent.data.ptr);
                        int fd = ePtr->fd;
                        if (epoll_ctl(workersEpoll[workerIdx], EPOLL_CTL_MOD, fd, &newEvent) < 0) {
                            std::cout << "(2) ERROR OCCURED WHEN MODIFYING: " << strerror(errno) << std::endl;
                            std::cout << "FD: " << fd << std::endl;
                            continue;
                        }
                    } else {
                        std::cout << "An unexpected event has occured: err=" << strerror(errno) << std::endl;
                    }
                }
            }
        };

        for (int i = 0; i < THREAD_POOL_SIZE; i++)
            workers[i] = std::thread(eventLoopImpl, i);

        int workerIdx = 0;
        for(;;) {
            sockaddr_in address;
            socklen_t addressSize = sizeof(address);
            int clientFd = accept4(tcpFd_, (sockaddr *)&address, &addressSize, SOCK_NONBLOCK);
            if (clientFd < 0) continue;
            setToNonBlocking(clientFd);
            epoll_event newEvent;
            newEvent.events = EPOLLIN; // | EPOLLET | EPOLLONESHOT;
            std::string clientIp = inet_ntoa(address.sin_addr);
            newEvent.data.ptr = new EventData(clientFd, clientIp);

            if (epoll_ctl(workersEpoll[workerIdx], EPOLL_CTL_ADD, clientFd, &newEvent) < 0) {
                std::cout << "ERROR OCCURED WHEN ADDING: " << strerror(errno) << std::endl;
                std::cout << "FD: " << clientFd << std::endl;
            }
            workerIdx++;
            workerIdx = workerIdx == THREAD_POOL_SIZE ? 0 : workerIdx;
        }
    }

    void EventLoop::setEventHandler(EventHandler* handler) {
        eventHandler = handler;
    }

    void EventLoop::setToNonBlocking(const int fd) {
       // Get current flags
        int flags = fcntl(fd, F_GETFL);
        // Set Non blocking flag for TCP fd. https://man7.org/linux/man-pages/man2/fcntl.2.html
        fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    }

    void EventLoop::setTcpFd(const int fd) {
        tcpFd_ = fd;
    }
}
