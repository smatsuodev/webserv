#include <gtest/gtest.h>
#include "http/handler/matcher.hpp"

using namespace http;

class MatcherTest : public ::testing::Test {
public:
    enum class Value { kValue1, kValue2, kValue3, kValue4 };
    std::map<std::string, Value> paths;
    Matcher<Value> matcher;

    MatcherTest() : matcher({}) {};

protected:
    void SetUp() override {
        paths = {{"/", Value::kValue1},
                 {"/path", Value::kValue2},
                 {"/path/longer", Value::kValue3},
                 {"/other", Value::kValue4}};
        matcher = Matcher(paths);
    }
};

TEST_F(MatcherTest, exactMatch) {
    auto result = matcher.match("/");
    ASSERT_TRUE(result.isSome());
    EXPECT_EQ(result.unwrap(), Value::kValue1);

    result = matcher.match("/path");
    ASSERT_TRUE(result.isSome());
    EXPECT_EQ(result.unwrap(), Value::kValue2);

    result = matcher.match("/path/longer");
    ASSERT_TRUE(result.isSome());
    EXPECT_EQ(result.unwrap(), Value::kValue3);

    result = matcher.match("/other");
    ASSERT_TRUE(result.isSome());
    EXPECT_EQ(result.unwrap(), Value::kValue4);
}

TEST_F(MatcherTest, noMatch) {
    const auto result = matcher.match("/not_found");
    ASSERT_TRUE(result.isNone());
}

TEST_F(MatcherTest, mustLongestMatch) {
    const auto result = matcher.match("/path/longer/extra");
    ASSERT_TRUE(result.isSome());
    EXPECT_EQ(result.unwrap(), Value::kValue3);
}

TEST_F(MatcherTest, matchRoot) {
    const auto result = matcher.match("/foo/bar/baz");
    ASSERT_TRUE(result.isSome());
    EXPECT_EQ(result.unwrap(), Value::kValue1);
}

TEST_F(MatcherTest, empty) {
    const auto result = matcher.match("");
    ASSERT_TRUE(result.isNone());
}

TEST_F(MatcherTest, trailingSlash) {
    const auto result = matcher.match("/path/");
    ASSERT_TRUE(result.isSome());
    EXPECT_EQ(result.unwrap(), Value::kValue2);
}

TEST_F(MatcherTest, extraSlash) {
    const auto result = matcher.match("/path//longer///");
    ASSERT_TRUE(result.isSome());
    EXPECT_EQ(result.unwrap(), Value::kValue3);
}

TEST_F(MatcherTest, partial) {
    // 上書き
    matcher = Matcher<Value>({std::make_pair("/path", Value::kValue1)});

    const auto result = matcher.match("/pa");
    ASSERT_TRUE(result.isNone());
}

TEST(MatcherTest, emptyMap) {
    const Matcher<int> matcher({});
    const auto result = matcher.match("/path");
    ASSERT_TRUE(result.isNone());
}
