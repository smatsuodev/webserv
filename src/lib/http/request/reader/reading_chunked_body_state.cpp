#include "reading_chunked_body_state.hpp"
#include "utils/logger.hpp"
#include "utils/types/try.hpp"
#include <sstream>

namespace http {
    ReadingChunkedBodyState::ReadingChunkedBodyState(RequestReader &reader, const std::size_t clientMaxBodySize)
        : reader_(reader), clientMaxBodySize_(clientMaxBodySize), state_(kReadingChunkSize), chunkSize_(0) {}

    /*
        chunked-body = *chunk
                       last-chunk
                       trailer-section
                       CRLF

        chunk = chunk-size [chunk-ext] CRLF
                chunk-data CRLF

        chunk-size = 1*HEXDIG
        chunk-data = 1*OCTET
        last-chunk = 1*("0") [chunk-ext] CRLF

        trailer-section = *(field-line CRLF)
        chunk-ext = *(BWS ";" BWS chunk-ext-name [BWS "=" BWS chunk-ext-val])
    */
    // NOTE: trailer-section, chunk-ext は無視する
    Result<RequestReader::IState::HandleStatus, error::AppError> ReadingChunkedBodyState::handle(ReadBuffer &readBuf) {
        while (state_ != kDone) {
            const Option<std::string> maybeLine = TRY(getLine(readBuf));
            if (maybeLine.isNone()) {
                return Ok(HandleStatus::kSuspend);
            }
            const std::string &line = maybeLine.unwrap();

            switch (state_) {
                case kReadingChunkSize: {
                    chunkSize_ = TRY(ReadingChunkedBodyState::parseChunkSizeLine(line));
                    if (chunkSize_ != 0) {
                        state_ = kReadingChunkData;
                    } else {
                        if (clientMaxBodySize_ < body_.size() + chunkSize_) {
                            LOG_WARN("Payload too large");
                            return Err(error::kHttpPayloadTooLarge);
                        }
                        state_ = kReadingTrailer;
                    }
                    break;
                }

                case kReadingChunkData: {
                    body_ += line;
                    state_ = kReadingChunkSize;
                    break;
                }

                case kReadingTrailer: {
                    // trailer は無視する
                    if (line.empty()) {
                        // chunked-body の終わり
                        state_ = kDone;
                    }
                    break;
                }

                default: {
                    // ここに到達することはない
                    break;
                }
            }
        }

        // NOTE: proxy の場合、Transfer-Encoding の chunked を削除するが、この課題では対応しない
        reader_.setBody(body_);
        reader_.changeState(NULL);

        return Ok(HandleStatus::kDone);
    }

    Result<std::size_t, error::AppError> ReadingChunkedBodyState::parseChunkSizeLine(const std::string &line) {
        std::istringstream iss(line);
        std::string chunkSizeStr;
        // ";" 以降は chunk-ext なので、それ以前を取り出す
        if (!std::getline(iss, chunkSizeStr, ';')) {
            return Err(error::kParseUnknown);
        }

        // chunk-size = 1*HEXDIG
        // TODO: std::hex で十分か確認
        std::size_t chunkSize;
        if (!(std::istringstream(chunkSizeStr) >> std::hex >> chunkSize)) {
            return Err(error::kParseUnknown);
        }

        return Ok(chunkSize);
    }
}
