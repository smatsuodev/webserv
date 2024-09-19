set(CMAKE_CXX_STANDARD 20)
# gmock は使わないのでビルドしない
set(BUILD_GMOCK OFF)

include(FetchContent)

# Google Test
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG f8d7d77c06936315286eb55f8de22cd23c188571 # v1.14.0
)
FetchContent_MakeAvailable(googletest)
include(GoogleTest)

# FakeIt
FetchContent_Declare(
        fakeit
        GIT_REPOSITORY https://github.com/eranpeer/FakeIt.git
        GIT_TAG cb39d8a053876f74dfeed66dd335d3041f142095 # 2.4.0
)
FetchContent_MakeAvailable(fakeit)

# テストの実行ファイルにリンクするライブラリ
link_libraries(webserv_lib gtest_main FakeIt::FakeIt-gtest)

# Tests
add_executable(add_test add_test.cpp)
gtest_discover_tests(add_test)
