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
    }
};

TEST_F(MatcherTest, exactMatch) {
    EXPECT_EQ(matcher.match("/"), Value::kValue1);
    EXPECT_EQ(matcher.match("/path"), Value::kValue2);
    EXPECT_EQ(matcher.match("/path/longer"), Value::kValue3);
    EXPECT_EQ(matcher.match("/other"), Value::kValue4);
}
