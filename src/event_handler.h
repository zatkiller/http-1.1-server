#pragma once

#include <string>
#include <sys/epoll.h>
#include <utility>  

namespace http_server {
class EventHandler {
public:
    virtual std::pair<int, std::string> handleAccept(epoll_event &event) = 0;
    virtual void handleRead(epoll_event &event) = 0;
    virtual void handleWrite(epoll_event &event) = 0;
};
}