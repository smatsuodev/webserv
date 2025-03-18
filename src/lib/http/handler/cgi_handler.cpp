#include "cgi_handler.hpp"

#include "http/response/response_builder.hpp"
#include "utils/logger.hpp"
#include "utils/string.hpp"
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>

namespace http {
    CgiHandler::CgiHandler(const config::LocationContext::DocumentRootConfig &docRootConfig, IHandler *next)
        : docRootConfig_(docRootConfig), next_(next) {}

    Response CgiHandler::serve(const Request &req) {
        const Matcher<CgiExtension> matcher = this->createMatcher();
        const std::string &requestTarget = req.getRequestTarget();
        const Option<CgiExtension> matchResult = matcher.match(requestTarget);
        if (matchResult.isNone()) {
            return next_->serve(req);
        }
        int fd[2];

        if (pipe(fd) == -1) {
            return ResponseBuilder().status(kStatusInternalServerError).build();
        }

        const int pid = fork();
        if (pid == -1) {
            close(fd[0]);
            close(fd[1]);
            return ResponseBuilder().status(kStatusInternalServerError).build();
        }

        if (pid == 0) {
            close(fd[0]);

            const std::string fullPath = docRootConfig_.getRoot() + requestTarget;
            char *fullPathCStr = strdup(fullPath.c_str());
            if (fullPathCStr == NULL) {
                close(fd[1]);
                std::exit(errno);
            }

            close(STDOUT_FILENO);
            dup2(fd[1], STDOUT_FILENO);

            char *argv[] = {fullPathCStr, NULL};
            execve(fullPathCStr, argv, environ);
            std::exit(errno);
        }

        close(fd[1]);
        int cgiStatus = 0;
        const int waitpidRes = waitpid(pid, &cgiStatus, WUNTRACED);
        if (waitpidRes == -1) {
            LOG_ERRORF("failed to execute CGI: %s", pid, strerror(errno));
            close(fd[1]);
            return ResponseBuilder().status(kStatusInternalServerError).build();
        }
        if (WEXITSTATUS(cgiStatus) != 0) {
            LOG_ERRORF("CGI did not terminate successfully: %d", WEXITSTATUS(cgiStatus));
            close(fd[1]);
            return ResponseBuilder().status(kStatusInternalServerError).build();
        }

        std::stringstream body;
        char buf[1024];
        ssize_t bytesRead = 0;

        while ((bytesRead = read(fd[0], buf, sizeof(buf))) > 0) {
            body.write(buf, bytesRead);
            body.flush();
        }
        close(fd[0]);

        if (bytesRead == -1) {
            LOG_ERRORF("failed to read CGI: %s", strerror(errno));
            return ResponseBuilder().status(kStatusInternalServerError).build();
        }

        return ResponseBuilder().status(kStatusOk).text(body.str()).build();
    }

    Matcher<CgiHandler::CgiExtension> CgiHandler::createMatcher() const {
        std::map<CgiExtension, CgiExtension> exts;
        CgiExtensions cgiExtensions = docRootConfig_.getCgiExtensions();
        for (CgiExtensions::const_iterator it = cgiExtensions.begin(); it != cgiExtensions.end(); ++it) {
            exts[*it] = *it;
        }
        return Matcher<CgiExtension>(exts, utils::endsWith);
    }
}
