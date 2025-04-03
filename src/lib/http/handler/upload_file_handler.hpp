#ifndef SRC_LIB_HTTP_UPLOAD_FILE_HANDLER_HPP
#define SRC_LIB_HTTP_UPLOAD_FILE_HANDLER_HPP
#include "handler.hpp"
#include "config/config.hpp"

namespace http {
    class UploadFileHandler : public IHandler {
    public:
        explicit UploadFileHandler(const config::LocationContext::DocumentRootConfig &config);
        Response serve(const Request &req);

    private:
        static const std::string kContentTypeMultipartFormData;
        static const std::string kBoundary;
        config::LocationContext::DocumentRootConfig docConfig_;
        std::string boundary_;

        // マルチパートボディを解析する
        Response parseMultipartBody(const std::string &body, const std::string &boundary) const;

        // マルチパートの各パートを処理する
        static Result<bool, Response>
        processMultipartPart(const std::string &part, std::string &name, std::string &filename);

        // ヘッダーをCRLFで分割するヘルパー関数
        static std::vector<std::string> splitHeadersByNewline(const std::string &headers);

        // Content-Dispositionヘッダーを処理するヘルパー関数
        static void
        processContentDispositionHeader(const std::string &headerValue, std::string &name, std::string &filename);

        // ファイルを保存する
        Response saveUploadedFile(const std::string &filename, const std::string &content) const;

        // 最初のバウンダリを検出して検証する
        static Result<std::string::size_type, Response>
        findInitialBoundary(const std::string &body, const std::string &delimiter);

        // パートを抽出して処理する
        Result<std::string::size_type, Response>
        extractAndProcessPart(const std::string &body, std::string::size_type pos, const std::string &delimiter) const;

        // バウンダリの終端をチェックする
        static Result<bool, Response> checkBoundaryEnd(const std::string &body, std::string::size_type pos);
    };
}

#endif
