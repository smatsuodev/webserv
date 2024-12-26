#ifndef SRC_LIB_UTILS_IO_READER_HPP
#define SRC_LIB_UTILS_IO_READER_HPP

#include "../types/result.hpp"
#include "utils/auto_fd.hpp"
#include "utils/types/error.hpp"

namespace io {
    class IReader {
    public:
        virtual ~IReader();

        typedef Result<std::size_t, error::AppError> ReadResult;
        virtual ReadResult read(char *buf, std::size_t nbyte) = 0;
        virtual bool eof() = 0;
    };

    class FdReader : public IReader, NonCopyable {
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
    class Reader : public io::IReader, NonCopyable {
    public:
        explicit Reader(io::IReader &reader, std::size_t bufCapacity = kDefaultBufSize);
        virtual ~Reader();

        /**
         * 途中で EAGAIN が起きたら、読み込めた分の結果を返す
         * 最初に EAGAIN が起きたら kWouldBlock エラーを返す
         */
        virtual ReadResult read(char *buf, std::size_t nbyte);
        virtual bool eof();

        // 途中で EAGAIN が起きたら、エラーを返す
        typedef Result<std::string, error::AppError> ReadUntilResult;
        ReadUntilResult readUntil(const std::string &delimiter);

        // 途中で EAGAIN が起きたら、エラーを返す
        typedef Result<std::string, error::AppError> ReadAllResult;
        ReadAllResult readAll();

        // バッファを消費せず、指定したバイト数を読む
        // NOTE: nbyte より短い文字列が返され得る実装になっている
        typedef Result<std::string, error::AppError> PeekResult;
        PeekResult peek(std::size_t nbyte);

        /**
         * 指定したバイト数を読み捨てる
         * 実際に読み捨てたバイト数を返す
         */
        // NOTE: eof に達しない限り、必ず nbyte 読み捨てるように実装できそう (現状そうはなっていない)
        typedef Result<std::size_t, error::AppError> DiscardResult;
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
        Result<std::size_t, error::AppError> fillBuf();
        /**
         * 未読み取りのバッファを nbyte 以上に "できる" ことを保証する
         * バッファを埋めるのは caller の責任
         * バッファの拡張や、未読み取りデータの移動を行う
         */
        Result<void, error::AppError> ensureBufSize(std::size_t nbyte);
        /**
         * 最大 nbyte 分、バッファから移す。バッファは消費される。
         * 戻り値は実際に移されたバイト数
         */
        std::size_t consumeAndCopyBuf(char *buf, std::size_t nbyte);
        // バッファの先頭にデータを追加する
        Result<void, error::AppError> prependBuf(const char *buf, std::size_t nbyte);
    };
}

#endif
