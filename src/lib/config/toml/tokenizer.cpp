#include "tokenizer.hpp"

#include "value.hpp"
#include "utils/logger.hpp"
#include "utils/types/option.hpp"
#include "utils/types/try.hpp"

#include <map>
#include <sstream>
#include <stack>

namespace toml {
    Token::Token(const TokenType type, const std::string &value) : type_(type), value_(value) {}

    TokenType Token::getType() const {
        return type_;
    }

    const std::string &Token::getValue() const {
        return value_;
    }

    Tokenizer::TokenizeResult Tokenizer::tokenize(std::istream &input) {
        std::vector<Tokens> tokensList;

        std::string line;
        while (std::getline(input, line)) {
            const std::string trimmedLine = utils::trim(line);
            if (trimmedLine.empty()) {
                continue;
            }

            Tokens tokens;
            switch (trimmedLine[0]) {
                case '#':
                    // 無視
                    break;
                case '[': {
                    Result<Tokens, std::string> result = Tokenizer::tokenizeTableHeader(trimmedLine);
                    if (result.isErr()) {
                        return Err(result.unwrapErr());
                    }
                    tokens = result.unwrap();
                    break;
                }
                default: {
                    // key = value
                    Result<Tokens, std::string> result = Tokenizer::tokenizeKeyValue(trimmedLine);
                    if (result.isErr()) {
                        return Err(result.unwrapErr());
                    }
                    tokens = result.unwrap();
                    break;
                }
            }

            if (!tokens.empty()) {
                tokensList.insert(tokensList.end(), tokens.begin(), tokens.end());
            }
        }

        return Ok(tokensList);
    }

    Result<Tokenizer::Tokens, std::string> Tokenizer::tokenizeTableHeader(const std::string &rawTableHeader) {
        Tokens tokens;

        // 念の為 trim
        const std::string trimmed = utils::trim(rawTableHeader);
        if (trimmed.empty()) {
            return Err(utils::format("invalid table header: %s", rawTableHeader.c_str()));
        }
        if (!(trimmed[0] == '[' && trimmed[trimmed.size() - 1] == ']')) {
            return Err(utils::format("invalid table header: %s", rawTableHeader.c_str()));
        }
        tokens.push_back(Token(kTableHeaderOpen, "["));

        const std::string headerKey = utils::trim(trimmed.substr(1, trimmed.size() - 2));
        const Result<Tokens, std::string> result = Tokenizer::tokenizeKey(headerKey);
        if (result.isErr()) {
            return Err(result.unwrapErr());
        }
        const Tokens tmp = result.unwrap();
        tokens.insert(tokens.end(), tmp.begin(), tmp.end());

        tokens.push_back(Token(kTableHeaderClose, "]"));

        return Ok(tokens);
    }

    Result<Tokenizer::Tokens, std::string> Tokenizer::tokenizeKeyValue(const std::string &rawKeyValue) {
        Tokens tokens;

        // TODO: (key, value) に分ける関数を作る
        const Option<std::size_t> assignmentPos = Tokenizer::findAssignment(rawKeyValue);
        if (assignmentPos.isNone()) {
            return Err(utils::format("invalid key-value: %s", rawKeyValue.c_str()));
        }
        const std::size_t pos = assignmentPos.unwrap();
        const std::string rawKey = utils::trim(rawKeyValue.substr(0, pos));
        const std::string rawValue = utils::trim(rawKeyValue.substr(pos + 1));

        const Result<Tokens, std::string> keyTokensResult = Tokenizer::tokenizeKey(rawKey);
        if (keyTokensResult.isErr()) {
            return Err(keyTokensResult.unwrapErr());
        }
        const Tokens keyTokens = keyTokensResult.unwrap();
        tokens.insert(tokens.end(), keyTokens.begin(), keyTokens.end());

        tokens.push_back(Token(kAssignment, "="));

        const Result<Tokens, std::string> result = Tokenizer::tokenizeValue(rawValue);
        if (result.isErr()) {
            return Err(result.unwrapErr());
        }
        const Tokens valueTokens = result.unwrap();
        tokens.insert(tokens.end(), valueTokens.begin(), valueTokens.end());

        return Ok(tokens);
    }

    // TODO: 途中に # があるとまずい
    Result<std::vector<Token>, std::string> Tokenizer::tokenizeKey(const std::string &rawKey) {
        std::vector<Token> tokens;

        const std::vector<std::string> splitted = utils::split(rawKey, '.');
        // a."b.c".d のように、quote された . に対応するために必要
        Option<char> quoteCh = None;
        std::vector<std::string> quoted;
        for (std::size_t i = 0; i < splitted.size(); ++i) {
            const std::string &elem = splitted[i];

            // "a . b" のような場合を考慮して、trim 前に quote を確認
            // quote 終了
            if (quoteCh.isSome() && elem[elem.size() - 1] == quoteCh.unwrap()) {
                quoted.push_back(elem);

                std::string joined = utils::join(quoted, ".");
                tokens.push_back(Token(kKey, joined));
                if (i == splitted.size() - 1) {
                    tokens.push_back(Token(kDot, "."));
                }

                quoteCh = None;
                quoted.clear();
                continue;
            }
            // quote 開始
            if (elem[0] == '"' || elem[0] == '\'') {
                quoteCh = Some(elem[0]);
                quoted.push_back(elem);
                continue;
            }

            const std::string trimmed = utils::trim(elem);
            if (trimmed.empty()) {
                return Err(utils::format("invalid key: %s", rawKey.c_str()));
            }

            tokens.push_back(Token(kKey, trimmed));
            if (i == splitted.size() - 1) {
                tokens.push_back(Token(kDot, "."));
            }
        }

        if (quoteCh.isSome()) {
            return Err(utils::format("missing quote: %s", rawKey.c_str()));
        }

        return Ok(tokens);
    }

