#include "cgi_handler.hpp"
#include "cgi/factory.hpp"
#include "cgi/request.hpp"
#include "core/action/action.hpp"
#include "http/response/response_builder.hpp"
#include "utils/types/try.hpp"

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

        return Left(new RunCgiAction(cgiRequest));
    }

    bool CgiHandler::isCgiRequest(const RequestContext &ctx) const {
        const std::string &path = ctx.getRequest().getRequestTarget();
        const std::vector<std::string> &cgiExtensions = docRootConfig_.getCgiExtensions();

        for (std::vector<std::string>::const_iterator it = cgiExtensions.begin(); it != cgiExtensions.end(); ++it) {
            if (path.length() >= it->length() && path.substr(path.length() - it->length()) == *it) {
                return true;
            }
        }
        return false;
    }

    Result<cgi::Request, error::AppError> CgiHandler::createCgiRequest(const RequestContext &ctx) const {
        const cgi::RequestFactory::Parameter param = {
            ctx.getRequest(),
            ctx.getConnection().get().getForeignAddress(),
            serverName_,
            "80", // TODO: 実際のサーバーポートを取得
            this->getScriptName(ctx),
            this->getPathInfo(ctx)
        };
        return cgi::RequestFactory::create(param);
    }

    std::string CgiHandler::getScriptName(const RequestContext &ctx) const {
        const std::string &path = ctx.getRequest().getRequestTarget();
        const std::vector<std::string> &cgiExtensions = docRootConfig_.getCgiExtensions();

        // CGI拡張子を持つファイルまでのパスを取得
        for (std::vector<std::string>::const_iterator it = cgiExtensions.begin(); it != cgiExtensions.end(); ++it) {
            size_t pos = path.find(*it);
            if (pos != std::string::npos) {
                return path.substr(0, pos + it->length());
            }
        }
        return path;
    }

    std::string CgiHandler::getPathInfo(const RequestContext &ctx) const {
        const std::string &path = ctx.getRequest().getRequestTarget();
        const std::string scriptName = this->getScriptName(ctx);

        // スクリプト名以降の部分をPATH_INFOとして返す
        if (path.length() > scriptName.length()) {
            return path.substr(scriptName.length());
        }
        return "";
    }
}
