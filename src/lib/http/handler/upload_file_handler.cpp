#include "upload_file_handler.hpp"
#include <sstream>
#include <vector>
#include <fstream>

http::UploadFileHandler::UploadFileHandler(const config::LocationContext::DocumentRootConfig &config)
    : docConfig_(config) {}

http::Response http::UploadFileHandler::serve(const Request &req) {
    if (req.getMethod() != kMethodPost) {
        return Response(kStatusMethodNotAllowed, Headers(), Some(std::string("Only POST method is allowed")));
    }

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
    boundaryPos += 9;
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

    const std::string &body = req.getBody();
    std::string delimiter = "--" + boundary;
    std::vector<std::string> parts;

    std::string::size_type pos = body.find(delimiter);
    if (pos == std::string::npos) {
        return Response(
            kStatusBadRequest, Headers(), Some(std::string("Invalid multipart format: boundary not found"))
        );
    }

    pos += delimiter.length();
    if (pos + 1 < body.length() && body[pos] == '\r' && body[pos + 1] == '\n') {
        pos += 2;
    } else {
        return Response(
            kStatusBadRequest, Headers(), Some(std::string("Invalid multipart format: expected CRLF after boundary"))
        );
    }

    while (true) {
        std::string::size_type nextPos = body.find(delimiter, pos);
        if (nextPos == std::string::npos) {
            return Response(
                kStatusBadRequest, Headers(), Some(std::string("Invalid multipart format: closing boundary not found"))
            );
        }

        std::string part = body.substr(pos, nextPos - pos);

        std::string::size_type headerEnd = part.find("\r\n\r\n");
        if (headerEnd == std::string::npos) {
            return Response(
                kStatusBadRequest, Headers(), Some(std::string("Invalid part format: no header/body separator"))
            );
        }

        std::string headers = part.substr(0, headerEnd);
        std::string content = part.substr(headerEnd + 4);
        std::string name;
        std::string filename;

        std::istringstream headerStream(headers);
        std::string headerLine;
        while (std::getline(headerStream, headerLine)) {
            if (headerLine.find("Content-Disposition:") == 0) {
                std::string::size_type namePos = headerLine.find("name=");
                if (namePos != std::string::npos) {
                    namePos += 5;
                    if (namePos < headerLine.length() && headerLine[namePos] == '"') {
                        std::string::size_type endQuote = headerLine.find('"', namePos + 1);
                        if (endQuote != std::string::npos) {
                            name = headerLine.substr(namePos + 1, endQuote - namePos - 1);
                        }
                    }
                }

                std::string::size_type filenamePos = headerLine.find("filename=");
                if (filenamePos != std::string::npos) {
                    filenamePos += 9;
                    if (filenamePos < headerLine.length() && headerLine[filenamePos] == '"') {
                        std::string::size_type endQuote = headerLine.find('"', filenamePos + 1);
                        if (endQuote != std::string::npos) {
                            filename = headerLine.substr(filenamePos + 1, endQuote - filenamePos - 1);
                        }
                    }
                }
            }
        }

        if (filename.empty() && !name.empty()) {
            filename = name;
        }

        if (!filename.empty()) {
            std::string filePath = docConfig_.getRoot() + "/" + filename;

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

        pos = nextPos + delimiter.length();

        if (pos + 1 < body.length() && body[pos] == '-' && body[pos + 1] == '-') {
            break;
        }
        if (pos + 1 < body.length() && body[pos] == '\r' && body[pos + 1] == '\n') {
            pos += 2;
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
