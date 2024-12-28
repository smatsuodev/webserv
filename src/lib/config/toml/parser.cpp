#include "parser.hpp"
#include "utils/types/try.hpp"

namespace toml {
    TomlParser::TomlParser(const Tokenizer::Tokens &tokens) : tokens_(tokens), pos_(0), nextPos_(0), token_(None) {
        nextToken();
    }

    TomlParser::~TomlParser() {}

    Option<Token> TomlParser::nextToken() {
        if (nextPos_ >= tokens_.size()) {
            token_ = None;
        } else {
            token_ = Some(tokens_[nextPos_]);
            pos_ = nextPos_;
            nextPos_++;
        }
        return token_;
    }

    bool TomlParser::consume(const TokenType expected) {
        if (token_.isSome() && token_.unwrap().getType() == expected) {
            nextToken();
            return true;
        }

        return false;
    }

    // toml = expression *( newline expression )
    Result<Table, error::AppError> TomlParser::parse() {
        Table table = TRY(parseExpression(Table()));

        while (token_ != None) {
            consume(kNewLine);
            table = TRY(parseExpression(table));
        }

        return Ok(table);
    }

    // expression =  ws keyval ws [ comment ]
    // expression =/ ws table ws [ comment ]
    Result<Table, error::AppError> TomlParser::parseExpression(Table table) {
        Result<Table, error::AppError> parseKeyValResult = parseKeyVal(table);

        if (parseKeyValResult.isOk()) {
            return parseKeyValResult;
        }

        return parseTable(table);
    }
}
