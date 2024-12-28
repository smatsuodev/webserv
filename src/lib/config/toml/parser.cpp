#include "parser.hpp"
#include <sstream>
#include <cstdlib>

namespace toml {
    TomlParser::TomlParser() {}

    TomlParser::~TomlParser() {}

    Result<Table, error::AppError> TomlParser::parse(std::istream &input) {
        Table rootTable;
        Table *currentTable = &rootTable;

        std::string line;
        while (std::getline(input, line)) {
            std::string trimmedLine = TomlParser::trim(line);
            if (trimmedLine.empty() || trimmedLine[0] == '#') continue;

            if (trimmedLine[0] == '[') {
                Result<Table *, error::AppError> tablePtrResult = parseTableHeader(currentTable, trimmedLine);
                if (tablePtrResult.isErr()) {
                    return Err(tablePtrResult.unwrapErr());
                }
                currentTable = tablePtrResult.unwrap();
            } else {
                ParseKeyValueResult kvResult = parseKeyValue(trimmedLine);
                if (kvResult.isErr()) {
                    return Err(kvResult.unwrapErr());
                }
                const std::pair<std::string, Value> kv = kvResult.unwrap();
                currentTable->setValue(kv.first, kv.second);
            }
        }

        return Ok(rootTable);
    }

    Result<Table *, error::AppError> TomlParser::parseTableHeader(Table *root, const std::string &line) {
        if (line[line.size() - 1] != ']') {
            return Err<error::AppError>(error::kParseUnknown);
        }

        std::string header = line.substr(1, line.length() - 2);
        header = TomlParser::trim(header);
        if (header.empty()) {
            return Err<error::AppError>(error::kParseUnknown);
        }

        // ネストされたテーブル [a.b.c] を処理
        const std::vector<std::string> parts = split(header, '.');
        Table *current = root;
        for (size_t i = 0; i < parts.size(); ++i) {
            std::string part = TomlParser::trim(parts[i]);
            if (part.empty()) {
                return Err<error::AppError>(error::kParseUnknown);
            }

            Option<Value> existingValue = current->getValue(part);
            if (existingValue.isSome()) {
                // 既存のテーブル
                const Option<Table *> table = existingValue.unwrap().getTablePointer();
                if (table.isNone()) {
                    return Err<error::AppError>(error::kParseUnknown);
                }
                current = table.unwrap();
            } else {
                // 新しいテーブルを作成
                current->setValue(part, Value(Value::kTable));
                current = current->getValues()[part].getTablePointer().unwrap();
            }
        }

        return Ok(current);
    }

    TomlParser::ParseKeyValueResult TomlParser::parseKeyValue(const std::string &line) {
        const size_t eq = line.find('=');
        if (eq == std::string::npos) {
            return Err<error::AppError>(error::kParseUnknown);
        }

        const std::string key = trim(line.substr(0, eq));
        const std::string rawValue = trim(line.substr(eq + 1));

        if (key.empty() || rawValue.empty()) {
            return Err<error::AppError>(error::kParseUnknown);
        }

        const Result<Value, error::AppError> valueResult = parseValue(rawValue);
        if (valueResult.isErr()) {
            return Err(valueResult.unwrapErr());
        }

        return Ok(std::make_pair(key, valueResult.unwrap()));
    }

    Result<Value, error::AppError> TomlParser::parseValue(const std::string &rawValue) {
        if (rawValue.empty()) {
            return Err<error::AppError>(error::kParseUnknown);
        }

        // String
        if (rawValue[0] == '\"') {
            if (rawValue[rawValue.size() - 1] != '\"') {
                return Err<error::AppError>(error::kParseUnknown);
            }
            const std::string strContent = rawValue.substr(1, rawValue.length() - 2);
            return Ok(Value(strContent));
        }

        // Array
        if (rawValue[0] == '[') {
            if (rawValue[rawValue.size() - 1] != ']') {
                return Err<error::AppError>(error::kParseUnknown);
            }
            const std::string inner = rawValue.substr(1, rawValue.length() - 2);
            Array array;
            const std::vector<std::string> elements = split(inner, ',');
            for (size_t i = 0; i < elements.size(); ++i) {
                std::string elemStr = trim(elements[i]);
                Result<Value, error::AppError> elemResult = parseValue(elemStr);
                if (elemResult.isErr()) {
                    return elemResult;
                }
                array.addElement(elemResult.unwrap());
            }
            return Ok(Value(array));
        }

        // Integer
        if (isInteger(rawValue)) {
            const long intVal = strtol(rawValue.c_str(), NULL, 10);
            return Ok(Value(intVal));
        }

        // Boolean
        const std::string lower = toLower(rawValue);
        if (lower == "true") {
            return Ok(Value(true));
        }
        if (lower == "false") {
            return Ok(Value(false));
        }

        return Err<error::AppError>(error::kParseUnknown);
    }

    std::string TomlParser::trim(const std::string &s) {
        size_t start = 0;
        while (start < s.length() && std::isspace(static_cast<unsigned char>(s[start])))
            start++;
        size_t end = s.length();
        while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1])))
            end--;
        return s.substr(start, end - start);
    }

    std::string TomlParser::toLower(const std::string &s) {
        std::string res = s;
        for (size_t i = 0; i < res.length(); ++i)
            res[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(res[i])));
        return res;
    }

    std::vector<std::string> TomlParser::split(const std::string &s, const char delim) {
        std::vector<std::string> elems;
        std::stringstream ss(s);
        std::string item;
        while (std::getline(ss, item, delim)) {
            elems.push_back(item);
        }
        return elems;
    }

    bool TomlParser::isInteger(const std::string &s) {
        if (s.empty()) return false;
        size_t start = 0;
        if (s[0] == '-' || s[0] == '+') start = 1;
        for (size_t i = start; i < s.length(); ++i)
            if (!std::isdigit(static_cast<unsigned char>(s[i]))) return false;
        return true;
    }
}
