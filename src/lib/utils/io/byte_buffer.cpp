#include "byte_buffer.hpp"
#include "utils/string.hpp"
#include "utils/types/try.hpp"

ByteBuffer::ByteBuffer(io::IReader &reader) : reader_(reader) {}

ByteBuffer::ConsumeResult ByteBuffer::consume(const std::size_t nbyte) {
    const std::size_t bytesToConsume = std::min(nbyte, buf_.size());
    const std::string consumed(buf_.data(), bytesToConsume);
    buf_.erase(buf_.begin(), buf_.begin() + static_cast<long>(bytesToConsume));
    return Ok(consumed);
}

// ReSharper disable once CppPassValueParameterByConstReference
Option<std::string> wrapString(const std::string s) { // NOLINT(*-unnecessary-value-param)
    return Some(s);
}

ByteBuffer::ConsumeUntilResult ByteBuffer::consumeUntil(const std::string &delimiter) {
    const Option<char *> result = utils::strnstr(buf_.data(), delimiter.c_str(), buf_.size());
    if (result.isNone()) {
        return Ok<Option<std::string> >(None);
    }
    const char *delimEnd = result.unwrap() + delimiter.size();
    const std::size_t bytesToConsume = delimEnd - buf_.data();
    return this->consume(bytesToConsume).map(Some<std::string>);
}

ByteBuffer::LoadResult ByteBuffer::load() {
    if (reader_.eof()) {
        return Ok(0ul);
    }

    char tmp[ByteBuffer::kLoadSize];
    const std::size_t bytesRead = TRY(reader_.read(tmp, ByteBuffer::kLoadSize));
    buf_.insert(buf_.begin(), tmp, tmp + bytesRead);
    return Ok(bytesRead);
}

std::size_t ByteBuffer::size() const {
    return buf_.size();
}
