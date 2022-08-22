#pragma once

#include "event_handler.h"
#include "event_loop.h"
#include "http_message.h"
#include "http_resource.h"

#include <arpa/inet.h>

#define DEFAULT_PORT 8080
#define LISTEN_BACKLOG_SIZE 4096
#define BUFFER_SIZE 2048

namespace http_server {
class HttpServer : public EventHandler {
public:
    HttpServer(int port = DEFAULT_PORT);

    sockaddr_in setupListeningAddress();
    void start();
    void stop();

    std::pair<int, std::string> handleAccept(epoll_event &event) override;
    void handleRead(epoll_event &event) override;
    void handleWrite(epoll_event &event) override;

    void routeRequest(HttpRequest& request, HttpResponse& response);
    void addResource(const std::string& path, const HttpResource& resource);

private:
    int port_;
    int tcpFd_;
    EventLoop &eventLoop_;
    std::unordered_map<std::string, HttpResource> httpResourceMap_;;
};

}