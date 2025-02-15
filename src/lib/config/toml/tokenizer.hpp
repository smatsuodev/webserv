#ifndef SRC_LIB_CONFIG_TOML_TOKENIZER_HPP
#define SRC_LIB_CONFIG_TOML_TOKENIZER_HPP

#include "utils/types/option.hpp"
#include "utils/types/result.hpp"
#include <string>
#include <vector>

namespace toml {
    enum TokenType {
        kUnknown,
        kEof,
        kNewLine,
        kNum,
        kKey,
        kString,
        kAssignment,
        kDot,
        kComma,
        kLBracket,
        kRBracket,
        kLBrace,
        kRBrace,
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

        explicit Tokenizer(const std::string &input);
        TokenizeResult tokenize();

    private:
        const std::string &input_;
        std::string::size_type pos_;
        std::string::size_type nextPos_;
        char ch_;

        void nextChar();
        void skipWhitespaces();
        bool isSymbolicChar() const;
        bool isQuote() const;
        static bool isNumber(const std::string &literal);
        static bool isSymbol(const std::string &literal);
        bool isWhitespace() const;
        std::string readSymbol();
        Result<std::string, std::string> readQuotedSymbol();
    };
}

#endif
