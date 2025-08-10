#include "./action.hpp"
#include "core/handler/read_cgi_response_handler.hpp"
#include "core/handler/write_cgi_request_handler.hpp"

#include <unistd.h>
#include <sys/socket.h>

extern char **environ;

// parent <-> child の双方向の IPC が必要
void RunCgiAction::execute(ActionContext &ctx) {
    int pair[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, pair) == -1) {
        // TODO?
        throw std::runtime_error("socketpair failed");
    }
    AutoFd socketParent(pair[0]);
    AutoFd socketChild(pair[1]);

    const pid_t childPid = fork();
    if (childPid == -1) {
        // TODO?
        throw std::runtime_error("fork failed");
    }
    if (childPid == 0) {
        // child
        // CGI の標準入出力を socket にする
        if (dup2(socketChild, STDIN_FILENO) == -1) {
            // TODO?
            throw std::runtime_error("dup2 failed");
        }
        if (dup2(socketChild, STDOUT_FILENO) == -1) {
            // TODO?
            throw std::runtime_error("dup2 failed");
        }

        // CGI を実行
        const char *cgiProgram = "/bin/cat";
        char *const argv[] = {const_cast<char *>(cgiProgram), NULL};
        execve(cgiProgram, argv, environ);

        // exec に失敗
        std::exit(1);
    }

    // parent
    // TODO: event handler は現状1つしか登録できない
    // FIXME: 無理やり Connection として扱うために、ダミーのアドレスを作成している
    const Address dummyLocal("127.0.0.1", 0);
    const Address dummyForeign("127.0.0.1", 0);
    ctx.getState().getConnectionRepository().set(socketParent, new Connection(socketParent, dummyLocal, dummyForeign));

    ctx.getState().getEventNotifier().registerEvent(Event(socketParent, Event::kRead | Event::kWrite));

    const Option<std::string> body = cgiRequest_.getBody();
    if (body.isSome()) {
        ctx.getState().getEventHandlerRepository().set(
            socketParent, Event::kWrite, new WriteCgiRequestBodyHandler(body.unwrap())
        );
    }
    ctx.getState().getEventHandlerRepository().set(socketParent, Event::kRead, new ReadCgiResponseHandler());
}
