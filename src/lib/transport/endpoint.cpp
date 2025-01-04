#include "endpoint.hpp"
#include "utils/string.hpp"

Endpoint::Endpoint(const std::string &ip, unsigned short port) : ip_(ip), port_(port) {}

const std::string &Endpoint::getIp() const {
    return ip_;
}

unsigned short Endpoint::getPort() const {
    return port_;
}

std::string Endpoint::toString() const {
    return utils::format("%s:%u", ip_.c_str(), port_);
}
