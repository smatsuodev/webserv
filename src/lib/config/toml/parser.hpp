#ifndef SRC_LIB_CONFIG_TOML_PARSER_HPP
#define SRC_LIB_CONFIG_TOML_PARSER_HPP

#include "tokenizer.hpp"
#include "value.hpp"
#include "utils/types/result.hpp"
#include "utils/types/error.hpp"
#include "utils/types/unit.hpp"

namespace toml {
    class TomlParser {
    public:
        explicit TomlParser(const Tokenizer::Tokens &tokens);
        ~TomlParser();

        Result<Table, error::AppError> parse(const std::string &tomlStr);

    private:
        const Tokenizer::Tokens &tokens_;
        int pos_;
        int nextPos_;
        Option<Token> token_;

        /**
         * 次のトークンを読み込む
         * @return まだトークンが残っているなら先頭のトークンを返す。<br>
         * トークンが無いならNoneを返す。
         */
        Option<Token> nextToken();
        bool consume(TokenType expected);

        Result<Table, error::AppError> parseExpression(Table table);
        Result<Table, error::AppError> parseKeyVal(Table table);
        Result<Table, error::AppError> parseTable(Table table);
    };
}

#endif
