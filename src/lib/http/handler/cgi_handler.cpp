#include "cgi_handler.hpp"
#include "cgi/factory.hpp"
#include "cgi/request.hpp"
#include "core/action/action.hpp"
#include "http/response/response_builder.hpp"
#include "utils/types/try.hpp"
#include "utils/string.hpp"
#include <sys/stat.h>

// めんどくさくてメソッドにしてない
static bool fileExistsUnderRoot(const std::string &docRoot, const std::string &urlPath) {
    std::string fsPath = docRoot;
    if (!fsPath.empty() && fsPath.back() != '/') fsPath += '/';
    if (!urlPath.empty() && urlPath[0] == '/')
        fsPath += urlPath.substr(1);
    else
        fsPath += urlPath;

    struct stat st = {};
    return ::stat(fsPath.c_str(), &st) == 0 && S_ISREG(st.st_mode);
}

namespace http {
    Either<IAction *, Response> CgiHandler::serve(const RequestContext &ctx) {
        if (!this->isCgiRequest(ctx)) {
            return fallbackHandler_->serve(ctx);
        }

        const Result<cgi::Request, error::AppError> &createResult = this->createCgiRequest(ctx);
        if (createResult.isErr()) {
            return Right(ResponseBuilder().status(kStatusInternalServerError).build());
        }
        const cgi::Request &cgiRequest = createResult.unwrap();

        return Left(new RunCgiAction(cgiRequest, ctx.getConnection().get().getFd()));
    }

    bool CgiHandler::isCgiRequest(const RequestContext &ctx) const {
        const std::string &path = ctx.getRequest().getRequestTarget();

        // クエリ文字列を除去
        const size_t queryPos = path.find('?');
        const std::string pathWithoutQuery = queryPos != std::string::npos ? path.substr(0, queryPos) : path;

        const std::vector<std::string> &cgiExtensions = docRootConfig_.getCgiExtensions();
        if (cgiExtensions.empty()) {
            return false;
        }

        // 設定された拡張子と照合
        for (std::vector<std::string>::const_iterator it = cgiExtensions.begin(); it != cgiExtensions.end(); ++it) {
            const std::string &ext = *it;
            if (pathWithoutQuery.length() >= ext.length() &&
                pathWithoutQuery.compare(pathWithoutQuery.length() - ext.length(), ext.length(), ext) == 0) {

                // 拡張子は一致。スクリプト本体の存在確認（PATH_INFO を除いたもの）
                const std::string script = this->getScriptName(ctx); // 例: /cgi/test.cgi

                if (fileExistsUnderRoot(docRootConfig_.getRoot(), script)) {
                    return true; // 実体があるので CGI
                }
                return false; // 実体が無ければ CGI ではない
            }
        }

        return false;
    }

    Result<cgi::Request, error::AppError> CgiHandler::createCgiRequest(const RequestContext &ctx) const {
        const cgi::RequestFactory::Parameter param = {
            ctx.getRequest(),
            ctx.getConnection().get().getForeignAddress(),
            serverName_,
            utils::toString(ctx.getConnection().get().getLocalAddress().getPort()),
            this->getScriptName(ctx),
            this->getPathInfo(ctx),
            docRootConfig_.getRoot()
        };
        return cgi::RequestFactory::create(param);
    }

    std::string CgiHandler::getScriptName(const RequestContext &ctx) const {
        const std::string &path = ctx.getRequest().getRequestTarget();

        // クエリ文字列を除去
        const size_t queryPos = path.find('?');
        std::string scriptPath = queryPos != std::string::npos ? path.substr(0, queryPos) : path;

        // PATH_INFOがある場合は、スクリプト名部分のみ返す
        // 例: /cgi/test.cgi/extra/path -> /cgi/test.cgi
        const std::vector<std::string> &cgiExtensions = docRootConfig_.getCgiExtensions();

        // 各拡張子で検索
        for (std::vector<std::string>::const_iterator it = cgiExtensions.begin(); it != cgiExtensions.end(); ++it) {
            const std::string &ext = *it;
            const size_t extPos = scriptPath.find(ext);
            if (extPos != std::string::npos && extPos + ext.length() <= scriptPath.length()) {
                return scriptPath.substr(0, extPos + ext.length());
            }
        }

        return scriptPath;
    }

    std::string CgiHandler::getPathInfo(const RequestContext &ctx) const {
        const std::string &path = ctx.getRequest().getRequestTarget();

        // クエリ文字列を除去
        const size_t queryPos = path.find('?');
        std::string fullPath = queryPos != std::string::npos ? path.substr(0, queryPos) : path;

        // スクリプト名の後の追加パスを取得
        // 例: /cgi/test.cgi/extra/path -> /extra/path
        const std::vector<std::string> &cgiExtensions = docRootConfig_.getCgiExtensions();

        // 各拡張子で検索
        for (std::vector<std::string>::const_iterator it = cgiExtensions.begin(); it != cgiExtensions.end(); ++it) {
            const std::string &ext = *it;
            const size_t extPos = fullPath.find(ext);
            if (extPos != std::string::npos && extPos + ext.length() < fullPath.length()) {
                return fullPath.substr(extPos + ext.length());
            }
        }

        return "";
    }
}
