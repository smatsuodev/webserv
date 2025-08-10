#include "./action.hpp"
#include "core/handler/read_cgi_response_handler.hpp"
#include "core/handler/write_cgi_request_handler.hpp"

#include <unistd.h>
#include <sys/socket.h>

#include "../../cgi/meta_variable.hpp"

#include "utils/log.hpp"
extern char **environ;

// parent <-> child の双方向の IPC が必要
void RunCgiAction::execute(ActionContext &ctx) {
    int pair[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, pair) == -1) {
        throw std::runtime_error("socketpair failed");
    }
    AutoFd socketParent(pair[0]);
    AutoFd socketChild(pair[1]);

    LOG_DEBUG("Forking CGI process");
    const pid_t childPid = fork();
    if (childPid == -1) {
        throw std::runtime_error("fork failed");
    }
    if (childPid == 0) {
        // child
        // CGI の標準入出力を socket にする
        if (dup2(socketChild, STDIN_FILENO) == -1) {
            throw std::runtime_error("dup2 failed");
        }
        if (dup2(socketChild, STDOUT_FILENO) == -1) {
            throw std::runtime_error("dup2 failed");
        }

        // CGI メタ変数を環境変数に変換
        const std::vector<cgi::MetaVariable> &variables = cgiRequest_.getVariables();
        std::vector<std::string> envStrings;
        std::vector<char *> envp;

        // 既存の環境変数を追加
        for (char **env = environ; *env != NULL; env++) {
            envStrings.push_back(*env);
        }

        // CGI メタ変数を追加
        for (size_t i = 0; i < variables.size(); i++) {
            envStrings.push_back(variables[i].getName() + "=" + variables[i].getValue());
        }

        // char* 配列に変換
        for (size_t i = 0; i < envStrings.size(); i++) {
            envp.push_back(const_cast<char *>(envStrings[i].c_str()));
        }
        envp.push_back(NULL);

        // SCRIPT_NAME と DOCUMENT_ROOT を探して CGI プログラムパスを取得
        std::string cgiProgram = "/bin/cat"; // デフォルト
        std::string scriptName;
        std::string documentRoot;
        for (size_t i = 0; i < variables.size(); i++) {
            if (variables[i].getName() == "SCRIPT_NAME") {
                scriptName = variables[i].getValue();
            } else if (variables[i].getName() == "DOCUMENT_ROOT") {
                documentRoot = variables[i].getValue();
            }
        }

        // ドキュメントルートとスクリプト名を結合して実際のファイルパスを生成
        if (!scriptName.empty() && !documentRoot.empty()) {
            // スクリプト名の先頭スラッシュを除去してドキュメントルートと結合
            if (scriptName[0] == '/') {
                cgiProgram = documentRoot + scriptName;
            } else {
                cgiProgram = documentRoot + "/" + scriptName;
            }
        } else if (!scriptName.empty()) {
            // ドキュメントルートが指定されていない場合は、従来のフォールバック
            if (scriptName.find("/cgi/") == 0) {
                cgiProgram = "example" + scriptName;
            }
        }

        // CGI を実行
        LOG_DEBUGF("Executing CGI program: %s", cgiProgram.c_str());
        char *const argv[] = {const_cast<char *>(cgiProgram.c_str()), NULL};
        execve(cgiProgram.c_str(), argv, envp.data());

        // exec に失敗
        LOG_ERRORF("Failed to execute CGI program: %s", cgiProgram.c_str());
        std::exit(1);
    }

    // parent
    // FIXME: 無理やり Connection として扱うために、ダミーのアドレスを作成している
    const Address dummyLocal("127.0.0.1", 0);
    const Address dummyForeign("127.0.0.1", 0);
    ctx.getState().getConnectionRepository().set(socketParent, new Connection(socketParent, dummyLocal, dummyForeign));
    
    // AutoFdの管理から外す（ConnectionがFDを管理するため）
    const int socketFd = socketParent.release();

    ctx.getState().getEventNotifier().registerEvent(Event(socketFd, Event::kRead | Event::kWrite));

    const Option<std::string> body = cgiRequest_.getBody();
    if (body.isSome()) {
        ctx.getState().getEventHandlerRepository().set(
            socketFd, Event::kWrite, new WriteCgiRequestBodyHandler(body.unwrap())
        );
    }
    ctx.getState().getEventHandlerRepository().set(
        socketFd, Event::kRead, new ReadCgiResponseHandler(clientFd_, childPid)
    );

    // クライアント接続のイベントハンドラーを削除（無限ループ防止）
    ctx.getState().getEventNotifier().unregisterEvent(Event(clientFd_, Event::kRead));
    ctx.getState().getEventHandlerRepository().remove(clientFd_, Event::kRead);
    ctx.getState().getEventHandlerRepository().remove(clientFd_, Event::kWrite);
}
