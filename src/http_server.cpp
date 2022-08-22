#include "http_server.h"

#include <cstring>
#include <errno.h>
#include <iostream>
#include <sys/socket.h>
#include <stdexcept>
#include <string>
#include <unistd.h>

namespace http_server {
    sockaddr_in HttpServer::setupListeningAddress() {
        sockaddr_in address;
        memset(&address, 0, sizeof(address));

        address.sin_family = AF_INET;
        address.sin_port = htons(port_);
        address.sin_addr.s_addr = htonl(INADDR_ANY);

        return address;
    }

    HttpServer::HttpServer(int port) : port_(port), eventLoop_(EventLoop::getInstance()) {
        if (tcpFd_ = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0); tcpFd_ < 0) {
            throw std::runtime_error("Error occured when trying to create TCP socket");
        }
        // Alternatively, can inet_pton(AF_INET, "0.0.0.0", &address.sin_addr);
    }

    void HttpServer::start() {
        int opt = 1;
        setsockopt(tcpFd_, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)); // To reuse address / port
        sockaddr_in address = setupListeningAddress();

        if (bind(tcpFd_, (sockaddr* )&address, sizeof(address)) < 0) {
            std::string msg = "Error occured when binding to socket. Error "
                "code: " + std::to_string(errno);
            throw std::runtime_error(msg);
        }

        if (listen(tcpFd_, LISTEN_BACKLOG_SIZE) < 0) {
            std::string msg = "Error occured when trying to listen to port: " +
                std::to_string(port_) + ". Error code: " + std::to_string(errno);
            throw std::runtime_error(msg);
        }
        eventLoop_.setTcpFd(tcpFd_);
        eventLoop_.setEventHandler(dynamic_cast<EventHandler*>(this));
        eventLoop_.start();
    }

    void HttpServer::stop() {
        close(tcpFd_);
    }

    std::pair<int, std::string> HttpServer::handleAccept(epoll_event &event) {
        sockaddr_in address;
        socklen_t addressSize = sizeof(address);

        int clientFd = accept(event.data.fd, (sockaddr *)&address, &addressSize);
        return std::make_pair(clientFd, inet_ntoa(address.sin_addr));
    }

    void HttpServer::handleRead(epoll_event &event) {
        EventData* eventData = (EventData*) event.data.ptr;

        char buffer[BUFFER_SIZE];
        memset(buffer, 0, BUFFER_SIZE);
        int fd = eventData->fd;
        read(fd, buffer, BUFFER_SIZE);

        eventData->getRequest().parseRequest(buffer);
        routeRequest(eventData->getRequest(), eventData->getResponse());
    }

    void HttpServer::handleWrite(epoll_event &event) {
        EventData* eventData = (EventData*) event.data.ptr;

        std::string responseStr = eventData->getResponse().toString();
        write(eventData->fd, responseStr.data(), responseStr.size());
    }

    void HttpServer::routeRequest(HttpRequest& request, HttpResponse& response) {
        std::string path = request.getPath();
        if (httpResourceMap_.find(path) == httpResourceMap_.end()) {
            response.setAsNotFound();
            return;
        }

        HttpResource resource = httpResourceMap_[path];
        HttpMethod method = request.getMethod();
        if (!resource.hasMethod(method)) {
            response.setAsMethodNotAllowed();
            return;
        }

        try {
            resource.handle(request, response);
            response.setAsOk();
        } catch(...) {
            response.setAsInternalServerErrror();
        }
    }

    void HttpServer::addResource(const std::string& path, const HttpResource& resource) {
        httpResourceMap_[path] = resource;
    }
};
