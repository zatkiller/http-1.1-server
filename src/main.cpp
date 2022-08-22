#include "http_message.h"
#include "http_resource.h"
#include "http_server.h"

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string> 
#include <unordered_map>

using json = nlohmann::json;
using http_server::HttpMethod;
using http_server::HttpRequest;
using http_server::HttpResource;
using http_server::HttpResponse;
using http_server::HttpServer;

int fibonacci(int n) {
    if (n <= 1)
        return 1;

    return fibonacci(n-1) + fibonacci(n-2);
}

void getFibonacci(HttpRequest &request, HttpResponse &response) {
    std::unordered_map<std::string, std::string> params = request.getQueryParams();

    json jsonBody;
    int result = fibonacci(std::stoi(params["number"]));
    jsonBody["result"] = result;

    response.setBody(jsonBody.dump());
}

void postFibonacci(HttpRequest &request, HttpResponse &response) {
    std::unordered_map<std::string, std::string> body = request.getBody();

    json jsonBody;
    int result = fibonacci(std::stoi(body["number"]));
    jsonBody["result"] = result;

    response.setBody(jsonBody.dump());
}

void echo(HttpRequest &request, HttpResponse &response) {
    std::unordered_map<std::string, std::string> body = request.getBody();
    std::unordered_map<std::string, std::string> params = request.getQueryParams();

    json jsonBody;
    for (auto &it : params)
        jsonBody["Found in Request Query Parameters"][it.first] = it.second;
    for (auto &it : body)
        jsonBody["Found in Request Body"][it.first] = it.second;

    response.setBody(jsonBody.dump());
}

void getHelloWorld(HttpRequest &request, HttpResponse &response) {
    response.setBody("Hello world!");
}

void getIndex(HttpRequest &request, HttpResponse &response) {
    std::ifstream ifs("../public/index.html");
    std::string str(std::istreambuf_iterator<char>{ifs}, {});

    response.setHeader("Content-Type", "text/html");
    response.setBody(str);
}

int main() {
    HttpServer server;
    
    HttpResource helloWorldResource;
    helloWorldResource.addRouteHandler(HttpMethod::GET, getHelloWorld);
    server.addResource("/hello", helloWorldResource);

    HttpResource echoResource;
    echoResource.addRouteHandler(HttpMethod::GET, echo);
    echoResource.addRouteHandler(HttpMethod::POST, echo);
    server.addResource("/echo", echoResource);

    HttpResource indexResource;
    indexResource.addRouteHandler(HttpMethod::GET, getIndex);
    server.addResource("/", indexResource);

    HttpResource fibonacciResource;
    fibonacciResource.addRouteHandler(HttpMethod::POST, postFibonacci);
    fibonacciResource.addRouteHandler(HttpMethod::GET, getFibonacci);
    server.addResource("/fibonacci", fibonacciResource);

    server.start();
    server.stop();
    return 0;
}