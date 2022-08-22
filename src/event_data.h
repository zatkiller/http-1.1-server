#pragma once

#include "http_message.h"

#include <string>

namespace http_server {
class EventData {
public:
    EventData(int fd, const std::string &address): fd(fd), ipAddress(std::move(address)) {};

    int fd;
    void *ptr;
    std::string ipAddress;

    HttpRequest& getRequest() { return request_; }
    HttpResponse& getResponse() { return response_; }

private:
    HttpRequest request_;
    HttpResponse response_;
};
}