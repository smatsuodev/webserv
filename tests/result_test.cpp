#include "utils/types/result.hpp"
#include "utils/types/try.hpp"
#include <gtest/gtest.h>

TEST(ResultTest, constructOkInt) {
    const Result<int, std::string> target = Ok(0);
    EXPECT_TRUE(target.isOk());
    EXPECT_FALSE(target.isErr());
}

TEST(ResultTest, constructErrString) {
    const Result<int, std::string> target = Err<std::string>("error");
    EXPECT_TRUE(target.isErr());
    EXPECT_FALSE(target.isOk());
}

TEST(ResultTest, constructOkVoid) {
    const Result<void, std::string> target = Ok();
    EXPECT_TRUE(target.isOk());
    EXPECT_FALSE(target.isErr());
}

TEST(ResultTest, voidErr) {
    const Result<void, std::string> target = Err<std::string>("error");
    EXPECT_FALSE(target.isOk());
    EXPECT_TRUE(target.isErr());
}

TEST(ResultTest, eqSameOks) {
    EXPECT_TRUE(Ok(1) == Ok(1));
}

TEST(ResultTest, eqSameOksVoid) {
    EXPECT_TRUE(Ok() == Ok());
}

TEST(ResultTest, eqDistinctOks) {
    EXPECT_FALSE(Ok(1) == Ok(2));
}

TEST(ResultTest, neDistinctOks) {
    EXPECT_TRUE(Ok(1) != Ok(2));
}

TEST(ResultTest, neSameOks) {
    EXPECT_FALSE(Ok(1) != Ok(1));
}

TEST(ResultTest, eqErrs) {
    EXPECT_TRUE(Err("error") == Err("error"));
}

TEST(ResultTest, neErrs) {
    EXPECT_FALSE(Err("error") != Err("error"));
}

TEST(ResultTest, eqSameOkResults) {
    const Result<int, std::string> res1 = Ok(1);
    const Result<int, std::string> res2 = Ok(1);
    EXPECT_TRUE(res1 == res2);
}

TEST(ResultTest, eqSameOkResultsVoid) {
    const Result<void, std::string> res1 = Ok();
    const Result<void, std::string> res2 = Ok();
    EXPECT_TRUE(res1 == res2);
}

TEST(ResultTest, eqDistinctOkResults) {
    const Result<int, std::string> res1 = Ok(1);
    const Result<int, std::string> res2 = Ok(2);
    EXPECT_FALSE(res1 == res2);
}

TEST(ResultTest, neDistinctOkResults) {
    const Result<int, std::string> res1 = Ok(1);
    const Result<int, std::string> res2 = Ok(2);
    EXPECT_TRUE(res1 != res2);
}

TEST(ResultTest, neSameOkResults) {
    const Result<int, std::string> res1 = Ok(1);
    const Result<int, std::string> res2 = Ok(1);
    EXPECT_FALSE(res1 != res2);
}

TEST(ResultTest, eqErrResults) {
    const Result<int, std::string> res1 = Err<std::string>("error");
    const Result<int, std::string> res2 = Err<std::string>("error");
    EXPECT_TRUE(res1 == res2);
}

TEST(ResultTest, eqErrResultsVoid) {
    const Result<void, std::string> res1 = Err<std::string>("error");
    const Result<void, std::string> res2 = Err<std::string>("error");
    EXPECT_TRUE(res1 == res2);
}

TEST(ResultTest, neErrResults) {
    const Result<int, std::string> res1 = Err<std::string>("error");
    const Result<int, std::string> res2 = Err<std::string>("error");
    EXPECT_FALSE(res1 != res2);
}

TEST(ResultTest, neErrResultsVoid) {
    const Result<void, std::string> res1 = Err<std::string>("error");
    const Result<void, std::string> res2 = Err<std::string>("error");
    EXPECT_FALSE(res1 != res2);
}

TEST(ResultTest, eqOkAndErrResults) {
    const Result<int, std::string> res1 = Ok(1);
    const Result<int, std::string> res2 = Err<std::string>("error");
    EXPECT_FALSE(res1 == res2);
    EXPECT_FALSE(res2 == res1);
}

