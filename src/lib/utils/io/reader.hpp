#ifndef SRC_LIB_UTILS_IO_READER_HPP
#define SRC_LIB_UTILS_IO_READER_HPP

#include "../types/result.hpp"
#include "utils/auto_fd.hpp"

namespace io {
    class IReader {
    public:
        virtual ~IReader();

        typedef Result<std::size_t, std::string> ReadResult;
        virtual ReadResult read(char *buf, std::size_t nbyte) = 0;
        virtual bool eof() = 0;
    };

    class FdReader : public IReader {
    public:
        // 所有権を奪わないことを明示するため、AutoFd の参照を受け取る
        explicit FdReader(AutoFd &fd);
        virtual ~FdReader();

        virtual ReadResult read(char *buf, std::size_t nbyte);
        virtual bool eof();

    private:
        AutoFd &fd_;
        bool eof_;
    };
}

namespace bufio {
    class Reader : public io::IReader {
    public:
        explicit Reader(io::IReader &reader);
        virtual ~Reader();

        virtual ReadResult read(char *buf, std::size_t nbyte);
        virtual bool eof();

        typedef Result<std::string, std::string> ReadUntilResult;
        ReadUntilResult readUntil(const std::string &delimiter);

        typedef Result<std::string, std::string> ReadAllResult;
        ReadAllResult readAll();

    private:
        static const std::size_t kDefaultBufSize = 4096;
        io::IReader &reader_;
        // バッファの容量
        std::size_t bufCapacity_;
        char *buf_;
        // バッファに残っているデータのサイズ
        std::size_t bufSize_;
        // 未読み取りのバッファの先頭位置
        std::size_t bufPos_;

        // 読み取っていないバッファの量
        std::size_t unreadBufSize() const;
        Result<std::size_t, std::string> fillBuf();
    };
}

#endif
