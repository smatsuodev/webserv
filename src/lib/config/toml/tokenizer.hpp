#ifndef SRC_LIB_CONFIG_TOML_TOKENIZER_HPP
#define SRC_LIB_CONFIG_TOML_TOKENIZER_HPP

#include "utils/types/option.hpp"
#include "utils/types/result.hpp"
#include <string>
#include <vector>

namespace toml {
    enum TokenType {
        kUnknown = 0,
        kEof = 1 << 0,
        kNewLine = 1 << 1,
        kNum = 1 << 2,
        kKey = 1 << 3,
        kString = 1 << 4,
        kAssignment = 1 << 5,
        kDot = 1 << 6,
        kComma = 1 << 7,
        kLBracket = 1 << 8,
        kRBracket = 1 << 9,
        kLBrace = 1 << 10,
        kRBrace = 1 << 11,
        kTrue = 1 << 12,
        kFalse = 1 << 13,
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
        bool isSymbolicChar() const;
        bool isQuote() const;
        static bool isNumber(const std::string &literal);
        static bool isSymbol(const std::string &literal);
        bool isWhitespace() const;
        void skipWhitespaces();
        std::string readNewlines();
        std::string readSymbol();
        Result<std::string, std::string> readQuotedSymbol();
    };
}

#endif
