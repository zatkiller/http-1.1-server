#include "utils.h"

namespace http_server {
    
    std::vector<std::string> split(std::string s, const std::string &delimiter) {
        std::vector<std::string> result;

        size_t pos = 0;
        std::string token;
        while ((pos = s.find(delimiter)) != std::string::npos) {
            token = s.substr(0, pos);
            s.erase(0, pos + delimiter.length());
            result.push_back(token);
        }  
        result.push_back(s);
        return result;
    }

    void printBuffer(const std::string &buffer) {
        for (int i = 0; i < BUFFER_SIZE; i++) {
            if (buffer[i] == 0)
                break;
            std::cout << buffer[i];    
        }
    }

    void printV(const std::vector<std::string> &v) {
        for (int i = 0; i < v.size(); i++) {
            std::cout << v[i] << std::endl;
        } 
    }

    std::string ltrim(const std::string &s) {
        size_t start = s.find_first_not_of(WHITESPACE);
        return (start == std::string::npos) ? "" : s.substr(start);
    }
 
    std::string rtrim(const std::string &s) {
        size_t end = s.find_last_not_of(WHITESPACE);
        return (end == std::string::npos) ? "" : s.substr(0, end + 1);
    }
    
    std::string trim(const std::string &s) {
        return rtrim(ltrim(s));
    }
}