#ifndef SRC_LIB_HTTP_HANDLER_MATCHER_HPP
#define SRC_LIB_HTTP_HANDLER_MATCHER_HPP
#include "utils/types/option.hpp"


#include <map>


namespace http {
    template <typename T>
    class Matcher {
    public:
        typedef std::map<std::string, T> Map;
        explicit Matcher(const Map &map) : map_(map) {}
        ~Matcher() {}

        Option<T> match(const std::string &key) const {

        }

    private:
        Map map_;
    };


}

#endif
