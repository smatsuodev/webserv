#ifndef SRC_LIB_CONFIG_TOML_PARSER_HPP
#define SRC_LIB_CONFIG_TOML_PARSER_HPP

#include "value.hpp"
#include "utils/types/result.hpp"
#include "utils/types/error.hpp"

namespace toml {
    class TomlParser {
    public:
        TomlParser();
        ~TomlParser();

        static Result<Table, error::AppError> parse(std::istream &input);

    private:
        static std::string trim(const std::string &s);
        static std::string toLower(const std::string &s);
        static std::vector<std::string> split(const std::string &s, char delim);
        static bool isInteger(const std::string &s);

        static Result<Table *, error::AppError> parseTableHeader(Table *root, const std::string &line);
        typedef Result<std::pair<std::string, Value>, error::AppError> ParseKeyValueResult;
        static ParseKeyValueResult parseKeyValue(const std::string &line);
        static Result<Value, error::AppError> parseValue(const std::string &rawValue);
    };
}

#endif