    // TODO: 途中に # があるとまずい
    Result<Tokenizer::Tokens, std::string> Tokenizer::tokenizeValue(const std::string &rawValue) {
        // 念の為 trim
        const std::string trimmed = utils::trim(rawValue);
        if (trimmed.empty()) {
            return Err<std::string>("value is empty");
        }

        switch (trimmed[0]) {
            case '[':
                return Tokenizer::tokenizeArray(trimmed);
            case '{':
                return Tokenizer::tokenizeInlineTable(trimmed);
            default: {
                Tokens tokens;
                tokens.push_back(Token(kValue, trimmed));
                return Ok(tokens);
            }
        }
    }

    // TODO: 途中に # があるとまずい
    Result<Tokenizer::Tokens, std::string> Tokenizer::tokenizeArray(const std::string &rawArray) {
        Tokens tokens;

        // splitElements は Array でなければエラーを返す
        const Result<std::vector<std::string>, std::string> result = Tokenizer::splitElements(rawArray);
        if (result.isErr()) {
            return Err(result.unwrapErr());
        }
        const std::vector<std::string> elements = result.unwrap();

        tokens.push_back(Token(kArrayOpen, "["));

        for (std::size_t i = 0; i < elements.size(); ++i) {
            const Result<Tokens, std::string> r = Tokenizer::tokenizeValue(elements[i]);
            if (r.isErr()) {
                return Err(r.unwrapErr());
            }
            const Tokens tmp = r.unwrap();
            tokens.insert(tokens.end(), tmp.begin(), tmp.end());
            if (i == elements.size() - 1) {
                tokens.push_back(Token(kComma, ","));
            }
        }

        tokens.push_back(Token(kArrayClose, "]"));

        return Ok(tokens);
    }

    // TODO: 途中に # があるとまずい
    Result<Tokenizer::Tokens, std::string> Tokenizer::tokenizeInlineTable(const std::string &rawInlineTable) {
        Tokens tokens;

        // splitElements は InlineTable でなければエラーを返す
        const Result<std::vector<std::string>, std::string> result = Tokenizer::splitElements(rawInlineTable);
        if (result.isErr()) {
            return Err(result.unwrapErr());
        }
        const std::vector<std::string> elements = result.unwrap();

        tokens.push_back(Token(kInlineTableOpen, "{"));

        for (std::size_t i = 0; i < elements.size(); ++i) {
            const Result<Tokens, std::string> r = Tokenizer::tokenizeKeyValue(elements[i]);
            if (r.isErr()) {
                return Err(r.unwrapErr());
            }
            const Tokens tmp = r.unwrap();
            tokens.insert(tokens.end(), tmp.begin(), tmp.end());
            if (i == elements.size() - 1) {
                tokens.push_back(Token(kComma, ","));
            }
        }

        tokens.push_back(Token(kInlineTableClose, "]"));

        return Ok(tokens);
    }

    /*
     * Array -> 要素の vector
     * "[1, 2, 3]" -> {"1", "2", "3"}
     *
     * Inline Table -> key-value の vector
     * "{k1 = 1, k2 = 2}" -> {"k1=1", "k2=2"}
     *
     * カンマが quote される場合に気をつける
     * ["1,2", 3] -> {"1,2", "3"}
     */
    Result<std::vector<std::string>, std::string> Tokenizer::splitElements(const std::string &str) {
        const bool isArray = utils::startsWith(str, "[") && utils::endsWith(str, "]");
        const bool isInlineTable = utils::startsWith(str, "{") && utils::endsWith(str, "}");
        if (!isArray && !isInlineTable) {
            return Err(utils::format("invalid array or inline table: %s", str.c_str()));
        }

        const std::string rawElements = utils::trim(str.substr(1, str.size() - 2));
        if (rawElements.empty()) {
            // 空配列
            return Ok(std::vector<std::string>());
        }

        std::map<char, char> closeChMap;
        closeChMap['"'] = '"';
        closeChMap['\''] = '\'';
        closeChMap['['] = ']';
        closeChMap['{'] = '}';

        std::vector<std::string> elements;
        std::string elem;
        std::stack<char> openChStack;
        for (std::size_t i = 0; i < str.size(); ++i) {
            const char c = str[i];

            if (openChStack.empty() && c == ',') {
                elements.push_back(utils::toString(elem));
                elem.clear();
                continue;
            }

            elem += c;

            if (!openChStack.empty() && openChStack.top() == closeChMap[c]) {
                openChStack.pop();
                continue;
            }
            if (c == '"' || c == '\'' || c == '[' || c == '{') {
                openChStack.push(c);
                continue;
            }
        }

        return Ok(elements);
    }

    // "=" = 1 みたいなことができるので、quote されたものは無視して探す
    Option<std::size_t> Tokenizer::findAssignment(const std::string &line) {
        Option<char> quoteCh = None;
        for (std::size_t i = 0; i < line.size(); ++i) {
            // quote 終了
            if (line[i] == quoteCh.unwrap()) {
                quoteCh = None;
                continue;
            }
            // quote 開始
            if (line[i] == '"' || line[i] == '\'') {
                quoteCh = Some(line[i]);
                continue;
            }

            if (quoteCh.isNone() && line[i] == '=') {
                return Some(i);
            }
        }
        return None;
    }
}
