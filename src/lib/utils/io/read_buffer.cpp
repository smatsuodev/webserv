#include "read_buffer.hpp"
#include "utils/string.hpp"
#include "utils/types/try.hpp"

ReadBuffer::ReadBuffer(io::IReader &reader) : reader_(reader) {}

std::string ReadBuffer::consume(const std::size_t nbyte) {
    const std::size_t bytesToConsume = std::min(nbyte, buf_.size());
    const std::string consumed(buf_.data(), bytesToConsume);
    buf_.erase(buf_.begin(), buf_.begin() + static_cast<long>(bytesToConsume));
    return consumed;
}

// ReSharper disable once CppPassValueParameterByConstReference
Option<std::string> wrapString(const std::string s) { // NOLINT(*-unnecessary-value-param)
    return Some(s);
}

Option<std::string> ReadBuffer::consumeUntil(const std::string &delimiter) {
    const char *found = TRY(utils::strnstr(buf_.data(), delimiter.c_str(), buf_.size()));
    const char *delimEnd = found + delimiter.size();
    const std::size_t bytesToConsume = delimEnd - buf_.data();
    return Some(this->consume(bytesToConsume));
}

ReadBuffer::LoadResult ReadBuffer::load() {
    if (reader_.eof()) {
        return Ok(0ul);
    }

    char tmp[ReadBuffer::kLoadSize];
    const std::size_t bytesRead = TRY(reader_.read(tmp, ReadBuffer::kLoadSize));
    buf_.insert(buf_.begin(), tmp, tmp + bytesRead);
    return Ok(bytesRead);
}

std::size_t ReadBuffer::size() const {
    return buf_.size();
}
