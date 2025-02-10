#include "tokenizer.hpp"

#include "value.hpp"
#include "utils/fd.hpp"
#include "utils/string.hpp"
#include "utils/types/option.hpp"
#include "utils/types/try.hpp"

#include <sstream>

namespace toml {
    Token::Token(const TokenType type, const std::string &value) : type_(type), value_(value) {}

    bool Token::operator==(const Token &other) const {
        return type_ == other.type_ && value_ == other.value_;
    }

    bool Token::operator!=(const Token &other) const {
        return !(*this == other);
    }

    TokenType Token::getType() const {
        return type_;
    }

    const std::string &Token::getValue() const {
        return value_;
    }

    Tokenizer::Tokenizer(const std::string &input) : input_(input), pos_(0), nextPos_(0), ch_('\0') {
        nextChar();
    }

    void Tokenizer::nextChar() {
        if (nextPos_ >= input_.size()) {
            ch_ = '\0';
        } else {
            ch_ = input_[nextPos_];
            pos_ = nextPos_;
            nextPos_++;
        }
    }

    bool Tokenizer::isSymbolicChar() const {
        return std::isalnum(ch_) || ch_ == '_';
    }

    bool Tokenizer::isQuote() const {
        return ch_ == '"' || ch_ == '\'';
    }

    bool Tokenizer::isNumber(const std::string &literal) {
        for (std::string::size_type i = 0; i < literal.size(); i++) {
            if (!std::isdigit(literal[i])) {
                return false;
            }
        }
        return true;
    }

    // in C++98
    Tokenizer::TokenizeResult Tokenizer::tokenize() {
        Tokens tokens;

        while (ch_ != '\0') {
            skipWhitespaces();

            if (ch_ == '#') {
                while (ch_ != '\0' && ch_ != '\n') {
                    nextChar();
                }
                continue;
            }

            if (isQuote()) {
                std::string literal = readQuotedSymbol();
                tokens.push_back(Token(kString, literal));
                continue;
            }

            if (isSymbolicChar()) {
                std::string literal = readSymbol();
                if (isNumber(literal)) {
                    tokens.push_back(Token(kNum, literal));
                    continue;
                }
                tokens.push_back(Token(kKey, literal));
                continue;
            }

            TokenType type;
            switch (ch_) {
                case '=':
                    type = kAssignment;
                    break;
                default:
                    return Err(utils::format("unknown token: %c", ch_));
            }

            tokens.push_back(Token(type, utils::toString(ch_)));
            nextChar();
        }

        tokens.push_back(Token(kEof, ""));
        return Ok(tokens);
    }

    void Tokenizer::skipWhitespaces() {
        while (ch_ == ' ' || ch_ == '\t') {
            nextChar();
        }
    }

    std::string Tokenizer::readSymbol() {
        std::string literal;
        while (isSymbolicChar()) {
            literal += ch_;
            nextChar();
        }
        return literal;
    }

    std::string Tokenizer::readQuotedSymbol() {
        const char quote = ch_;
        nextChar();

        std::string literal;
        while (isSymbolicChar() || ch_ != quote) {
            literal += ch_;
            nextChar();
        }

        if (ch_ == quote) nextChar();
        return literal;
    }
}
