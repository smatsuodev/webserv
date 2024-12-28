#include "config/toml/tokenizer.hpp"
#include <gtest/gtest.h>

using namespace toml;

TEST(TomlTokenizerOk, keyValue) {
    const std::string document = "key=value";

    const auto result = Tokenizer::tokenize(document);
    ASSERT_TRUE(result.isOk());

    const auto tokens = result.unwrap();
    EXPECT_EQ(tokens.size(), 4);

    std::vector<TokenType> types;
    for (const auto &token : tokens) {
        types.push_back(token.getType());
    }

    const std::vector expected = {kKey, kAssignment, kValue, kNewLine};
    EXPECT_EQ(types, expected);
}
