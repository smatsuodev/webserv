cmake_minimum_required(VERSION 3.11)
project(webserv)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-fno-omit-frame-pointer -fsanitize=address,undefined)
    add_link_options(-fsanitize=address,undefined)
endif ()

include(CTest)

add_subdirectory(src)
add_subdirectory(tests)
