#include "handler.hpp"
#include "utils/logger.hpp"
#include "http/response/response_builder.hpp"
#include <fstream>
#include <dirent.h>
#include <sys/stat.h>
#include "static_file_handler.hpp"

#include "http/mime.hpp"

namespace http {
    StaticFileHandler::StaticFileHandler(const config::LocationContext::DocumentRootConfig &docRootConfig)
        : docRootConfig_(docRootConfig) {}

    static std::string removeDuplicateSlashes(const std::string &path) {
        std::string result;
        bool lastWasSlash = false;

        for (char c : path) {
            if (c == '/') {
                if (!lastWasSlash) {
                    result += c;
                    lastWasSlash = true;
                }
            } else {
                result += c;
                lastWasSlash = false;
            }
        }
        if (result.size() > 1 && result.back() == '/') {
            result.pop_back();
        }
        if (result.size() > 1 && result.front() == '.') {
            result = result.substr(1);
        }
        return result;
    }

    Response StaticFileHandler::directoryListing(const std::string &path) {
        std::string newPath = removeDuplicateSlashes(path);

        std::string h1 = "Index of " + newPath;
        std::string body = "<html><head><title>" + h1 + "</title></head><body><h1>" + h1 + "</h1><ul>";
        body += "<hr>";
        DIR *dir = opendir(path.c_str());
        if (dir == nullptr) {
            LOG_DEBUGF("failed to open directory: %s", path.c_str());
            return ResponseBuilder().status(kStatusInternalServerError).build();
        }

        struct dirent *dp;
        while ((dp = readdir(dir)) != nullptr) {
            std::string nextLink;
            if (dp->d_name == std::string(".") || dp->d_name == std::string("..")) {
                if (dp->d_name == std::string(".")) {
                    nextLink = newPath;
                } else {
                    nextLink = newPath.substr(0, newPath.find_last_of('/'));
                    if (nextLink.empty()) {
                        nextLink = "/";
                    }
                }
            } else {
                nextLink = newPath + "/" + dp->d_name;
            }
            LOG_WARNF("%s -> %s", dp->d_name, nextLink.c_str());

            body += "<li><a href=\"";
            body += nextLink;
            body += "\">";
            body += dp->d_name;
            if (dp->d_type == DT_DIR) {
                body += "/";
            }
            body += "</a>";
            // TODO: ファイルサイズ、更新日時
            body += "</li>";
        }
        closedir(dir);

        body += "</ul></body></html>";
        return ResponseBuilder().html(body).build();
    }

    Response StaticFileHandler::serve(const Request &req) {
        const std::string path = docRootConfig_.getRoot() + '/' + req.getRequestTarget();
        LOG_DEBUGF("request target: %s", req.getRequestTarget().c_str());

        struct stat buf = {};
        if (stat(path.c_str(), &buf) == -1) {
            if (errno == ENOENT) {
                LOG_DEBUGF("file does not exist: %s", path.c_str());
                return ResponseBuilder().status(kStatusNotFound).build();
            }
            LOG_DEBUGF("failed to stat file: %s", path.c_str());
            return ResponseBuilder().status(kStatusInternalServerError).build();
        }

        if (S_ISDIR(buf.st_mode)) {
            LOG_DEBUGF("is a directory: %s", path.c_str());
            // TODO: directory listing
            return directoryListing(path);
            // return ResponseBuilder().status(kStatusForbidden).build();
        }

        if (!S_ISREG(buf.st_mode)) {
            LOG_DEBUGF("is not a regular file: %s", path.c_str());
            return ResponseBuilder().status(kStatusForbidden).build();
        }

        return ResponseBuilder().file(path).build();
    }
}
