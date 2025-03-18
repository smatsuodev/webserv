#ifndef SRC_LIB_HTTP_HANDLER_MATCHER_HPP
#define SRC_LIB_HTTP_HANDLER_MATCHER_HPP

#include "utils/types/option.hpp"
#include "utils/string.hpp"
#include <map>

namespace http {
    template <typename T>
    class Matcher {
    private:
        typedef std::map<std::string, T> Map;
        Map map_;

        typedef bool (*Compare)(const std::string &a, const std::string &b);
        const Compare customComp_;

        bool compare(const std::string &a, const std::string &b) const {
            if (customComp_ != NULL) return customComp_(a, b);
            return utils::startsWith(a, b);
        }

    public:
        explicit Matcher(const Map &map) : map_(map), customComp_(NULL) {}
        explicit Matcher(const Map &map, const Compare comp) : map_(map), customComp_(comp) {}
        ~Matcher() {}

        Option<T> match(const std::string &key) const {
            if (key.empty()) {
                return None;
            }
            Option<std::string> bestKey = None;
            for (typename Map::const_iterator it = map_.begin(); it != map_.end(); ++it) {
                const std::string &candidate = it->first;
                if (compare(key, candidate) && (bestKey.isNone() || candidate.size() > bestKey.unwrap().size())) {
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
