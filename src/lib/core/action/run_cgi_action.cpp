#include <unistd.h>
#include <sys/socket.h>
#include <vector>
#include <map>
#include <cerrno>
#include "./action.hpp"
#include "core/handler/read_cgi_response_handler.hpp"
#include "core/handler/write_cgi_request_handler.hpp"
#include "../../cgi/meta_variable.hpp"
#include "utils/fd.hpp"
#include "utils/logger.hpp"
#include <cstdlib>

void RunCgiAction::execute(ActionContext &ctx) {
    int pair[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, pair) == -1) {
        throw std::runtime_error("socketpair failed");
    }
    AutoFd socketParent(pair[0]);
    AutoFd socketChild(pair[1]);

    utils::setNonBlocking(socketParent);
    utils::setCloseOnExec(socketParent);
    utils::setCloseOnExec(socketChild);

    LOG_DEBUGF("socketParent: %d, socketChild: %d", socketParent.get(), socketChild.get());

    LOG_DEBUG("Forking CGI process");
    const pid_t childPid = fork();
    if (childPid == -1) {
        throw std::runtime_error("fork failed");
    }

    if (childPid == 0) {
        this->childRoutine(socketChild.release());
    } else {
        this->parentRoutine(ctx, socketParent.release(), childPid);
    }
}

void RunCgiAction::childRoutine(const int socketFd) const {
    // CGI の標準入出力を socket にする
    if (dup2(socketFd, STDIN_FILENO) == -1) {
        throw std::runtime_error("dup2 failed");
    }
    if (dup2(socketFd, STDOUT_FILENO) == -1) {
        throw std::runtime_error("dup2 failed");
    }

    const std::vector<cgi::MetaVariable> &variables = cgiRequest_.getVariables();

    std::map<std::string, std::string> envMap;
    for (std::size_t i = 0; i < variables.size(); i++) {
        const std::string &k = variables[i].getName();
        const std::string &v = variables[i].getValue();
        envMap[k] = v;
    }

    // 環境変数を継承
    static const char *envWhitelist[] = {"PATH", "TZ", "LANG", "LC_ALL", "LC_CTYPE", NULL};
    for (const char **envVar = envWhitelist; *envVar != NULL; ++envVar) {
        const char *value = std::getenv(*envVar);
        if (value != NULL) {
            envMap[*envVar] = value;
        }
    }
    if (envMap.find("PATH") == envMap.end()) {
        envMap["PATH"] = "/usr/local/bin:/bin:/usr/bin";
    }

    std::vector<std::string> envStrings;
    envStrings.reserve(envMap.size());
    for (std::map<std::string, std::string>::const_iterator it = envMap.begin(); it != envMap.end(); ++it) {
        envStrings.push_back(it->first + "=" + it->second);
    }
    std::vector<char *> envp;
    envp.reserve(envStrings.size() + 1);
    for (std::size_t i = 0; i < envStrings.size(); i++) {
        envp.push_back(const_cast<char *>(envStrings[i].c_str()));
    }
    envp.push_back(NULL);

    // SCRIPT_NAME と DOCUMENT_ROOT を探して CGI プログラムパスを取得
    std::string scriptName;
    std::string documentRoot;
    for (size_t i = 0; i < variables.size(); i++) {
        if (variables[i].getName() == "SCRIPT_NAME") {
            scriptName = variables[i].getValue();
        } else if (variables[i].getName() == "DOCUMENT_ROOT") {
            documentRoot = variables[i].getValue();
        }
    }

    // TODO: エラーハンドリングはこれでいい?
    if (scriptName.empty() || documentRoot.empty()) {
        std::exit(126);
    }

    // ドキュメントルートとスクリプト名を結合して実際のファイルパスを生成
    const std::string cgiProgram = scriptName[0] == '/' ? documentRoot + scriptName : documentRoot + "/" + scriptName;

    // CGI を実行
    char *const argv[] = {const_cast<char *>(cgiProgram.c_str()), NULL};
    execve(cgiProgram.c_str(), argv, envp.data());

    // exec に失敗
    std::exit(errno == ENOENT ? 127 : 126);
}

void RunCgiAction::parentRoutine(const ActionContext &ctx, const int socketFd, const pid_t childPid) const {
    // FIXME: 無理やり Connection として扱うために、ダミーのアドレスを作成している
    const Address dummyAddr("127.0.0.1", 0);
    ctx.getState().getConnectionRepository().set(socketFd, new Connection(socketFd, dummyAddr, dummyAddr));

    // 子プロセスからの出力の読み込みを待つ
    uint32_t eventTypeFlag = Event::kRead;
    ctx.getState().getEventHandlerRepository().set(socketFd, Event::kRead, new ReadCgiResponseHandler(clientFd_));

    const Option<std::string> body = cgiRequest_.getBody();
    if (body.isSome()) {
        // 子プロセスへの body の書き込みを待つ
        eventTypeFlag |= Event::kWrite;
        ctx.getState().getEventHandlerRepository().set(
            socketFd, Event::kWrite, new WriteCgiRequestBodyHandler(body.unwrap())
        );
    } else {
        shutdown(socketFd, SHUT_WR);
    }

    // 子プロセスとの間のソケットのイベントを待つ
    ctx.getState().getEventNotifier().registerEvent(Event(socketFd, eventTypeFlag));

    // HTTP レスポンスが準備できるまで、peer に関するイベント待ちなどは解除
    ctx.getState().getEventNotifier().unregisterEvent(Event(clientFd_, Event::kRead));
    ctx.getState().getEventNotifier().unregisterEvent(Event(clientFd_, Event::kWrite));
    ctx.getState().getEventHandlerRepository().remove(clientFd_, Event::kRead);
    ctx.getState().getEventHandlerRepository().remove(clientFd_, Event::kWrite);

    CgiProcessRepository::Data data = {clientFd_, socketFd};
    ctx.getState().getCgiProcessRepository().set(childPid, data);
}
