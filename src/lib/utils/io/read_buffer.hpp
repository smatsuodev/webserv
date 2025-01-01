#ifndef SRC_LIB_UTILS_IO_READ_BUFFER_HPP
#define SRC_LIB_UTILS_IO_READ_BUFFER_HPP

#include "reader.hpp"
#include "../types/result.hpp"
#include <vector>

// utils/io にあるべきかは微妙
class ReadBuffer {
public:
    explicit ReadBuffer(io::IReader &reader);

    // バッファから nbyte を消費して返す
    typedef Result<std::string, error::AppError> ConsumeResult;
    ConsumeResult consume(std::size_t nbyte);
    /**
     * バッファから delimiter までを消費して返す
     * eof が近い場合を除き、delimiter が見つからない場合は Ok(None) を返す
     */
    typedef Result<Option<std::string>, error::AppError> ConsumeUntilResult;
    ConsumeUntilResult consumeUntil(const std::string &delimiter);
    /**
     * バッファにデータを追加する
     * read (2) したバイト数を返す
     */
    typedef Result<std::size_t, error::AppError> LoadResult;
    LoadResult load();
    // 未読み取りのバイト数を返す
    std::size_t size() const;

    private:
        static const std::size_t kLoadSize = 4096;
        io::IReader &reader_;
        std::vector<char> buf_;
};

#endif
