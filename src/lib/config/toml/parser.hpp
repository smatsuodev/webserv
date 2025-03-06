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

        typedef Result<Table, error::AppError> ParseResult;
        ParseResult parse();

    private:
        const Tokenizer::Tokens &tokens_;
        std::vector<Token>::size_type pos_;
        std::vector<Token>::size_type nextPos_;
        Token token_;

        Token nextToken();
        bool consume(int expected);
        Result<Token, error::AppError> expect(int expected);
        void consumeNewlines();
        Result<types::Unit, error::AppError> expectNewlines();
        bool peek(int expected) const;

        Result<Table, error::AppError> parseTableHeader(Table table);
        Result<Table, error::AppError> parseKeyVal(Table table);
        Result<std::vector<std::string>, error::AppError> parseKey();
        Result<Value, error::AppError> parseVal();
    };
}

#endif
