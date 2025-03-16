#include "upload_file_handler.hpp"
#include <sstream>
#include <vector>
#include <fstream>

http::UploadFileHandler::UploadFileHandler(const config::LocationContext::DocumentRootConfig &config)
    : docConfig_(config) {}

http::Response http::UploadFileHandler::serve(const Request &req) {
    // リクエストメソッドの確認
    if (req.getMethod() != kMethodPost) {
        return Response(kStatusMethodNotAllowed, Headers(), Some(std::string("Only POST method is allowed")));
    }

    // リクエストヘッダーの検証
    Option<std::string> contentTypeOpt = req.getHeader("Content-Type");
    if (contentTypeOpt.isNone() || contentTypeOpt.unwrap().find("multipart/form-data") != 0) {
        return Response(kStatusBadRequest, Headers(), Some(std::string("Content-Type must be multipart/form-data")));
    }

    std::string contentType = contentTypeOpt.unwrap();
    std::string::size_type boundaryPos = contentType.find("boundary=");
    if (boundaryPos == std::string::npos) {
        return Response(kStatusBadRequest, Headers(), Some(std::string("No boundary parameter in Content-Type")));
    }

    std::string boundary;
    boundaryPos += 9; // "boundary="の長さ

    if (boundaryPos < contentType.length()) {
        std::string paramValue = contentType.substr(boundaryPos);

        if (!paramValue.empty()) {
            if (paramValue[0] == '"') {
                std::string::size_type endQuote = paramValue.find('"', 1);
                if (endQuote != std::string::npos) {
                    boundary = paramValue.substr(1, endQuote - 1);
                }
            } else {
                std::string::size_type endParam = paramValue.find_first_of(";, \t");
                if (endParam != std::string::npos) {
                    boundary = paramValue.substr(0, endParam);
                } else {
                    boundary = paramValue;
                }
            }
        }
    }

    if (boundary.empty()) {
        return Response(kStatusBadRequest, Headers(), Some(std::string("Invalid or empty boundary parameter")));
    }

    // マルチパートデータのパース
    const std::string &body = req.getBody();
    std::string delimiter = "--" + boundary;
    std::vector<std::string> parts;

    // 先頭の区切り文字を探す
    std::string::size_type pos = body.find(delimiter);
    if (pos == std::string::npos) {
        return Response(
            kStatusBadRequest, Headers(), Some(std::string("Invalid multipart format: boundary not found"))
        );
    }

    // 先頭の区切り文字をスキップ
    pos += delimiter.length();
    if (pos + 1 < body.length() && body[pos] == '\r' && body[pos + 1] == '\n') {
        pos += 2; // CRLF をスキップ
    } else {
        return Response(
            kStatusBadRequest, Headers(), Some(std::string("Invalid multipart format: expected CRLF after boundary"))
        );
    }

    while (true) {
        // 次の区切り文字を探す
        std::string::size_type nextPos = body.find(delimiter, pos);
        if (nextPos == std::string::npos) {
            return Response(
                kStatusBadRequest, Headers(), Some(std::string("Invalid multipart format: closing boundary not found"))
            );
        }

        // パート本体を取得
        std::string part = body.substr(pos, nextPos - pos);

        // ヘッダーとボディを分離
        std::string::size_type headerEnd = part.find("\r\n\r\n");
        if (headerEnd == std::string::npos) {
            return Response(
                kStatusBadRequest, Headers(), Some(std::string("Invalid part format: no header/body separator"))
            );
        }

        std::string headers = part.substr(0, headerEnd);
        std::string content = part.substr(headerEnd + 4); // "\r\n\r\n"の長さが4

        // Content-Dispositionヘッダーを解析
        std::string name;
        std::string filename;

        // Content-Dispositionヘッダーを解析する部分を修正
        std::istringstream headerStream(headers);
        std::string headerLine;
        while (std::getline(headerStream, headerLine)) {
            if (headerLine.find("Content-Disposition:") == 0) {
                std::string::size_type namePos = headerLine.find("name=");
                if (namePos != std::string::npos) {
                    namePos += 5; // "name="の長さ
                    if (namePos < headerLine.length() && headerLine[namePos] == '"') {
                        std::string::size_type endQuote = headerLine.find('"', namePos + 1);
                        if (endQuote != std::string::npos) {
                            name = headerLine.substr(namePos + 1, endQuote - namePos - 1);
                        }
                    }
                }

                std::string::size_type filenamePos = headerLine.find("filename=");
                if (filenamePos != std::string::npos) {
                    filenamePos += 9; // "filename="の長さ
                    if (filenamePos < headerLine.length() && headerLine[filenamePos] == '"') {
                        std::string::size_type endQuote = headerLine.find('"', filenamePos + 1);
                        if (endQuote != std::string::npos) {
                            filename = headerLine.substr(filenamePos + 1, endQuote - filenamePos - 1);
                        }
                    }
                }
            }
        }

        // filenameが空の場合は、nameをfilenameとして使用する
        if (filename.empty() && !name.empty()) {
            filename = name;
        }

        // ファイルアップロード処理
        if (!filename.empty()) {
            // セキュリティ対策不要なので、ファイル名をそのまま使用
            std::string filePath = docConfig_.getRoot() + "/" + filename;

            // CRLFの処理（末尾がCRLFで終わっている場合は削除）
            if (content.length() >= 2 && content[content.length() - 2] == '\r' &&
                content[content.length() - 1] == '\n') {
                content = content.substr(0, content.length() - 2);
            }

            std::ofstream outFile(filePath, std::ios::binary);
            if (!outFile) {
                return Response(
                    kStatusConflict, Headers(), Some(std::string("Failed to open file for writing: " + filename))
                );
            }
            outFile.write(content.c_str(), content.size());
            outFile.close();

            if (!outFile) {
                return Response(
                    kStatusInternalServerError, Headers(), Some(std::string("Failed to write file: " + filename))
                );
            }
        }

        // 次のパートに移動
        pos = nextPos + delimiter.length();

        // 終了判定
        if (pos + 1 < body.length() && body[pos] == '-' && body[pos + 1] == '-') {
            break; // 終了区切り文字 (--boundary--) を検出
        }
        if (pos + 1 < body.length() && body[pos] == '\r' && body[pos + 1] == '\n') {
            pos += 2; // 次のパートへ
        } else {
            return Response(
                kStatusBadRequest,
                Headers(),
                Some(std::string("Invalid multipart format: expected -- or CRLF after boundary"))
            );
        }
    }

    return Response(kStatusCreated, Headers(), Some(std::string("File uploaded successfully")));
}
