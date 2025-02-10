#ifndef SRC_LIB_CGI_META_VARIABLE_HPP
#define SRC_LIB_CGI_META_VARIABLE_HPP
#include <string>

namespace cgi {
    class MetaVariable {
    public:
        MetaVariable(const std::string &name, const std::string &value);

        const std::string &getName() const;
        const std::string &getValue() const;

    private:
        std::string name_;
        std::string value_;
    };
}

#endif
