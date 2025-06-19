#include "./action.hpp"
#include "utils/logger.hpp"
#include <sys/socket.h>
#include <unistd.h>
#include <sys/wait.h>
#include <cstdlib>

void RunCgiAction::execute(ActionContext &ctx) {
    (void)ctx; // 未使用パラメータの警告を回避
    int sockfds[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockfds) == -1) {
        LOG_ERROR("socketpair failed");
        return;
    }

    pid_t pid = fork();
    if (pid == -1) {
        LOG_ERROR("fork failed");
        close(sockfds[0]);
        close(sockfds[1]);
        return;
    }

    if (pid == 0) {
        // 子プロセス: CGIスクリプトを実行
        close(sockfds[0]); // 親プロセス用のソケットを閉じる

        // 標準入出力をソケットに接続
        dup2(sockfds[1], STDIN_FILENO);
        dup2(sockfds[1], STDOUT_FILENO);
        close(sockfds[1]);

        // 環境変数を設定
        const std::vector<cgi::MetaVariable> &variables = cgiRequest_.getVariables();
        for (std::vector<cgi::MetaVariable>::const_iterator it = variables.begin(); it != variables.end(); ++it) {
            if (setenv(it->getName().c_str(), it->getValue().c_str(), 1) != 0) {
                LOG_ERROR("setenv failed for " + it->getName());
                _exit(1);
            }
        }

        // CGIスクリプトを実行
        // TODO: SCRIPT_FILENAMEから実際のファイルパスを取得
        const char *scriptPath = getenv("SCRIPT_FILENAME");
        if (!scriptPath) {
            LOG_ERROR("SCRIPT_FILENAME not found");
            _exit(1);
        }

        execl(scriptPath, scriptPath, NULL);
        LOG_ERROR("execl failed");
        _exit(1);
    } else {
        // 親プロセス: CGIプロセスとの通信を管理
        close(sockfds[1]); // 子プロセス用のソケットを閉じる

        // TODO: 非ブロッキングI/OでWriteCgiRequestHandlerとReadCgiResponseHandlerに委譲
        // 今は簡単な実装として同期的に処理

        // リクエストボディをCGIプロセスに送信
        const Option<std::string> &body = cgiRequest_.getBody();
        if (body.isSome()) {
            const std::string &bodyStr = body.unwrap();
            write(sockfds[0], bodyStr.c_str(), bodyStr.length());
        }

        // CGIプロセスからのレスポンスを受信
        // TODO: 実際の実装では非ブロッキングI/Oを使用

        close(sockfds[0]);

        // 子プロセスの終了を待機（非ブロッキングで）
        int status;
        waitpid(pid, &status, WNOHANG);
    }
}
