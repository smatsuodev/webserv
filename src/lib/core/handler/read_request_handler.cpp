#include "read_request_handler.hpp"
#include "write_response_handler.hpp"
#include "core/action.hpp"
#include "http/response/response.hpp"
#include "http/response/response_builder.hpp"
#include "utils/logger.hpp"
#include "utils/io/reader.hpp"
#include "utils/types/try.hpp"

#include <fcntl.h>
#include <sys/stat.h>

// TODO: config を元に適切な path に解決する
// 今はカレントディレクトリからの相対パスとして扱う
Result<std::string, error::AppError> getPath(const http::Request &req) {
    LOG_DEBUGF("request target: %s", req.getRequestTarget().c_str());

    const std::string path = req.getRequestTarget().substr(1);
    if (path.empty()) {
        LOG_DEBUGF("invalid request target: %s", req.getRequestTarget().c_str());
        return Err(error::kUnknown);
    }
    return Ok(path);
}

// andThen で使いたいので、ref ではなく値を受け取る
// ReSharper disable once CppPassValueParameterByConstReference
Result<int, error::AppError> openFile(const std::string path) { // NOLINT(*-unnecessary-value-param)
    const int fd = open(path.c_str(), O_RDONLY);
    if (fd == -1) {
        LOG_DEBUGF("failed to open file: %s", path.c_str());
        return Err(error::kUnknown);
    }
    return Ok(fd);
}

Result<std::string, error::AppError> readFile(const int rawFd) {
    AutoFd fd(rawFd);
    io::FdReader fdReader(fd);
    bufio::Reader bufReader(fdReader);
    return Ok(TRY(bufReader.readAll()));
}

// ReSharper disable once CppPassValueParameterByConstReference
http::Response toResponse(const std::string fileContent) { // NOLINT(*-unnecessary-value-param)
    return http::ResponseBuilder().text(fileContent).build();
}

ReadRequestHandler::ReadRequestHandler(bufio::Reader &reader) : reqReader_(reader) {}

IEventHandler::InvokeResult ReadRequestHandler::invoke(const Context &ctx) {
    LOG_DEBUG("start ReadRequestHandler");

    const http::Request req = TRY(reqReader_.readRequest());

    LOG_DEBUGF("HTTP request parsed");

    // TODO: すべてのエラーが 404 とは限らない. 適切なレスポンスを返すようにする
    // TODO: readFile が kWouldBlock を返すと、読み取った req が失われる
    const http::Response res = getPath(req)
                                   .andThen(openFile)
                                   .andThen(readFile)
                                   .map(toResponse)
                                   .unwrapOr(http::ResponseBuilder().text("not found", http::kStatusNotFound).build());

    std::vector<IAction *> actions;
    actions.push_back(new RegisterEventAction(Event(ctx.getEvent().getFd(), Event::kWrite)));
    actions.push_back(new RegisterEventHandlerAction(ctx.getConnection().unwrap(), new WriteResponseHandler(res)));

    return Ok(actions);
}
