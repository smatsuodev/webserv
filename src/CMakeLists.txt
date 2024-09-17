set(CMAKE_CXX_STANDARD 98)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

add_library(webserv_lib STATIC
        lib/add.cpp
        lib/add.hpp
)
target_include_directories(webserv_lib PUBLIC lib)

add_executable(webserv cmd/main.cpp)
target_link_libraries(webserv PRIVATE webserv_lib)