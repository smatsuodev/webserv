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

    static std::string makeAppropriatePath(const std::string &path) {
        std::string result;
        bool lastWasSlash = false;

        for (std::size_t i = 0; i < path.size(); i++) {
            const char c = path[i];
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

    std::string StaticFileHandler::getNextLink(const std::string &newPath, struct dirent *dp) {
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
        return nextLink;
    }

    Response StaticFileHandler::directoryListing(const std::string &path) {
        DIR *dir = opendir(path.c_str());
        if (dir == nullptr) {
            LOG_DEBUGF("failed to open directory: %s", path.c_str());
            return ResponseBuilder().status(kStatusInternalServerError).build();
        }

        std::string newPath = makeAppropriatePath(path);
        std::string title = "Index of " + newPath;

        std::string res = "<html><head><title>" + title + "</title></head>";
        res += "<body><h1>" + title + "</h1><hr>";

        res += "<ul>";
        struct dirent *dp;
        while ((dp = readdir(dir)) != NULL) {
            res += "<li>";
            res += "<a href=\"" + getNextLink(newPath, dp) + "\">";
            res += dp->d_name;
            if (dp->d_type == DT_DIR) {
                res += "/";
            }
            res += "</a>";
            // TODO: ファイルサイズ、更新日時
            res += "</li>";
        }
        closedir(dir);
        res += "</ul>";

        res += "</body></html>";
        return ResponseBuilder().html(res).build();
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
            return directoryListing(path);
        }

        if (!S_ISREG(buf.st_mode)) {
            LOG_DEBUGF("is not a regular file: %s", path.c_str());
            return ResponseBuilder().status(kStatusForbidden).build();
        }

        return ResponseBuilder().file(path).build();
    }
}