TEST(ResultTest, eqOkAndErrResultsVoid) {
    const Result<void, std::string> res1 = Ok();
    const Result<void, std::string> res2 = Err<std::string>("error");
    EXPECT_FALSE(res1 == res2);
    EXPECT_FALSE(res2 == res1);
}

TEST(ResultTest, neOkAndErrResults) {
    const Result<int, std::string> res1 = Ok(1);
    const Result<int, std::string> res2 = Err<std::string>("error");
    EXPECT_TRUE(res1 != res2);
    EXPECT_TRUE(res2 != res1);
}

TEST(ResultTest, neOkAndErrResultsVoid) {
    const Result<void, std::string> res1 = Ok();
    const Result<void, std::string> res2 = Err<std::string>("error");
    EXPECT_TRUE(res1 != res2);
    EXPECT_TRUE(res2 != res1);
}

TEST(ResultTest, unwrapOnOk) {
    const Result<int, std::string> target = Ok(1);
    EXPECT_EQ(target.unwrap(), 1);
}

TEST(ResultTest, unwrapOnOkVoid) {
    const Result<void, std::string> target = Ok();
    EXPECT_NO_THROW(target.unwrap());
}

TEST(ResultTest, unwrapOnErr) {
    const Result<int, std::string> target = Err<std::string>("error");
    EXPECT_THROW(target.unwrap(), std::runtime_error);
}

TEST(ResultTest, unwrapOnErrVoid) {
    const Result<void, std::string> target = Err<std::string>("error");
    EXPECT_THROW(target.unwrap(), std::runtime_error);
}

TEST(ResultTest, unwrapOrOnOk) {
    const Result<std::string, int> target = Ok<std::string>("ok");
    EXPECT_EQ(target.unwrapOr("error"), "ok");
}

TEST(ResultTest, unwrapOrOnErr) {
    const Result<std::string, int> target = Err(1);
    EXPECT_EQ(target.unwrapOr("error"), "error");
}

TEST(ResultTest, unwrapErrOnErr) {
    const Result<int, std::string> target = Err<std::string>("error");
    EXPECT_EQ(target.unwrapErr(), "error");
}

TEST(ResultTest, unwrapErrOnErrVoid) {
    const Result<void, std::string> target = Err<std::string>("error");
    EXPECT_EQ(target.unwrapErr(), "error");
}

TEST(ResultTest, unwrapErrOnOk) {
    const Result<int, std::string> target = Ok(1);
    EXPECT_THROW(target.unwrapErr(), std::runtime_error);
}

TEST(ResultTest, unwrapErrOnOkVoid) {
    const Result<void, std::string> target = Ok();
    EXPECT_THROW(target.unwrapErr(), std::runtime_error);
}

Result<int, std::string> addOneIf(const Result<int, std::string> &res) {
    const int value = TRY(res);
    return Ok(value + 1);
}

TEST(ResultTest, tryOk) {
    const Result<int, std::string> expect = Ok(2);
    EXPECT_EQ(expect, addOneIf(Ok(1)));
}

TEST(ResultTest, tryErr) {
    const Result<int, std::string> expect = Err<std::string>("error");
    EXPECT_EQ(expect, addOneIf(Err<std::string>("error")));
}

int doubleIf(const Result<int, std::string> &res, const int defaultValue) {
    const int value = TRY_OR(res, defaultValue);
    return value * 2;
}

TEST(ResultTest, tryOrOk) {
    const Result<int, std::string> target = Ok(1);
    EXPECT_EQ(doubleIf(target, 0), 2);
}

TEST(ResultTest, tryOrErr) {
    const Result<int, std::string> target = Err<std::string>("error");
    EXPECT_EQ(doubleIf(target, 0), 0);
}

Result<void, int> doNothingIf(const Result<void, int> &res) {
    TRY(res);
    return Ok();
}

TEST(ResultTest, tryVoidOk) {
    EXPECT_TRUE(doNothingIf(Ok()).isOk());
}

TEST(ResultTest, tryVoidErr) {
    EXPECT_TRUE(doNothingIf(Err(1)).isErr());
}
