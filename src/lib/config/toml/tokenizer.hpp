#ifndef SRC_LIB_CONFIG_TOML_TOKENIZER_HPP
#define SRC_LIB_CONFIG_TOML_TOKENIZER_HPP

#include "utils/types/option.hpp"
#include "utils/types/result.hpp"
#include <string>
#include <vector>

namespace toml {
    enum TokenType {
        kKey,
        kDot,
        kComma,
        kAssignment,
        kValue,
        kTableHeaderOpen,
        kTableHeaderClose,
        kInlineTableOpen,
        kInlineTableClose,
        kArrayOpen,
        kArrayClose,
        kNewLine,
    };

    class Token {
    public:
        explicit Token(TokenType type, const std::string &value);

        bool operator==(const Token &other) const;
        bool operator!=(const Token &other) const;

        TokenType getType() const;
        const std::string &getValue() const;

    private:
        TokenType type_;
        std::string value_;
    };

    class Tokenizer {
    public:
        typedef std::vector<Token> Tokens;
        /**
         * 意味のある単位でまとめて返す
         * 改行トークンで区切っても良さそう
         */
        typedef Result<Tokens, std::string> TokenizeResult;
        static TokenizeResult tokenize(const std::string &input);

    private:
        static Result<Tokens, std::string> tokenizeArray(const std::string &rawArray);
        static Result<Tokens, std::string> tokenizeInlineTable(const std::string &rawInlineTable);
        static Result<Tokens, std::string> tokenizeTableHeader(const std::string &rawTableHeader);
        static Result<Tokens, std::string> tokenizeKeyValue(const std::string &rawKeyValue);
        static Result<Tokens, std::string> tokenizeKey(const std::string &rawKey);
        static Result<Tokens, std::string> tokenizeValue(const std::string &rawValue);

        static Option<std::size_t> findAssignment(const std::string &line);
        static Result<std::vector<std::string>, std::string> splitElements(const std::string &);
    };
}

#endif