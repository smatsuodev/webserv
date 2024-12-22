#ifndef SRC_LIB_UTILS_IO_READER_HPP
#define SRC_LIB_UTILS_IO_READER_HPP

#include "../types/result.hpp"
#include "utils/auto_fd.hpp"
#include "utils/types/unit.hpp"

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
        explicit Reader(io::IReader &reader, std::size_t bufCapacity = kDefaultBufSize);
        virtual ~Reader();

        virtual ReadResult read(char *buf, std::size_t nbyte);
        virtual bool eof();

        typedef Result<std::string, std::string> ReadUntilResult;
        ReadUntilResult readUntil(const std::string &delimiter);

        typedef Result<std::string, std::string> ReadAllResult;
        ReadAllResult readAll();

        // バッファを消費せず、指定したバイト数を読む
        typedef Result<std::string, std::string> PeekResult;
        PeekResult peek(std::size_t nbyte);

        /**
         * 指定したバイト数を読み捨てる
         * 実際に読み捨てたバイト数を返す
         */
        typedef Result<std::size_t, std::string> DiscardResult;
        DiscardResult discard(std::size_t nbyte);

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
        /**
         * バッファを埋める
         * バッファに追加されたバイト数を返す
         */
        Result<std::size_t, std::string> fillBuf();
        /**
         * 未読み取りのバッファを nbyte 以上に "できる" ことを保証する
         * バッファを埋めるのは caller の責任
         * バッファの拡張や、未読み取りデータの移動を行う
         */
        Result<types::Unit, std::string> ensureBufSize(std::size_t nbyte);
    };
}

#endif
