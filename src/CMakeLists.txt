set(CMAKE_CXX_STANDARD 11)  # TODO: C++98 にする
# 実行ファイルを /bin に出力
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# ここに lib/ 以下のファイルを追加する
# 実行ファイルとテストで再利用するために、静的ライブラリを作成
add_library(webserv_lib STATIC
        lib/core/server.cpp
        lib/core/server.hpp
        lib/utils/non_copyable.hpp
        lib/utils/auto_fd.cpp
        lib/utils/auto_fd.hpp
        lib/transport/listener.cpp
        lib/transport/listener.hpp
        lib/transport/connection.cpp
        lib/transport/connection.hpp
        lib/utils/logger.cpp
        lib/utils/logger.hpp
        lib/utils/string.cpp
        lib/utils/string.hpp
        lib/utils/types/option.hpp
        lib/utils/types/result.hpp
        lib/utils/types/unit.hpp
        lib/utils/types/try.hpp
        lib/utils/io/reader.cpp
        lib/utils/io/reader.hpp
        lib/utils/types/result.cpp
        lib/utils/types/error.cpp
        lib/utils/types/error.hpp
        lib/event/event_notifier.cpp
        lib/event/event_notifier.hpp
        lib/event/event.cpp
        lib/event/event.hpp
        lib/utils/fd.cpp
        lib/utils/fd.hpp
        lib/event/event_handler.cpp
        lib/event/event_handler.hpp
        lib/core/handler/accept_handler.cpp
        lib/core/handler/accept_handler.hpp
        lib/core/action.cpp
        lib/core/action.hpp
        lib/http/method.cpp
        lib/http/method.hpp
        lib/http/status.cpp
        lib/http/status.hpp
        lib/http/request/request.cpp
        lib/http/request/request.hpp
        lib/http/request/request_parser.cpp
        lib/http/request/request_parser.hpp
        lib/core/handler/read_request_handler.cpp
        lib/core/handler/read_request_handler.hpp
        lib/http/request/request_reader.cpp
        lib/http/request/request_reader.hpp
        lib/core/server_state.cpp
        lib/core/server_state.hpp
        lib/http/header.hpp
        lib/http/response/response.cpp
        lib/http/response/response.hpp
        lib/core/handler/write_response_handler.cpp
        lib/core/handler/write_response_handler.hpp
        lib/http/response/response_builder.cpp
        lib/http/response/response_builder.hpp
        lib/utils/ref.hpp
        lib/http/handler/handler.cpp
        lib/http/handler/handler.hpp
        lib/utils/io/read_buffer.cpp
        lib/utils/io/read_buffer.hpp
        lib/config/config.cpp
        lib/config/config.hpp
        lib/config/toml/value.cpp
        lib/config/toml/value.hpp
        lib/http/handler/matcher.hpp
)
# lib/ を include path に追加
# PUBLIC にすると、依存するターゲットにも反映される
target_include_directories(webserv_lib PUBLIC lib)

# webserv という実行ファイルを作成
add_executable(webserv cmd/main.cpp)
target_link_libraries(webserv PRIVATE webserv_lib)