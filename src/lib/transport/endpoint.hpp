#ifndef SRC_LIB_TRANSPORT_ENDPOINT_HPP
#define SRC_LIB_TRANSPORT_ENDPOINT_HPP

#include <string>

class Endpoint {
public:
    Endpoint(const std::string &ip, unsigned short port);

    const std::string &getIp() const;
    unsigned short getPort() const;

    std::string toString() const;

private:
    std::string ip_;
    unsigned short port_{};
};

#endif
