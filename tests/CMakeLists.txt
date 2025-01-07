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

# テスト用の utility
add_library(test_utils STATIC
        utils/reader.cpp
        utils/reader.hpp
)
target_link_libraries(test_utils PUBLIC webserv_lib)

# テストの実行ファイルにリンクするライブラリ
# NOTE: test_utils に webserv_lib が入っているので、改めて含めてない
link_libraries(gtest_main FakeIt::FakeIt-gtest test_utils)

# Tests
add_executable(option_test option_test.cpp)
gtest_discover_tests(option_test)

add_executable(result_test result_test.cpp)
gtest_discover_tests(result_test)

add_executable(http_method_test http_method_test.cpp)
gtest_discover_tests(http_method_test)

add_executable(http_status_test http_status_test.cpp)
gtest_discover_tests(http_status_test)

add_executable(string_utils_test string_utils_test.cpp)
gtest_discover_tests(string_utils_test)

add_executable(request_parser_test request_parser_test.cpp)
gtest_discover_tests(request_parser_test)

add_executable(request_reader_test request_reader_test.cpp)
gtest_discover_tests(request_reader_test)

add_executable(response_test response_test.cpp)
gtest_discover_tests(response_test)

add_executable(ref_test ref_test.cpp)
gtest_discover_tests(ref_test)

add_executable(read_buffer_test read_buffer_test.cpp)
gtest_discover_tests(read_buffer_test)

add_executable(matcher_test matcher_test.cpp)
gtest_discover_tests(matcher_test)

add_executable(virtual_server_resolver_test virtual_server_resolver_test.cpp)
gtest_discover_tests(virtual_server_resolver_test)
