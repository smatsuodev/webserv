#ifndef SRC_LIB_CGI_REQUEST_HPP
#define SRC_LIB_CGI_REQUEST_HPP

#include "meta_variable.hpp"
#include "utils/types/error.hpp"
#include "utils/types/option.hpp"
#include <vector>

namespace cgi {
    class Request {
    public:
        static Result<Request, error::AppError>
        create(const std::vector<MetaVariable> &variables, const Option<std::string> &body = None);

        const std::vector<MetaVariable> &getVariables() const;
        const Option<std::string> &getBody() const;

    private:
        std::vector<MetaVariable> variables_;
        Option<std::string> body_;

        explicit Request(const std::vector<MetaVariable> &variables, const Option<std::string> &body = None);
    };
}

#endif
