#include "./mime.hpp"

namespace http {
    std::string getMimeType(const std::string &fileName) {
        const std::size_t lastDot = fileName.find_last_of('.');
        if (lastDot == std::string::npos) {
            return "application/octet-stream";
        }
        // 拡張子を取り出して小文字にする
        std::string extension = fileName.substr(lastDot + 1);
        std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

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
