#include "upload_file_handler.hpp"

#include "http/request/request_parser.hpp"
#include "http/response/response_builder.hpp"
#include "utils/string.hpp"

#include <vector>
#include <fstream>

const std::string http::UploadFileHandler::kContentTypeMultipartFormData = "multipart/form-data";
const std::string http::UploadFileHandler::kBoundary = "boundary";

http::UploadFileHandler::UploadFileHandler(const config::LocationContext::DocumentRootConfig &config)
    : docConfig_(config) {}

Either<IAction *, http::Response> http::UploadFileHandler::serve(const RequestContext &ctx) {
    const Request &req = ctx.getRequest();
    const Option<std::string> contentTypeOpt = req.getHeader("Content-Type");
    if (contentTypeOpt.isNone()) {
        return Right(
            ResponseBuilder()
                .status(kStatusUnsupportedMediaType)
                .text("Content-Type must be multipart/form-data")
                .build()
        );
    }

    // Content-Typeをセミコロン区切りでパース
    const std::string contentType = contentTypeOpt.unwrap();
    const std::vector<std::string> contentTypeParams = utils::split(contentType, ';');

    // 最初の部分がmultipart/form-dataであるか確認
    if (contentTypeParams.empty() || utils::trim(contentTypeParams[0]) != kContentTypeMultipartFormData) {
        return Right(
            ResponseBuilder()
                .status(kStatusUnsupportedMediaType)
                .text("Content-Type must be multipart/form-data")
                .build()
        );
    }

    // boundaryパラメータを探す
    std::string boundary;
    bool foundBoundary = false;

    // 2つ目以降のパラメータをチェック
    for (size_t i = 1; i < contentTypeParams.size(); ++i) {
        std::string param = utils::trim(contentTypeParams[i]);
        if (param.empty()) {
            continue;
        }

        // key=valueの形式を確認
        const std::string::size_type eqPos = param.find('=');
        if (eqPos == std::string::npos) {
            return Right(
                ResponseBuilder().status(kStatusBadRequest).text("Invalid parameter format: " + param).build()
            );
        }

        std::string key = utils::trim(param.substr(0, eqPos));
        const std::string value = utils::trim(param.substr(eqPos + 1));

        // boundaryパラメータの処理
        if (key != kBoundary) continue;

        if (value.empty()) {
            return Right(ResponseBuilder().status(kStatusBadRequest).text("Empty boundary parameter").build());
        }

        // 引用符付きの値の処理
        if (value[0] == '"') {
            if (value.length() < 2 || value[value.length() - 1] != '"') {
                return Right(
                    ResponseBuilder().status(kStatusBadRequest).text("Unclosed quoted boundary parameter").build()
                );
            }
            boundary = value.substr(1, value.length() - 2);
        } else {
            boundary = value;
        }

        // RFC 1341に基づく境界値の検証
        if (boundary.length() > 70) {
            return Right(
                ResponseBuilder()
                    .status(kStatusBadRequest)
                    .text("Boundary parameter too long (max 70 characters)")
                    .build()
            );
        }

        for (size_t j = 0; j < boundary.length(); j++) {
            const char c = boundary[j];
            if (c < 32 || c > 126) {
                return Right(
                    ResponseBuilder().status(kStatusBadRequest).text("Invalid character in boundary parameter").build()
                );
            }
        }

        foundBoundary = true;
        break;
    }

    if (!foundBoundary || boundary.empty()) {
        return Right(ResponseBuilder().status(kStatusBadRequest).text("No boundary parameter in Content-Type").build());
    }

    const std::string &body = req.getBody();
    return Right(parseMultipartBody(body, boundary));
}

http::Response http::UploadFileHandler::parseMultipartBody(const std::string &body, const std::string &boundary) const {
    const std::string delimiter = "--" + boundary;

    // 最初のバウンダリを検出
    const Result<std::string::size_type, Response> findResult = findInitialBoundary(body, delimiter);
    if (findResult.isErr()) {
        return findResult.unwrapErr();
    }

    std::string::size_type pos = findResult.unwrap();

    // 各パートを処理
    while (true) {
        // パートを抽出して処理
        Result<std::string::size_type, Response> processResult = extractAndProcessPart(body, pos, delimiter);
        if (processResult.isErr()) {
            return processResult.unwrapErr();
        }

        pos = processResult.unwrap();

        // バウンダリの終端をチェック
        Result<bool, Response> endResult = checkBoundaryEnd(body, pos);
        if (endResult.isErr()) {
            return endResult.unwrapErr();
        }

        if (endResult.unwrap()) {
            // 最後のバウンダリ（--で終わる）
            break;
        }

        // 次のパートへ
        pos += 2; // CRLFをスキップ
    }

    return ResponseBuilder().status(kStatusCreated).text("File uploaded successfully").build();
}

Result<std::string::size_type, http::Response>
http::UploadFileHandler::findInitialBoundary(const std::string &body, const std::string &delimiter) {
    std::string::size_type pos = body.find(delimiter);
    if (pos == std::string::npos) {
        return Err(
            ResponseBuilder().status(kStatusBadRequest).text("Invalid multipart format: boundary not found").build()
        );
    }

    pos += delimiter.length();
    if (pos + 1 < body.length() && body[pos] == '\r' && body[pos + 1] == '\n') {
        pos += 2;
    } else {
        return Err(
            ResponseBuilder()
                .status(kStatusBadRequest)
                .text("Invalid multipart format: expected CRLF after boundary")
                .build()
        );
    }

    return Ok(pos);
}

