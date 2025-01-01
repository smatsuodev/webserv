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

Option<double> inverse(const int x) {
    if (x == 0) {
        return None;
    }
    return Some(1.0 / x);
}

Option<double> inverse(const double x) {
    if (x == 0) {
        return None;
    }
    return Some(1.0 / x);
}

TEST(OptionTest, noneAndThen) {
    const Option<int> op = None;
    const auto res = op.andThen(inverse);
    EXPECT_TRUE(res.isNone());
}

TEST(OptionTest, someAndThen) {
    const Option<int> op = Some(2);
    const auto res = op.andThen(inverse);
    EXPECT_TRUE(res.isSome());
    EXPECT_DOUBLE_EQ(res.unwrap(), 0.5);
}

TEST(OptionTest, someAndThenNone) {
    const Option<int> op = Some(0);
    const auto res = op.andThen(inverse);
    EXPECT_TRUE(res.isNone());
}

TEST(OptionTest, chainAndThen) {
    const Option<int> op = Some(2);
    const auto res = op.andThen(inverse).andThen(inverse);
    EXPECT_TRUE(res.isSome());
    EXPECT_DOUBLE_EQ(res.unwrap(), 2.0);
}

int addOne(const int x) {
    return x + 1;
}

TEST(OptionTest, noneMap) {
    const Option<int> op = None;
    const auto res = op.map(addOne);
    EXPECT_TRUE(res.isNone());
}

TEST(OptionTest, someMap) {
    const Option<int> op = Some(1);
    const auto res = op.map(addOne);
    EXPECT_TRUE(res.isSome());
    EXPECT_EQ(res.unwrap(), 2);
}

TEST(OptionTest, chainMap) {
    const Option<int> op = Some(1);
    const auto res = op.map(addOne).map(addOne);
    EXPECT_TRUE(res.isSome());
    EXPECT_EQ(res.unwrap(), 3);
}

// 前は Some<std::string> の注釈が必要だった
TEST(OptionTest, optionalString) {
    const Option<std::string> op = Some("hello");
    EXPECT_TRUE(op.isSome());
    EXPECT_EQ(op.unwrap(), "hello");
}
