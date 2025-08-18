cmake_minimum_required(VERSION 3.11)
project(webserv)

# 指定したバージョンが使用できなければエラー
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-fno-omit-frame-pointer -fsanitize=address,undefined)
    add_link_options(-fsanitize=address,undefined)
endif ()

# compile_commands.json を生成
# clang-tidy などのツールで使う
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# CTest (テストランナー) を有効化
include(CTest)

# src, tests の設定を読み込む
add_subdirectory(src)
add_subdirectory(tests)
