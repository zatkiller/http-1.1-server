#pragma once

#include <iostream>
#include <string>
#include <vector>

#define BUFFER_SIZE 2048

namespace http_server {
    const std::string WHITESPACE = " \n\r\t\f\v";

    std::vector<std::string> split(const std::string s, const std::string &delimiter);
    void printBuffer(const std::string &buffer);
    void printV(const std::vector<std::string> &v);

    std::string ltrim(const std::string &s);
    std::string rtrim(const std::string &s);
    std::string trim(const std::string &s);
}