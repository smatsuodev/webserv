#include "cgi_handler.hpp"

#include "http/response/response_builder.hpp"
#include "utils/auto_fd.hpp"
#include "utils/logger.hpp"
#include "utils/string.hpp"
#include <cstring>
#include <fcntl.h>
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

        const std::string fullPath = docRootConfig_.getRoot() + requestTarget;
        char *fullPathCStr = static_cast<char *>(malloc((fullPath.size() + 1) * sizeof(char)));
        if (fullPathCStr == NULL) {
            LOG_ERRORF("failed to duplicate path: %s", strerror(errno));
            return ResponseBuilder().status(kStatusInternalServerError).build();
        }
        std::strcpy(fullPathCStr, fullPath.c_str());

        int fd[2];
        if (pipe(fd) == -1) {
            LOG_ERRORF("failed to create pipe: %s", strerror(errno));
            return ResponseBuilder().status(kStatusInternalServerError).build();
        }

        AutoFd pipeIn(fd[1]), pipeOut(fd[0]);
        LOG_DEBUGF("pipeIn: %d, pipeOut: %d", fd[1], fd[0]);

        const int pid = fork();
        if (pid == -1) {
            LOG_ERRORF("failed to create fork: %s", strerror(errno));
            return ResponseBuilder().status(kStatusInternalServerError).build();
        }

        if (pid == 0) {
            signal(SIGCHLD, SIG_IGN);
            if (fcntl(pipeIn.get(), F_SETFL, FD_CLOEXEC) == -1) {
                LOG_ERRORF("failed to set FD_CLOEXEC on pipe: %s", strerror(errno));
                close(pipeIn.release());
                return ResponseBuilder().status(kStatusInternalServerError).build();
            }

            close(pipeOut.release());
            dup2(pipeIn.get(), STDOUT_FILENO);

            char *argv[] = {fullPathCStr, NULL};
            execve(fullPathCStr, argv, environ);
            std::exit(errno);
        }

        close(pipeIn.release());
        free(fullPathCStr);

        // TODO: 子プロセスがエラーだったらハンドリングする。（ノンブロッキングで処理する必要がある）
        // int cgiStatus = 0;
        // const int waitpidRes = waitpid(pid, &cgiStatus, WUNTRACED);
        // if (waitpidRes == -1) {
        //     LOG_ERRORF("failed to execute CGI: %s", pid, strerror(errno));
        //     close(fd[1]);
        //     return ResponseBuilder().status(kStatusInternalServerError).build();
        // }
        // if (WEXITSTATUS(cgiStatus) != 0) {
        //     LOG_ERRORF("CGI did not terminate successfully: %d", WEXITSTATUS(cgiStatus));
        //     close(fd[1]);
        //     return ResponseBuilder().status(kStatusInternalServerError).build();
        // }

        std::stringstream body;
        char buf[1024];
        ssize_t bytesRead = 0;

        while ((bytesRead = read(pipeOut.get(), buf, sizeof(buf))) > 0) {
            body.write(buf, bytesRead);
            body.flush();
        }

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
