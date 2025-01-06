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
            Option<std::string> bestKey = None;
            for (typename Map::const_iterator it = map_.begin(); it != map_.end(); ++it) {
                const std::string &candidate = it->first;
                if (utils::startsWith(key, candidate) && (bestKey.isNone() || candidate.size() > bestKey.unwrap().size())) {
                    bestKey = Some(candidate);
                }
            }
            if (bestKey.isNone()) {
                return None;
            }
            return Some(map_.at(bestKey.unwrap()));
        }
    };


}

#endif
