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
        lib/core/action/action.cpp
        lib/core/action/action.hpp
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
        lib/core/server_state.cpp
        lib/core/server_state.hpp
        lib/http/header.hpp
        lib/http/response/response.cpp
        lib/http/response/response.hpp
        lib/core/handler/write_response_body_handler.cpp
        lib/core/handler/write_response_body_handler.hpp
        lib/http/response/response_builder.cpp
        lib/http/response/response_builder.hpp
        lib/utils/ref.hpp
        lib/http/handler/handler.hpp
        lib/utils/io/read_buffer.cpp
        lib/utils/io/read_buffer.hpp
        lib/config/config.cpp
        lib/config/config.hpp
        lib/config/toml/value.cpp
        lib/config/toml/value.hpp
        lib/config/toml/parser.cpp
        lib/config/toml/parser.hpp
        lib/config/toml/tokenizer.cpp
        lib/config/toml/tokenizer.hpp
        lib/http/handler/redirect_handler.cpp
        lib/http/handler/redirect_handler.hpp
        lib/http/handler/router.cpp
        lib/http/handler/router.hpp
        lib/http/handler/matcher.hpp
        lib/transport/address.cpp
        lib/transport/address.hpp
        lib/core/virtual_server_resolver.cpp
        lib/core/virtual_server_resolver.hpp
        lib/core/virtual_server.cpp
        lib/core/virtual_server.hpp
        lib/http/handler/static_file_handler.cpp
        lib/http/handler/static_file_handler.hpp
        lib/http/handler/delete_file_handler.cpp
        lib/http/handler/delete_file_handler.hpp
        lib/utils/auto_deleter.hpp
        lib/http/request/reader/request_reader.cpp
        lib/http/request/reader/request_reader.hpp
        lib/http/request/reader/reading_request_line_state.cpp
        lib/http/request/reader/reading_request_line_state.hpp
        lib/http/request/reader/utils.cpp
        lib/http/request/reader/utils.hpp
        lib/http/request/reader/reading_headers_state.cpp
        lib/http/request/reader/reading_headers_state.hpp
        lib/http/request/reader/reading_body_state.cpp
        lib/http/request/reader/reading_body_state.hpp
        lib/http/request/reader/reading_chunked_body_state.cpp
        lib/http/request/reader/reading_chunked_body_state.hpp
        lib/http/mime.cpp
        lib/http/mime.hpp
        lib/http/handler/middleware/middleware.hpp
        lib/http/handler/middleware/logger.cpp
        lib/http/handler/middleware/logger.hpp
        lib/http/handler/middleware/error_page.cpp
        lib/http/handler/middleware/error_page.hpp
        lib/cgi/meta_variable.cpp
        lib/cgi/meta_variable.hpp
        lib/cgi/request.cpp
        lib/cgi/request.hpp
        lib/cgi/factory.cpp
        lib/cgi/factory.hpp
        lib/cgi/response.cpp
        lib/cgi/response.hpp
        lib/utils/types/either.hpp
        lib/core/action/serve_http_action.cpp
        lib/http/handler/cgi_handler.cpp
        lib/http/handler/cgi_handler.hpp
        lib/core/handler/write_cgi_request_handler.cpp
        lib/core/handler/write_cgi_request_handler.hpp
        lib/core/handler/read_cgi_response_handler.cpp
        lib/core/handler/read_cgi_response_handler.hpp
        lib/core/action/run_cgi_action.cpp
)
# lib/ を include path に追加
# PUBLIC にすると、依存するターゲットにも反映される
target_include_directories(webserv_lib PUBLIC lib)

# webserv という実行ファイルを作成
add_executable(webserv cmd/main.cpp)
target_link_libraries(webserv PRIVATE webserv_lib)