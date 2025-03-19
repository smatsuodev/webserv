#include "utils/types/either.hpp"


#include <gtest/gtest.h>

TEST(EitherTest, constructLeftInt) {
    const Either<int, std::string> target = Left(42);
    EXPECT_TRUE(target.isLeft());
    EXPECT_FALSE(target.isRight());
}

TEST(EitherTest, constructRightStr) {
    const Either<int, std::string> target = Right<std::string>("hello");
    EXPECT_TRUE(target.isRight());
    EXPECT_FALSE(target.isLeft());
}

TEST(EitherTest, eqSameLeft) {
    EXPECT_TRUE(Left(1) == Left(1));
}

TEST(EitherTest, eqDistinctLeft) {
    EXPECT_FALSE(Left(1) == Left(2));
}

TEST(EitherTest, neDistinctLeft) {
    EXPECT_TRUE(Left(1) != Left(2));
}

TEST(EitherTest, neSameLeft) {
    EXPECT_FALSE(Left(1) != Left(1));
}

TEST(EitherTest, eqSameRight) {
    EXPECT_TRUE(Right(1) == Right(1));
}

TEST(EitherTest, eqDistinctRight) {
    EXPECT_FALSE(Right(1) == Right(2));
}

TEST(EitherTest, neDistinctRight) {
    EXPECT_TRUE(Right(1) != Right(2));
}

TEST(EitherTest, neSameRight) {
    EXPECT_FALSE(Right(1) != Right(1));
}

TEST(EitherTest, eqSameLeftEithers) {
    const Either<int, std::string> e1 = Left(1);
    const Either<int, std::string> e2 = Left(1);
    EXPECT_TRUE(e1 == e2);
}

TEST(EitherTest, eqDistinctLeftEithers) {
    const Either<int, std::string> e1 = Left(1);
    const Either<int, std::string> e2 = Left(2);
    EXPECT_FALSE(e1 == e2);
}

TEST(EitherTest, neDistinctLeftEithers) {
    const Either<int, std::string> e1 = Left(1);
    const Either<int, std::string> e2 = Left(2);
    EXPECT_TRUE(e1 != e2);
}

TEST(EitherTest, neSameLeftEithers) {
    const Either<int, std::string> e1 = Left(1);
    const Either<int, std::string> e2 = Left(1);
    EXPECT_FALSE(e1 != e2);
}

TEST(EitherTest, eqSameRightEithers) {
    const Either<int, std::string> e1 = Right<std::string>("a");
    const Either<int, std::string> e2 = Right<std::string>("a");
    EXPECT_TRUE(e1 == e2);
}

TEST(EitherTest, eqDistinctRightEithers) {
    const Either<int, std::string> e1 = Right<std::string>("a");
    const Either<int, std::string> e2 = Right<std::string>("b");
    EXPECT_FALSE(e1 == e2);
}

TEST(EitherTest, neDistinctRightEithers) {
    const Either<int, std::string> e1 = Right<std::string>("a");
    const Either<int, std::string> e2 = Right<std::string>("b");
    EXPECT_TRUE(e1 != e2);
}

TEST(EitherTest, neSameRightEithers) {
    const Either<int, std::string> e1 = Right<std::string>("a");
    const Either<int, std::string> e2 = Right<std::string>("a");
    EXPECT_FALSE(e1 != e2);
}

TEST(EitherTest, eqLeftAndRightEithers) {
    const Either<int, int> e1 = Left(1);
    const Either<int, int> e2 = Right(1);
    EXPECT_FALSE(e1 == e2);
}

TEST(EitherTest, neLeftAndRightEithers) {
    const Either<int, int> e1 = Left(1);
    const Either<int, int> e2 = Right(1);
    EXPECT_TRUE(e1 != e2);
}

TEST(EitherTest, unwrapLeftOnLeft) {
    const Either<int, int> target = Left(1);
    EXPECT_EQ(target.unwrapLeft(), 1);
}

TEST(EitherTest, unwrapLeftOnRight) {
    const Either<int, int> target = Right(1);
    EXPECT_THROW(target.unwrapLeft(), std::runtime_error);
}

TEST(EitherTest, unwrapRightOnRight) {
    const Either<int, int> target = Right(1);
    EXPECT_EQ(target.unwrapRight(), 1);
}

TEST(EitherTest, unwrapRightOnLeft) {
    const Either<int, int> target = Left(1);
    EXPECT_THROW(target.unwrapRight(), std::runtime_error);
}

TEST(EitherTest, unwrapLeftOrOnLeft) {
    const Either<int, int> target = Left(1);
    EXPECT_EQ(target.unwrapLeftOr(2), 1);
}

TEST(EitherTest, unwrapLeftOrOnRight) {
    const Either<int, int> target = Right(1);
    EXPECT_EQ(target.unwrapLeftOr(2), 2);
}

TEST(EitherTest, unwrapRightOrOnRight) {
    const Either<int, int> target = Right(1);
    EXPECT_EQ(target.unwrapRightOr(2), 1);
}

TEST(EitherTest, unwrapRightOrOnLeft) {
    const Either<int, int> target = Left(1);
    EXPECT_EQ(target.unwrapRightOr(2), 2);
}

Either<int, std::string> left42() {
    return Left(42);
}

Either<int, std::string> right42() {
    return Right<std::string>("42");
}

TEST(EitherTest, returnLeft) {
    EXPECT_EQ(left42(), Left(42));
}

TEST(EitherTest, returnRight) {
    EXPECT_EQ(right42(), Right<std::string>("42"));
}
