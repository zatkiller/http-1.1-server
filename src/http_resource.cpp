#include "http_resource.h"

namespace http_server {

    bool HttpResource::hasMethod(HttpMethod method) {
         return methodHandlerMap_.find(method) != methodHandlerMap_.end();
    }

    void HttpResource::addRouteHandler(HttpMethod method, const RouteHandler &routeHandler) {
        methodHandlerMap_.emplace(method, routeHandler);
    }

    void HttpResource::handle(HttpRequest& request, HttpResponse& response) {
        HttpMethod httpMethod = request.getMethod();
        auto func = methodHandlerMap_[httpMethod];
        func(request, response);
    }
}