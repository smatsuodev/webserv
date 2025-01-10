#include "./mime.hpp"

namespace http {
    std::string getMimeType(const std::string &fileName) {
        const std::size_t lastDot = fileName.find_last_of('.');
        if (lastDot == std::string::npos) {
            return "application/octet-stream";
        }
        const std::string extension = fileName.substr(lastDot + 1);

        if (extension == "html" || extension == "htm") {
            return "text/html";
        }
        if (extension == "css") {
            return "text/css";
        }
        if (extension == "js") {
            return "application/javascript";
        }
        if (extension == "json") {
            return "application/json";
        }
        if (extension == "png") {
            return "image/png";
        }
        if (extension == "jpg" || extension == "jpeg") {
            return "image/jpeg";
        }
        if (extension == "txt") {
            return "text/plain";
        }

        return "application/octet-stream";
    }
}
