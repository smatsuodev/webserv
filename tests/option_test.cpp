#include <gtest/gtest.h>
#include "utils/types/option.hpp"
#include "utils/types/try.hpp"

TEST(OptionTest, constructSomeInt) {
    const Option<int> target = Some(0);
    EXPECT_TRUE(target.isSome());
    EXPECT_FALSE(target.isNone());
}

TEST(OptionTest, constructNone) {
    const Option<int> target = None;
    EXPECT_TRUE(target.isNone());
    EXPECT_FALSE(target.isSome());
}

TEST(OptionTest, eqSameSomes) {
    EXPECT_TRUE(Some(1) == Some(1));
}

TEST(OptionTest, eqDistinctSomes) {
    EXPECT_FALSE(Some(1) == Some(2));
}

TEST(OptionTest, neDistinctSomes) {
    EXPECT_TRUE(Some(1) != Some(2));
}

TEST(OptionTest, neSameSomes) {
    EXPECT_FALSE(Some(1) != Some(1));
}

TEST(OptionTest, eqNones) {
    EXPECT_TRUE(types::None() == types::None());
}

TEST(OptionTest, neNones) {
    EXPECT_FALSE(types::None() != types::None());
}

TEST(OptionTest, eqSameSomeOptions) {
    const Option<int> op1 = Some(1);
    const Option<int> op2 = Some(1);
    EXPECT_TRUE(op1 == op2);
}

TEST(OptionTest, eqDistinctSomeOptions) {
    const Option<int> op1 = Some(1);
    const Option<int> op2 = Some(2);
    EXPECT_FALSE(op1 == op2);
}

TEST(OptionTest, neDistinctSomeOptions) {
    const Option<int> op1 = Some(1);
    const Option<int> op2 = Some(2);
    EXPECT_TRUE(op1 != op2);
}

TEST(OptionTest, neSameSomeOptions) {
    const Option<int> op1 = Some(1);
    const Option<int> op2 = Some(1);
    EXPECT_FALSE(op1 != op2);
}

TEST(OptionTest, eqNoneOptions) {
    const Option<int> op1 = None;
    const Option<int> op2 = None;
    EXPECT_TRUE(op1 == op2);
}

TEST(OptionTest, neNoneOptions) {
    const Option<int> op1 = None;
    const Option<int> op2 = None;
    EXPECT_FALSE(op1 != op2);
}

TEST(OptionTest, eqSomeAndNoneOptions) {
    const Option<int> op1 = Some(1);
    const Option<int> op2 = None;
    EXPECT_FALSE(op1 == op2);
    EXPECT_FALSE(op2 == op1);
}

TEST(OptionTest, neSomeAndNoneOptions) {
    const Option<int> op1 = Some(1);
    const Option<int> op2 = None;
    EXPECT_TRUE(op1 != op2);
    EXPECT_TRUE(op2 != op1);
}

TEST(OptionTest, unwrapSome) {
    const Option<int> target = Some(1);
    EXPECT_EQ(target.unwrap(), 1);
}

TEST(OptionTest, unwrapNone) {
    const Option<int> target = None;
    EXPECT_THROW(target.unwrap(), std::runtime_error);
}

TEST(OptionTest, unwrapOrSome) {
    const Option<std::string> target = Some<std::string>("some");
    EXPECT_EQ(target.unwrapOr("none"), "some");
}

TEST(OptionTest, unwrapOrNone) {
    const Option<std::string> target = None;
    EXPECT_EQ(target.unwrapOr("none"), "none");
}

Option<int> plusOne(Option<int> &op) {
    const auto x = TRY(op);
    return Some(x + 1);
}

TEST(OptionTest, trySome) {
    Option<int> op = Some(1);
    const auto res = plusOne(op);
    EXPECT_TRUE(res.isSome());
    EXPECT_EQ(res.unwrap(), 2);
}

TEST(OptionTest, tryNone) {
    Option<int> op = None;
    const auto res = plusOne(op);
    EXPECT_TRUE(res.isNone());
}

int plusOneOr(Option<int> &op, const int defaultValue) {
    const auto x = TRY_OR(op, defaultValue);
    return x + 1;
}

TEST(OptionTest, tryOrSome) {
    Option<int> op = Some(1);
    const auto res = plusOneOr(op, 0);
    EXPECT_EQ(res, 2);
}

TEST(OptionTest, tryOrNone) {
    Option<int> op = None;
    const auto res = plusOneOr(op, 0);
    EXPECT_EQ(res, 0);
}

TEST(OptionTest, assignSome2Some) {
    Option<int> op = Some(1);
    op = Some(1);
    EXPECT_EQ(op.unwrap(), 1);
}

// メモリリークが起きたことがある
TEST(OptionTest, assignSome2None) {
    Option<int> op = Some(1);
    op = None;
    EXPECT_EQ(op, None);
}

TEST(OptionTest, assignNone2None) {
    Option<int> op = None;
    op = None;
    EXPECT_EQ(op, None);
}

TEST(OptionTest, assignNone2Some) {
    Option<int> op = None;
    op = Some(1);
    EXPECT_EQ(op.unwrap(), 1);
}

TEST(OptionTest, exprIsEvaluatedOnce) {
    int cnt = 0;
    auto func = [&]() -> Option<bool> {
        cnt++;
        return None;
    };
    auto callTry = [&]() -> Option<bool> {
        TRY(func());
        return None;
    };

    callTry();

    EXPECT_EQ(cnt, 1);
}

TEST(OptionTest, okOrSome) {
    Option<int> op = Some(1);
    Result<int, int> res = op.okOr(2);

    EXPECT_TRUE(res.isOk());
    EXPECT_EQ(res.unwrap(), 1);
}

TEST(OptionTest, okOrNone) {
    Option<int> op = None;
    Result<int, int> res = op.okOr(2);

    EXPECT_TRUE(res.isErr());
    EXPECT_EQ(res.unwrapErr(), 2);
}