Result<std::string::size_type, http::Response> http::UploadFileHandler::extractAndProcessPart(
    const std::string &body, std::string::size_type pos, const std::string &delimiter
) const {
    std::string::size_type nextPos = body.find(delimiter, pos);
    if (nextPos == std::string::npos) {
        return Err(
            ResponseBuilder()
                .status(kStatusBadRequest)
                .text("Invalid multipart format: closing boundary not found")
                .build()
        );
    }

    std::string part = body.substr(pos, nextPos - pos);

    std::string::size_type headerEnd = part.find("\r\n\r\n");
    if (headerEnd == std::string::npos) {
        return Err(
            ResponseBuilder().status(kStatusBadRequest).text("Invalid part format: no header/body separator").build()
        );
    }

    std::string headers = part.substr(0, headerEnd);
    std::string content = part.substr(headerEnd + 4);
    std::string name;
    std::string filename;

    Result<bool, Response> processResult = processMultipartPart(headers, name, filename);
    if (processResult.isErr()) {
        return Err(processResult.unwrapErr());
    }

    if (!filename.empty()) {
        if (content.length() >= 2 && content[content.length() - 2] == '\r' && content[content.length() - 1] == '\n') {
            content = content.substr(0, content.length() - 2);
        }

        Response saveResult = saveUploadedFile(filename, content);
        if (saveResult.getStatusCode() != kStatusCreated) {
            return Err(saveResult);
        }
    }

    return Ok(nextPos + delimiter.length());
}

Result<bool, http::Response>
http::UploadFileHandler::checkBoundaryEnd(const std::string &body, std::string::size_type pos) {
    if (pos + 1 < body.length() && body[pos] == '-' && body[pos + 1] == '-') {
        return Ok(true); // 最終バウンダリ
    }

    if (pos + 1 < body.length() && body[pos] == '\r' && body[pos + 1] == '\n') {
        return Ok(false); // 通常のバウンダリ（続きがある）
    }

    return Err(
        ResponseBuilder()
            .status(kStatusBadRequest)
            .text("Invalid multipart format: expected -- or CRLF after boundary")
            .build()
    );
}

Result<bool, http::Response>
http::UploadFileHandler::processMultipartPart(const std::string &headers, std::string &name, std::string &filename) {
    // ヘッダーをCRLFで分割
    const std::vector<std::string> headerLines = splitHeadersByNewline(headers);

    for (const std::string &headerLine : headerLines) {
        // 空行や不正な形式の行はスキップ
        if (headerLine.empty()) {
            continue;
        }

        // RequestParserを使用してヘッダーを解析
        RequestParser::ParseHeaderFieldLineResult parseResult = RequestParser::parseHeaderFieldLine(headerLine);
        if (parseResult.isErr()) {
            return Err(ResponseBuilder().status(kStatusBadRequest).text("Invalid header format in multipart").build());
        }

        const RequestParser::HeaderField headerField = parseResult.unwrap();
        std::string headerName = headerField.first;
        std::string headerValue = headerField.second;

        // Content-Dispositionヘッダーを処理
        if (headerName == "Content-Disposition") {
            processContentDispositionHeader(headerValue, name, filename);
        }
    }

    // filenameが空でnameが設定されている場合、filenameにnameを設定
    if (filename.empty() && !name.empty()) {
        filename = name;
    }

    return Ok(true);
}

std::vector<std::string> http::UploadFileHandler::splitHeadersByNewline(const std::string &headers) {
    std::vector<std::string> result;

    size_t start = 0;
    size_t end = headers.find("\r\n");

    while (end != std::string::npos) {
        result.push_back(headers.substr(start, end - start));
        start = end + 2; // CRLFをスキップ
        end = headers.find("\r\n", start);
    }

    // 最後の部分が残っていればそれも追加
    if (start < headers.length()) {
        result.push_back(headers.substr(start));
    }

    return result;
}

void http::UploadFileHandler::processContentDispositionHeader(
    const std::string &headerValue, std::string &name, std::string &filename
) {
    // 各パラメータをセミコロンで分割
    const std::vector<std::string> params = utils::split(headerValue, ';');

    for (const std::string &param : params) {
        std::string trimmedParam = utils::trim(param);

        // name属性の処理
        if (trimmedParam.find("name=") == 0) {
            const size_t nameStart = trimmedParam.find('"');
            if (nameStart != std::string::npos) {
                const size_t nameEnd = trimmedParam.find('"', nameStart + 1);
                if (nameEnd != std::string::npos) {
                    name = trimmedParam.substr(nameStart + 1, nameEnd - nameStart - 1);
                }
            }
        }

        // filename属性の処理
        else if (trimmedParam.find("filename=") == 0) {
            const size_t filenameStart = trimmedParam.find('"');
            if (filenameStart != std::string::npos) {
                const size_t filenameEnd = trimmedParam.find('"', filenameStart + 1);
                if (filenameEnd != std::string::npos) {
                    filename = trimmedParam.substr(filenameStart + 1, filenameEnd - filenameStart - 1);
                }
            }
        }
    }
}

http::Response
http::UploadFileHandler::saveUploadedFile(const std::string &filename, const std::string &content) const {
    const std::string filePath = docConfig_.getRoot() + "/" + filename;

    std::ofstream outFile(filePath, std::ios::binary);
    if (!outFile) {
        return ResponseBuilder().status(kStatusConflict).text("Failed to open file for writing: " + filename).build();
    }

    outFile.write(content.c_str(), static_cast<std::streamsize>(content.size()));
    outFile.close();

    if (outFile.bad()) {
        return ResponseBuilder().status(kStatusInternalServerError).text("Failed to write file: " + filename).build();
    }

    return ResponseBuilder().status(kStatusCreated).text("File saved successfully").build();
}
