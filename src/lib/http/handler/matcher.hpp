#ifndef SRC_LIB_HTTP_HANDLER_MATCHER_HPP
#define SRC_LIB_HTTP_HANDLER_MATCHER_HPP

#include "utils/types/option.hpp"
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
            if (key.empty() || key.front() != '/') {
                return None;
            }
            std::string bestKey;
            for (const std::pair<std::string, T> &entry : map_) {
                const std::string &candidate = entry.first;
                if (key.size() >= candidate.size() && key.compare(0, candidate.size(), candidate) == 0 &&
                    candidate.size() > bestKey.size()) {
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
