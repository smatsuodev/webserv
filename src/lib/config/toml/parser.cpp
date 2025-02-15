#include "parser.hpp"
#include "utils/types/try.hpp"

namespace toml {
    TomlParser::TomlParser(const Tokenizer::Tokens &tokens)
        : tokens_(tokens), pos_(0), nextPos_(0), token_(kUnknown, "") {
        nextToken();
    }

    TomlParser::~TomlParser() {}

    Token TomlParser::nextToken() {
        if (nextPos_ < tokens_.size()) {
            token_ = tokens_[nextPos_];
            pos_ = nextPos_;
            nextPos_++;
        }
        return token_;
    }

    bool TomlParser::consume(const int expected) {
        if (peek(expected)) {
            nextToken();
            return true;
        }

        return false;
    }

    Result<Token, error::AppError> TomlParser::expect(const int expected) {
        if (peek(expected)) {
            const Token t = token_;
            nextToken();
            return Ok(t);
        }

        return Err(error::AppError::kParseUnknown);
    }

    void TomlParser::consumeNewlines() {
        while (consume(kNewLine))
            ;
    }

    Result<types::Unit, error::AppError> TomlParser::expectNewlines() {
        TRY(expect(kNewLine));
        consumeNewlines();
        return Ok(types::Unit());
    }

    bool TomlParser::peek(const int expected) const {
        return token_.getType() & expected;
    }

    Result<Table, error::AppError> TomlParser::parse() {
        consumeNewlines();
        Table table = TRY(parseKeyVal(Table()));

        while (token_.getType() != kEof) {
            TRY(expectNewlines());
            if (token_.getType() == kEof) {
                break;
            }
            table = TRY(parseKeyVal(table));
        }

        return Ok(table);
    }

    Result<Table, error::AppError> TomlParser::parseKeyVal(Table table) {
        const std::string key = TRY(expect(kKey | kString)).getValue();

        TRY(expect(kAssignment));

        table.setValue(key, TRY(parseVal()));
        return Ok(table);
    }

    Result<Value, error::AppError> TomlParser::parseVal() {
        Value value;
        if (peek(kTrue)) {
            value = Value(true);
            nextToken();
        } else if (peek(kFalse)) {
            value = Value(false);
            nextToken();
        } else if (peek(kString)) {
            value = Value(token_.getValue());
            nextToken();
        } else if (peek(kNum)) {
            value = Value(std::stol(token_.getValue()));
            nextToken();
        } else if (peek(kLBracket)) {
            nextToken();

            Array array;
            if (consume(kRBracket)) {
                value = Value(array);
                return Ok(value);
            }

            array.addElement(TRY(parseVal()));

            while (!peek(kRBracket | kEof)) {
                TRY(expect(kComma));
                array.addElement(TRY(parseVal()));
            }

            TRY(expect(kRBracket));
            value = Value(array);
        } else if (peek(kLBrace)) {
            nextToken();

            Table table;
            if (consume(kRBrace)) {
                value = Value(table);
                return Ok(value);
            }

            table = TRY(parseKeyVal(table));

            while (!peek(kRBrace | kEof)) {
                TRY(expect(kComma));
                table = TRY(parseKeyVal(table));
            }

            TRY(expect(kRBrace));
            value = Value(table);
        } else {
            return Err(error::AppError::kParseUnknown);
        }

        return Ok(value);
    }
}
