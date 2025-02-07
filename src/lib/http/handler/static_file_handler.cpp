#include "handler.hpp"
#include "utils/logger.hpp"
#include "http/response/response_builder.hpp"
#include <fstream>
#include <dirent.h>
#include <cstring>
#include <sys/stat.h>
#include "static_file_handler.hpp"

#include "http/mime.hpp"

namespace http {
    StaticFileHandler::StaticFileHandler(const config::LocationContext::DocumentRootConfig &docRootConfig)
        : docRootConfig_(docRootConfig) {}

    Result<std::string, HttpStatusCode>
    StaticFileHandler::makeDirectoryListingHtml(const std::string &root, const std::string &target) {
        DIR *dir = opendir((root + target).c_str());
        if (dir == NULL) {
            LOG_DEBUGF("failed to open directory: %s", (root + target).c_str());
            return Err(kStatusInternalServerError);
        }
        const std::string title = "Index of " + target;
        std::string res = "<!DOCTYPE html>";
        res += "<html>";
        res += "<head><title>" + title + "</title></head>";
        res += "<body>";
        res += "<h1>" + title + "</h1>";
        res += "<hr>";
        res += "<ul>";
        const dirent *dp;
        while ((dp = readdir(dir)) != NULL) {
            const std::string dName = dp->d_name;
            if (dName == ".") {
                continue;
            }
            const std::string dNameLink = dName + (dp->d_type == DT_DIR ? "/" : "");
            res += "<li>";
            res += "<a href=\"" + dNameLink + "\">";
            res += dName;
            if (dp->d_type == DT_DIR) {
                res += "/";
            }
            res += "</a>";
            res += "</li>";
        }
        closedir(dir);
        res += "</ul>";
        res += "<hr>";
        res += "</body>";
        res += "</html>";
        return Ok(res);
    }

    Response StaticFileHandler::directoryListing(const std::string &root, const std::string &target) {
        const Result<std::string, HttpStatusCode> result = makeDirectoryListingHtml(root, target);
        if (result.isErr()) {
            return ResponseBuilder().status(result.unwrapErr()).build();
        }
        const std::string res = result.unwrap();
        return ResponseBuilder().html(res).build();
    }

    Response StaticFileHandler::serve(const Request &req) {
        // root が '/' で終わると、log で '//' が発生する。
        const std::string path = docRootConfig_.getRoot() + req.getRequestTarget();
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
            const std::string indexPath = path + '/' + docRootConfig_.getIndex();
            struct stat indexBuf = {};
            if (stat(indexPath.c_str(), &indexBuf) == -1) {
                LOG_ERRORF("failed to stat file: %s", std::strerror(errno));
                return ResponseBuilder().status(kStatusInternalServerError).build();
            }
            if (S_ISREG(indexBuf.st_mode)) {
                return ResponseBuilder().file(indexPath).build();
            }
            if (docRootConfig_.isAutoindexEnabled()) {
                return directoryListing(docRootConfig_.getRoot(), req.getRequestTarget());
            }
            return ResponseBuilder().status(kStatusForbidden).build();
        }

        if (!S_ISREG(buf.st_mode)) {
            LOG_DEBUGF("is not a regular file: %s", path.c_str());
            return ResponseBuilder().status(kStatusForbidden).build();
        }

        return ResponseBuilder().file(path).build();
    }
}
