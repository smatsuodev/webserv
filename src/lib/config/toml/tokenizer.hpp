#ifndef SRC_LIB_CONFIG_TOML_TOKENIZER_HPP
#define SRC_LIB_CONFIG_TOML_TOKENIZER_HPP

#include "utils/types/option.hpp"
#include "utils/types/result.hpp"
#include <string>
#include <vector>

namespace toml {
    enum TokenType {
        kKey,
        kDot,
        kAssignment,
        kValue,
        kTableHeaderOpen,
        kTableHeaderClose,
        kInlineTableOpen,
        kInlineTableClose,
        kArrayOpen,
        kArrayClose
    };

    class Token {
    public:
        explicit Token(TokenType type, const std::string &value);

        TokenType getType() const;
        const std::string &getValue() const;

    private:
        TokenType type_;
        std::string value_;
    };

    class Tokenizer {
    public:
        typedef std::vector<Token> Tokens;
        typedef Result<Tokens, std::string> TokenizeResult;
        static TokenizeResult tokenize(std::istream &input);

    private:
        static Result<Tokens, std::string> tokenizeKey(const std::string &rawKey);
        static Result<Tokens, std::string> tokenizeValue(const std::string &rawValue);
        static Result<Tokens, std::string> tokenizeMultiLineArray(const std::vector<std::string> &lines);
        static Option<std::size_t> findAssignment(const std::string &line);
        static Result<std::vector<std::string>, std::string> readUntilArrayClose(const std::string &firstLine,
                                                                                 std::istream &input);
    };
}

#endif
