#ifndef SRC_LIB_HTTP_MIME_HPP
#define SRC_LIB_HTTP_MIME_HPP

#include <string>

namespace http {
    // 本来は設定ファイルを元に解決するべき
    std::string getMimeType(const std::string &fileName);
}

#endif
