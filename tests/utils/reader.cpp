#include "reader.hpp"
#include <cstring>

StringReader::StringReader(const std::string &data) : data_(data), pos_(0) {}

StringReader::ReadResult StringReader::read(char *buf, const std::size_t nbyte) {
    const std::size_t bytesToRead = std::min(nbyte, data_.size() - pos_);
    std::memcpy(buf, data_.c_str() + pos_, bytesToRead);
    pos_ += bytesToRead;
    return Ok(bytesToRead);
}

bool StringReader::eof() {
    return pos_ == data_.size();
}

BrokenReader::ReadResult BrokenReader::read(char *, std::size_t) {
    return Err(error::kUnknown);
}

bool BrokenReader::eof() {
    return false;
}

WouldBlockReader::WouldBlockReader(io::IReader &reader, const int n) : reader_(reader), n_(n), counter_(0) {}

WouldBlockReader::ReadResult WouldBlockReader::read(char *buf, const std::size_t nbyte) {
    if (++counter_ == n_) {
        errno = EAGAIN;
        return Err(error::kIOWouldBlock);
    }
    return reader_.read(buf, nbyte);
}

bool WouldBlockReader::eof() {
    return reader_.eof();
}
