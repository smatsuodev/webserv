#include "http/response/response.hpp"
#include <gtest/gtest.h>

TEST(Response, toString) {
    const http::Response response(
        http::kStatusOk,
        http::Headers({std::make_pair("Content-Length", "5"), std::make_pair("Content-Type", "text/plain")}),
        Some("hello")
    );
    const auto expected = "HTTP/1.1 200 OK\r\n"
                          "Content-Length: 5\r\n"
                          "Content-Type: text/plain\r\n"
                          "\r\n"
                          "hello";
    EXPECT_EQ(response.toString(), expected);
}
