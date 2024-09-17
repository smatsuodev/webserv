set(CMAKE_CXX_STANDARD 98)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

add_library(libwebserv STATIC
        lib/add.cpp
        lib/add.h
)
target_include_directories(libwebserv PUBLIC lib)

add_executable(webserv cmd/main.cpp)
target_link_libraries(webserv PRIVATE libwebserv)