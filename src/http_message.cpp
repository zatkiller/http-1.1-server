#include "http_message.h"
#include "utils.h"

#include <iostream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <string>
#include <vector>

namespace http_server {
    std::unordered_map<std::string, HttpVersion> httpVersionMapping = {
        {"HTTP/1.1", HttpVersion::HTTP_1_DOT_1},
    };

    std::unordered_map<std::string, HttpMethod> httpMethodMapping = {
        {"GET", HttpMethod::GET},
        {"POST", HttpMethod::POST},
        {"PUT", HttpMethod::PUT},
        {"DELETE", HttpMethod::DELETE},
    };

    HttpMethod getHttpMethod(const std::string &s) {
        return httpMethodMapping[s];
    }

    HttpVersion getHttpVersion(const std::string &s) {
        if (httpVersionMapping.find(s) == httpVersionMapping.end())
            throw std::runtime_error("Invalid Http Version!");

        return httpVersionMapping[s];
    }

    void HttpRequest::parseHeaders(std::vector<std::string> &data) {
        for (int i = 1; i < data.size(); i++) {
            if (data[i] == "")
                break;

            std::vector<std::string> headerLabelAndValue = split(data[i], ": ");
            headers_.emplace(headerLabelAndValue[0], headerLabelAndValue[1]);
        }
    }

    void HttpRequest::parseRequest(const std::string &bufferStr) {
        size_t pos = bufferStr.find(DELIMITER);
        
        if (pos == std::string::npos) {
            std::cout << "Error occurred when parsing request. Request: " << std::endl;
            std::cout << bufferStr << std::endl;
            hasError_ = true;
            return;
        }

        std::vector<std::string> results = split(bufferStr, "\n");
        
        for (int i = 0; i < results.size(); i++)
            results[i] = trim(results[i]);

        parseRequestLine(results[0]);
        parseHeaders(results);

        if (method_ == HttpMethod::POST) {
            int bodyIdx = 1;
            for (; bodyIdx < results.size(); bodyIdx++) {
                if (results[bodyIdx] == "") {
                    bodyIdx++;
                    break;
                }
            }

            parseBody(results[bodyIdx]);
        }
    }

    void HttpRequest::parseBody(const std::string &body) {
        try {
            nlohmann::json json_data = nlohmann::json::parse(body);
            for(auto it = json_data.begin(); it != json_data.end(); ++it) {
                // std::cout << "BODY K-V: "  << it.key() << " " << it.value() << std::endl;
                body_[it.key()] = it.value();
            }
        } catch (nlohmann::json::parse_error& err) {
            std::cout << "Error occured when parsing Json!" << std::endl;
        }
    }

    void HttpRequest::parseRequestLine(const std::string &line) {
        std::vector<std::string> results = split(line, " ");

        method_ = getHttpMethod(results[0]);
        parsePathAndParameters(results[1]);
        version_ = getHttpVersion(results[2]);
    }

    void HttpRequest::parsePathAndParameters(const std::string &line) {
        std::vector<std::string> results = split(line, "?");
        path_ = results[0];

        // query params are present
        if (results.size() > 1) {
            std::vector<std::string> params = split(results[1], "&");
            for (int i = 0; i < params.size(); i++) {
                std::vector<std::string> paramAndValue = split(params[i], "=");
                queryParams_.emplace(paramAndValue[0], paramAndValue[1]);
                // std::cout << "Query Param: " << paramAndValue[0] << "=" << paramAndValue[1] << std::endl;
            }
        }
    }

    std::string HttpResponse::toString() {
        std::ostringstream oss;
        
        std::string version = "HTTP/1.1";

        oss << version << " " << "200" << " " << statusMessage_ << DELIMITER;
        if (headers_.find("Content-Type") != headers_.end())
            oss << "Content-Type: " << headers_["Content-Type"] << DELIMITER;
        
        oss << "Content-Length: " << body_.size() << DELIMITER;
        for (auto &it : headers_) {
            if (it.first == "Content-Length" || it.second == "Content-Type")
                continue;
            oss << it.first << ": " << it.second << DELIMITER;
        }

        oss << DELIMITER;
        oss << body_;
        // std::cout << oss.str() << std::endl;
        return oss.str();
    }

    void HttpResponse::setAsMethodNotAllowed() {
        statusCode_ = HttpStatusCode::MethodNotAllowed;
        statusMessage_ = "Method Not Allowed";
        body_ = "Method not allowed";
    }

    void HttpResponse::setAsNotFound() {
        statusCode_ = HttpStatusCode::NotFound;
        statusMessage_ = "Not Found";
        body_ = "Page not Found";
    }

    void HttpResponse::setAsOk() {
        statusCode_ = HttpStatusCode::Ok;
        statusMessage_ = "OK";
    }
    
    void HttpResponse::setAsInternalServerErrror() {
        statusCode_ = HttpStatusCode::InternalServerError;
        statusMessage_ = "Internal Server Error";
        body_ = "Internal Server Error";
    }
}