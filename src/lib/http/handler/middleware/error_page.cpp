#include "error_page.hpp"
#include "http/response/response_builder.hpp"
#include "utils/string.hpp"

http::Response http::ErrorPage::intercept(const Request &req, IHandler &next) {
    const Response res = next.serve(req);
    const HttpStatusCode status = res.getStatusCode();
    if (status < 400) {
        // エラーでなければそのまま返す
        return res;
    }

    // NOTE: handler のエラーレスポンスは上書きされる
    const std::string statusStr = utils::format("%d %s", status, getHttpStatusText(status).c_str());
    const std::string body = utils::format(
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
    return ResponseBuilder().html(body, status).build();
}
