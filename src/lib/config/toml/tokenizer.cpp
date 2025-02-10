#include "tokenizer.hpp"

#include "value.hpp"
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

    // in C++98
    Tokenizer::TokenizeResult Tokenizer::tokenize() {
        Tokens tokens;

        while (ch_ != '\0') {
            skipWhitespaces();

            if (std::isdigit(ch_)) {
                std::string literal = readNumber();
                tokens.push_back(Token(kNum, literal));
                continue;
            }

            switch (ch_) {
                case '#':
                    while (ch_ != '\0' && ch_ != '\n') {
                        nextChar();
                    }
                    break;
                default:;
            }
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

    std::string Tokenizer::readNumber() {
        std::string literal;
        while (std::isdigit(ch_)) {
            literal += ch_;
            nextChar();
        }
        return literal;
    }
}
