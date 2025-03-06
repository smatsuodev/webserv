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
        Table table;

        while (token_.getType() != kEof) {
            if (peek(kLBracket)) {
                table = TRY(parseTableHeader(table));
            } else {
                table = TRY(parseKeyVal(table));
                expectNewlines();
            }
        }

        return Ok(table);
    }

    Result<Table, error::AppError> TomlParser::parseTableHeader(Table table) {
        TRY(expect(kLBracket));
        std::vector<std::string> keys = TRY(parseKey());
        TRY(expect(kRBracket));

        if (keys.empty()) {
            return Err(error::AppError::kParseUnknown);
        }

        // ルートテーブルへのパスを作成
        Table *currentTable = &table;

        for (size_t i = 0; i < keys.size() - 1; ++i) {
            const std::string &key = keys[i];
            Option<Value> nextValue = currentTable->getValue(key);

            if (nextValue.isSome() && nextValue.unwrap().getType() == Value::kTable) {
                // 既存のテーブルを取得
                currentTable = &currentTable->getValueRef(key).getTableRef();
            } else {
                // 新しいテーブルを作成
                Table newTable;
                currentTable->setValue(key, Value(newTable));
                currentTable = &currentTable->getValueRef(key).getTableRef();
            }
        }

        // 最後のキーに空テーブルを設定
        const std::string &lastKey = keys.back();
        Table newTable;

        // 既に同じキーが存在する場合はエラー（二重定義）
        if (currentTable->getValue(lastKey).isSome()) {
            return Err(error::AppError::kParseUnknown);
        }

        currentTable->setValue(lastKey, Value(newTable));
        currentTable = &currentTable->getValueRef(lastKey).getTableRef();

        // テーブル内のキー値ペアを解析
        while (!peek(kLBracket) && !peek(kEof)) {
            if (peek(kNewLine)) {
                consumeNewlines();
                if (peek(kLBracket) || peek(kEof)) {
                    break;
                }
            }

            *currentTable = TRY(parseKeyVal(*currentTable));
            TRY(expectNewlines());
        }

        return Ok(table);
    }

    Result<Table, error::AppError> TomlParser::parseKeyVal(Table table) {
        std::vector<std::string> keys = TRY(parseKey());
        TRY(expect(kAssignment));
        Value val = TRY(parseVal());

        if (keys.empty()) {
            return Err(error::AppError::kParseUnknown);
        }

        if (keys.size() == 1) {
            TRY(table.setValue(keys[0], val).okOr(error::kParseUnknown));
            return Ok(table);
        }

        Table *currentTable = &table;

        for (size_t i = 0; i < keys.size() - 1; ++i) {
            const std::string &key = keys[i];
            Option<Value> nextValue = currentTable->getValue(key);

            if (nextValue.isSome() && nextValue.unwrap().getType() == Value::kTable) {
                Value tableValue = nextValue.unwrap();
                Table subTable = tableValue.getTable().unwrap();
                currentTable->setValue(key, Value(subTable));
                currentTable = &currentTable->getValueRef(key).getTableRef();
            } else {
                Table newTable;
                currentTable->setValue(key, Value(newTable));
                currentTable = &currentTable->getValueRef(key).getTableRef();
            }
        }

        TRY(currentTable->setValue(keys.back(), val).okOr(error::kParseUnknown));

        return Ok(table);
    }

    Result<std::vector<std::string>, error::AppError> TomlParser::parseKey() {
        std::vector<std::string> keys;

        if (peek(kString)) {
            keys.push_back(token_.getValue());
            nextToken();
            return Ok(keys);
        }

        keys.push_back(TRY(expect(kKey)).getValue());
        while (consume(kDot)) {
            keys.push_back(TRY(expect(kKey)).getValue());
        }

        return Ok(keys);
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
            consumeNewlines();

            Array array;
            if (consume(kRBracket)) {
                value = Value(array);
                return Ok(value);
            }

            array.addElement(TRY(parseVal()));

            while (!peek(kRBracket | kEof)) {
                TRY(expect(kComma));
                consumeNewlines();

                if (peek(kRBracket)) break;

                array.addElement(TRY(parseVal()));
                consumeNewlines();
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
            table = table.readOnly();
            value = Value(table);
        } else {
            return Err(error::AppError::kParseUnknown);
        }

        return Ok(value);
    }
}
