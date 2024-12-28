#include "tokenizer.hpp"

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
        std::vector<Token> tokens;

        std::string line;
        while (std::getline(input, line)) {
            const std::string trimmedLine = utils::trim(line);
            if (trimmedLine.empty()) {
                continue;
            }

            const char back = trimmedLine[trimmedLine.size() - 1];
            switch (trimmedLine[0]) {
                case '#':
                    // 無視
                    break;
                case '[': {
                    if (back != ']') {
                        return Err<std::string>(utils::format("invalid table header: %s", trimmedLine.c_str()));
                    }
                    std::string headerKey = utils::trim(trimmedLine.substr(1, trimmedLine.size() - 2));
                    tokens.push_back(Token(kTableHeaderOpen, headerKey));
                    break;
                }
                default: {
                    // key = value
                    Option<std::size_t> assignmentPos = Tokenizer::findAssignment(trimmedLine);
                    if (assignmentPos.isNone()) {
                        return Err<std::string>(utils::format("invalid key-value: %s", trimmedLine.c_str()));
                    }
                    const std::size_t pos = assignmentPos.unwrap();

                    const std::string rawKey = utils::trim(trimmedLine.substr(0, pos));
                    Result<Tokens, std::string> keyTokensResult = Tokenizer::tokenizeKey(rawKey);
                    if (keyTokensResult.isErr()) {
                        return Err(keyTokensResult.unwrapErr());
                    }
                    const Tokens keyTokens = keyTokensResult.unwrap();
                    tokens.insert(tokens.end(), keyTokens.begin(), keyTokens.end());

                    tokens.push_back(Token(kAssignment, "="));

                    const std::string rawValue = utils::trim(trimmedLine.substr(pos + 1));
                    Tokens valueTokens;
                    if (rawValue[0] == '[' && rawValue[rawValue.size() - 1] != ']') {
                        // 複数行の Array
                        // Array の終わりまで読み込む
                        Result<std::vector<std::string>, std::string> result =
                            Tokenizer::readUntilArrayClose(rawValue, input);
                        if (result.isErr()) {
                            return Err(result.unwrapErr());
                        }

                        Result<Tokens, std::string> tokenizeResult = Tokenizer::tokenizeMultiLineArray(result.unwrap());
                        if (tokenizeResult.isErr()) {
                            return Err(tokenizeResult.unwrapErr());
                        }
                        valueTokens = tokenizeResult.unwrap();
                    } else {
                        Result<Tokens, std::string> result = Tokenizer::tokenizeValue(rawValue);
                        if (result.isErr()) {
                            return Err(result.unwrapErr());
                        }
                        valueTokens = result.unwrap();
                    }
                    tokens.insert(tokens.end(), valueTokens.begin(), valueTokens.end());
                }
            }
        }

        return Ok(tokens);
    }

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
                return Err<std::string>(utils::format("invalid key: %s", rawKey.c_str()));
            }

            tokens.push_back(Token(kKey, trimmed));
            if (i == splitted.size() - 1) {
                tokens.push_back(Token(kDot, "."));
            }
        }

        if (quoteCh.isSome()) {
            return Err<std::string>(utils::format("missing quote: %s", rawKey.c_str()));
        }

        return Ok(tokens);
    }

    Result<std::vector<Token>, std::string> Tokenizer::tokenizeValue(const std::string &rawValue) {
        std::vector<Token> tokens;
        return Ok(tokens);
    }

    Result<Tokenizer::Tokens, std::string> tokenizeMultiLineArray(const std::vector<std::string> &lines) {}

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

    Result<std::vector<std::string>, std::string> Tokenizer::readUntilArrayClose(const std::string &firstLine,
                                                                                 std::istream &input) {
        std::map<char, char> closeChMap;
        closeChMap['"'] = '"';
        closeChMap['\''] = '\'';
        closeChMap['['] = ']';
        closeChMap['{'] = '}';

        std::vector<std::string> lines;
        std::stack<char> stack;
        std::string line = firstLine;
        do {
            for (std::size_t i = 0; i < line.size(); ++i) {
                const char c = line[i];
                if (!stack.empty() && closeChMap[c] == stack.top()) {
                    stack.pop();
                } else if (c == '"' || c == '\'' || c == '[' || c == '{') {
                    stack.push(c);
                }
            }

            std::string trimmedLine = utils::trim(line);
            if (trimmedLine.empty() || trimmedLine[0] == '#') {
                continue;
            }
            lines.push_back(trimmedLine);

            if (stack.empty()) {
                // Array の終わりが見つかった
                return Ok(lines);
            }
        } while (std::getline(input, line));

        if (!stack.empty()) {
            return Err(utils::format("missing `%c'", closeChMap[stack.top()]));
        }

        // 実行されないはず
        return  Err<std::string>("unknown");
    }
}
