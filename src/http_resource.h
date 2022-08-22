#pragma once

#include "http_message.h"

#include <functional>

namespace http_server {
using RouteHandler = std::function<void(HttpRequest&, HttpResponse&)>;

class HttpResource {
public:
    bool hasMethod(HttpMethod method);
    void addRouteHandler(HttpMethod method, const RouteHandler &routeHandler);
    void handle(HttpRequest& request, HttpResponse& response);

private:
    std::unordered_map<HttpMethod, RouteHandler> methodHandlerMap_;
};
}