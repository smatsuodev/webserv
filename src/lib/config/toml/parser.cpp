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
                nextToken();
                if (peek(kLBracket)) {
                    table = TRY(parseArrayOfTableHeader(table));
                } else {
                    table = TRY(parseTableHeader(table));
                }
            } else {
                table = TRY(parseKeyVal(table));
                expectNewlines();
            }
        }

        return Ok(table);
    }

    Result<Table, error::AppError> TomlParser::parseArrayOfTableHeader(Table table) {
        nextToken();
        const std::vector<std::string> keys = TRY(parseKey());
        TRY(expect(kRBracket));
        TRY(expect(kRBracket));

        if (keys.empty()) {
            return Err(error::AppError::kParseUnknown);
        }

        Table *currentTable = TRY(table.findOrCreateTablePath(keys));
        const std::string &lastKey = keys.back();
        const Option<Value> lastValue = currentTable->getValue(lastKey);
        const Table newTable;

        if (lastValue.isSome()) {
            Value &value = currentTable->getValueRef(lastKey);
            if (value.getType() == Value::kArray) {
                Array &array = value.getArrayRef();
                if (array.size() == 0) {
                    return Err(error::AppError::kParseUnknown);
                }
                array.addElement(Value(newTable));
                currentTable = &array.getElementRef(array.size() - 1).getTableRef();
            } else {
                return Err(error::AppError::kParseUnknown);
            }
        } else {
            Array array;
            array.addElement(Value(newTable));
            TRY(currentTable->setValue(lastKey, Value(array)).okOr(error::kParseUnknown));
            currentTable = &currentTable->getValueRef(lastKey).getArrayRef().getElementRef(0).getTableRef();
        }

        TRY(parseTableContent(*currentTable));
        return Ok(table);
    }

    Result<Table, error::AppError> TomlParser::parseTableHeader(Table table) {
        const std::vector<std::string> keys = TRY(parseKey());
        TRY(expect(kRBracket));

        if (keys.empty()) {
            return Err(error::AppError::kParseUnknown);
        }

        Table *currentTable = TRY(table.findOrCreateTablePath(keys));
        const std::string &lastKey = keys.back();
        const Table newTable;

        if (currentTable->getValue(lastKey).isSome()) {
            return Err(error::AppError::kParseUnknown);
        }

        currentTable->setValue(lastKey, Value(newTable));
        currentTable = &currentTable->getValueRef(lastKey).getTableRef();

        TRY(parseTableContent(*currentTable));
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

        if (peek(kString | kNum)) {
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

    Result<types::Unit, error::AppError> TomlParser::parseTableContent(Table &table) {
        while (!peek(kLBracket) && !peek(kEof)) {
            if (peek(kNewLine)) {
                consumeNewlines();
                if (peek(kLBracket) || peek(kEof)) {
                    break;
                }
            }

            table = TRY(parseKeyVal(table));
            TRY(expectNewlines());
        }

        return Ok(types::Unit());
    }
}
