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
        // TODO
        return true;
    }

    Result<cgi::Request, error::AppError> CgiHandler::createCgiRequest(const RequestContext &ctx) const {
        const cgi::RequestFactory::Parameter param = {
            ctx.getRequest(),
            ctx.getConnection().get().getForeignAddress(),
            serverName_,
            "", // TODO: server port
            this->getScriptName(ctx), // TODO: script name
            this->getPathInfo(ctx) // TODO: path info
        };
        return cgi::RequestFactory::create(param);
    }

    std::string CgiHandler::getScriptName(const RequestContext &ctx) const {
        // TODO
        return "";
    }

    std::string CgiHandler::getPathInfo(const RequestContext &ctx) const {
        // TODO
        return "";
    }
}
