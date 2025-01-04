#include "address.hpp"
#include "utils/string.hpp"

Address::Address(const std::string &ip, const unsigned short port) : ip_(ip), port_(port) {}

const std::string &Address::getIp() const {
    return ip_;
}

unsigned short Address::getPort() const {
    return port_;
}

std::string Address::toString() const {
    return utils::format("%s:%u", ip_.c_str(), port_);
}
