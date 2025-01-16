#include "meta_variable.hpp"

namespace cgi {
    MetaVariable::MetaVariable(const std::string &name, const std::string &value) : name_(name), value_(value) {}

    const std::string &MetaVariable::getName() const {
        return name_;
    }

    const std::string &MetaVariable::getValue() const {
        return value_;
    }
}
