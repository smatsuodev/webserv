#include "config/toml/tokenizer.hpp"
#include <gtest/gtest.h>

using namespace toml;

TEST(TomlTokenizerOk, keyValue) {
    const std::string document = "key=value";

    const auto result = Tokenizer::tokenize(document);
    EXPECT_TRUE(result.isOk());

    const auto lst = result.unwrap();
    EXPECT_EQ(lst.size(), 1);

    const auto &tokens = lst[0];
    std::vector<TokenType> types;
    for (const auto &token : tokens) {
        types.push_back(token.getType());
    }

    const std::vector expected = {kKey, kAssignment, kValue};
    EXPECT_EQ(types, expected);
}
