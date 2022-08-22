#pragma once

#include "utils.h"

#include <chrono>
#include <string>
#include <unordered_map>

#define DELIMITER "\r\n"

namespace http_server {
enum class HttpMethod{
    GET,
    POST,
    DELETE,
    PUT,
};

enum class HttpVersion {
    HTTP_1_DOT_1 = 1,
};

enum class HttpStatusCode {
    Ok = 200,
    Created = 201,
    NotFound = 404,
    MethodNotAllowed = 405,
    InternalServerError = 500,
};

extern std::unordered_map<std::string, HttpVersion> httpVersionMapping;
extern std::unordered_map<std::string, HttpMethod> httpMethodMapping;

HttpMethod getHttpMethod(const std::string &s);
HttpVersion getHttpVersion(const std::string &s);

class HttpMessage {
public:
    HttpMessage(): version_(HttpVersion::HTTP_1_DOT_1) {}

    HttpVersion getVersion() { return version_; }
    void setHeader(const std::string &key, const std::string &value) { headers_.emplace(key, value); }

protected:
    HttpVersion version_;
    std::unordered_map<std::string, std::string> headers_;
};

class HttpRequest : public HttpMessage {
public:
    HttpRequest(): method_(HttpMethod::GET) {}

    void parseRequest(const std::string &bufferStr);
    void parseRequestLine(const std::string &line);
    void parsePathAndParameters(const std::string &line);
    void parseHeaders(std::vector<std::string> &data);
    void parseBody(const std::string &body);

    HttpMethod getMethod() { return method_; }
    std::string getPath() { return path_; }
    std::unordered_map<std::string, std::string> getQueryParams() { return queryParams_; }
    std::unordered_map<std::string, std::string> getBody() { return body_; }
    std::chrono::time_point<std::chrono::high_resolution_clock> getCreated() { return created_; }
    bool hasError() { return hasError_; }

    void setCreatedTime() { created_ = std::chrono::high_resolution_clock::now(); }

private:
    HttpMethod method_;
    std::string path_;
    std::unordered_map<std::string, std::string> queryParams_;
    std::unordered_map<std::string, std::string> body_;
    std::chrono::time_point<std::chrono::high_resolution_clock> created_;
    bool hasError_ = false;
};

class HttpResponse : public HttpMessage {
public:
    HttpResponse(): statusCode_(HttpStatusCode::Ok) {}
    HttpResponse(HttpStatusCode statusCode) : statusCode_(statusCode) {}

    void setStatusCode(HttpStatusCode status) { statusCode_ = status; }
    void setStatusMessage(const std::string& message) { statusMessage_ = message; }    
    void setBody(const std::string& body) { body_ = body; }
    std::string toString();

    void setAsMethodNotAllowed();
    void setAsNotFound();
    void setAsOk();
    void setAsInternalServerErrror();
    void setCompletedTime() { completed_ = std::chrono::high_resolution_clock::now(); }

    std::chrono::time_point<std::chrono::high_resolution_clock> getCompleted() { return completed_; }

private:
    HttpStatusCode statusCode_;
    std::string statusMessage_;
    std::string body_;
    std::chrono::time_point<std::chrono::high_resolution_clock> completed_;
};

}