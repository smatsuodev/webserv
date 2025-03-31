#include "error_page.hpp"
#include "http/response/response_builder.hpp"
#include "utils/string.hpp"
#include <fstream>

http::ErrorPage::ErrorPage(const config::ServerContext::ErrorPageMap &errorPage) : errorPage_(errorPage) {}

Either<IAction *, http::Response> http::ErrorPage::intercept(const Request &req, IHandler &next) {
    const Either<IAction *, Response> serveRes = next.serve(req);
    if (serveRes.isLeft()) {
        // Left の場合はまだレスポンスが決定できない
        return serveRes;
    }

    const Response res = serveRes.unwrapRight();
    const HttpStatusCode status = res.getStatusCode();
    if (status < 400) {
        // エラーでなければそのまま返す
        return Right(res);
    }

    std::string body;
    config::ServerContext::ErrorPageMap::iterator customErrorPage = errorPage_.find(status);

    if (customErrorPage != errorPage_.end()) {
        std::ifstream file(customErrorPage->second);
        std::stringstream buf;
        buf << file.rdbuf();
        body = buf.str();
    } else {
        // NOTE: handler のエラーレスポンスは上書きされる
        const std::string statusStr = utils::format("%d %s", status, getHttpStatusText(status).c_str());
        body = utils::format(
            "<!DOCTYPE html>\n"
            "<html>\n"
            "<head><title>%s</title></head>\n"
            "<body>\n"
            "<center><h1>%s</h1></center>\n"
            "<hr>\n"
            "<center>webserv/0.1.0</center>\n"
            "</body>\n"
            "</html>\n",
            statusStr.c_str(),
            statusStr.c_str()
        );
    }

    return Right(ResponseBuilder().html(body, status).build());
}
