#ifndef SRC_LIB_HTTP_HANDLER_MATCHER_HPP
#define SRC_LIB_HTTP_HANDLER_MATCHER_HPP

#include "utils/types/option.hpp"
#include <map>
#include <iterator>

namespace http {
    template <typename T>
    class Matcher {
    public:
        typedef std::map<std::string, T> Map;
        explicit Matcher(const Map &map) : map_(map) {}
        ~Matcher() {}

        Option<T> match(const std::string &key) {
            if (key.size() > 1 && key[key.size() - 1] == '/') {
                return match(key.substr(0, key.size() - 1));
            }
            if (key.find("//") != std::string::npos) {
                return match(key.substr(0, key.find("//") + 1) + key.substr(key.find("//") + 2));
            }
            typename Map::const_iterator it = map_.find(key);
            if (it == map_.end()) {
                if (key.find_last_of('/') != std::string::npos) {
                    if (key.substr(0, key.find_last_of('/')).empty()) { // /root -> / になるのを防ぎたい
                        return match("/");
                    }
                    return match(key.substr(0, key.find_last_of('/')));
                }
                return None;
            }
            return Some(it->second);
        }

    private:
        Map map_;
    };


}

#endif
