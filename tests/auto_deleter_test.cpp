#include "utils/auto_deleter.hpp"
#include <gtest/gtest.h>

// delete の代わりに値を変更
void deleter(int *ptr) {
    *ptr -= 1;
}

TEST(AutoDeleterTest, op) {
    int value = 42;
    const AutoDeleter autoDel(&value);

    // そのまま dereference できる
    const int dereferenced = *autoDel;
    EXPECT_EQ(dereferenced, value);

    // ポインタへの暗黙の型変換
    const int *ptr = autoDel;
    EXPECT_EQ(ptr, &value);

    EXPECT_EQ(value, 42);
}

TEST(AutoDeleterTest, get) {
    int value = 42;
    {
        const AutoDeleter autoDel(&value, deleter);
        EXPECT_EQ(autoDel.get(), &value);
        EXPECT_EQ(*autoDel, value);
    } // ここで destruct

    // deleter が呼ばれているはず
    EXPECT_EQ(value, 41);
}

TEST(AutoDeleterTest, getNull) {
    {
        const AutoDeleter<int> autoDel(nullptr, deleter);
        EXPECT_EQ(autoDel.get(), nullptr);
    }
}

TEST(AutoDeleterTest, release) {
    int value = 42;
    {
        AutoDeleter autoDel(&value, deleter);
        const int *ptr = autoDel.release();
        EXPECT_EQ(ptr, &value);
        EXPECT_EQ(*ptr, 42);
        EXPECT_EQ(autoDel.get(), nullptr);
    }

    // deleter は呼ばれない
    EXPECT_EQ(value, 42);
}

TEST(AutoDeleterTest, releaseNull) {
    {
        AutoDeleter<int> autoDel(nullptr, deleter);
        const int *ptr = autoDel.release();
        EXPECT_EQ(ptr, nullptr);
    }
}

TEST(AutoDeleterTest, reset) {
    int value = 42;
    {
        AutoDeleter autoDel(&value, deleter);
        autoDel.reset(); // deleter が呼ばれる
        EXPECT_EQ(autoDel.get(), nullptr);
    } // ここでは deleter は呼ばれない

    EXPECT_EQ(value, 41);
}

TEST(AutoDeleterTest, resetToAnother) {
    int value = 42;
    int other = 24;
    {
        AutoDeleter autoDel(&value, deleter);
        autoDel.reset(&other); // deleter が呼ばれる
        EXPECT_EQ(autoDel.get(), &other);
        EXPECT_EQ(*autoDel, other);
    } // deleter が呼ばれる

    EXPECT_EQ(value, 41);
    EXPECT_EQ(other, 23);
}
