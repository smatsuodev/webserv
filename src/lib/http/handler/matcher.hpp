#ifndef SRC_LIB_HTTP_HANDLER_MATCHER_HPP
#define SRC_LIB_HTTP_HANDLER_MATCHER_HPP

#include "utils/types/option.hpp"
#include "utils/string.hpp"
#include <map>
#include <iterator>

namespace http {
    template <typename T>
    class Matcher {
    private:
        typedef std::map<std::string, T> Map;
        Map map_;

    public:
        explicit Matcher(const Map &map) : map_(map) {}
        ~Matcher() {}

        Option<T> match(const std::string &key) const {
            if (key.empty()) {
                return None;
            }
            std::string bestKey;
            for (typename Map::const_iterator it = map_.begin(); it != map_.end(); ++it) {
                const std::string &candidate = it->first;
                if (utils::startsWith(key, candidate) && candidate.size() > bestKey.size()) {
                    bestKey = candidate;
                }
            }
            if (bestKey.empty()) {
                return None;
            }
            return Some(map_.at(bestKey));
        }
    };


}

#endif
