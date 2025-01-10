#include "listener.hpp"

#include "utils/auto_deleter.hpp"
#include "utils/fd.hpp"
#include <string>
#include <stdexcept>
#include <arpa/inet.h>
#include "utils/logger.hpp"
#include "utils/string.hpp"
#include <cerrno>
#include <cstring>
#include <netdb.h>
#include <unistd.h>

// NOTE: Address のデフォルトコンストラクタがなく、適切な初期化が難しいので、ダミーの値を設定している
Listener::Listener(const std::string &host, const std::string &port, const int backlog) : bindAddress_("", 0) {
    this->setupSocket(host, port, backlog);
}

Listener::~Listener() {
    LOG_DEBUG("Listener: destruct");
}

int Listener::getFd() const {
    return serverFd_.get();
}

const Address &Listener::getBindAddress() const {
    return bindAddress_;
}

Listener::AcceptConnectionResult Listener::acceptConnection() const {
    sockaddr_in sockAddr = {};
    socklen_t sockAddrLen = sizeof(sockAddr);
    AutoFd fd(accept(serverFd_, reinterpret_cast<sockaddr *>(&sockAddr), &sockAddrLen));
    if (fd == -1) {
        LOG_WARNF("failed to accept connection: %s", std::strerror(errno));
        return Err<std::string>("failed to accept connection");
    }
    const Address foreignAddress(inet_ntoa(sockAddr.sin_addr), ntohs(sockAddr.sin_port));

    const Result<void, error::AppError> result = utils::setNonBlocking(fd);
    if (result.isErr()) {
        LOG_ERROR("failed to set non-blocking fd");
        return Err<std::string>("failed to set non-blocking fd");
    }

    LOG_INFOF("connection established from %s (fd: %d)", foreignAddress.toString().c_str(), fd.get());

    // local address を取得
    sockaddr_in localAddr = {};
    socklen_t localAddrLen = sizeof(localAddr);
    if (getsockname(fd, reinterpret_cast<sockaddr *>(&localAddr), &localAddrLen) == -1) {
        LOG_ERRORF("failed to get local address: %s", std::strerror(errno));
        return Err<std::string>("failed to get local address");
    }
    const Address localAddress(inet_ntoa(localAddr.sin_addr), ntohs(localAddr.sin_port));

    return Ok(new Connection(fd.release(), localAddress, foreignAddress));
}

Result<addrinfo *, std::string> resolveAddress(const std::string &host, const std::string &port) {
    addrinfo hints = {};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    addrinfo *result;
    const int status = getaddrinfo(host.c_str(), port.c_str(), &hints, &result);
    if (status != 0) {
        return Err(utils::format("failed to resolve address: %s", gai_strerror(status)));
    }

    return Ok(result);
}

typedef std::pair<int, Address> FdAddressPair;

// fd と bind に成功したアドレスを返す
Result<FdAddressPair, std::string> tryCreateAndBindSocket(addrinfo *addrList) {
    // free 漏れを防ぐため、AutoDeleter で wrap
    const AutoDeleter<addrinfo> list(addrList, freeaddrinfo);

    for (const addrinfo *listNode = list; listNode != NULL; listNode = listNode->ai_next) {
        AutoFd sFd(socket(listNode->ai_family, listNode->ai_socktype, listNode->ai_protocol));
        if (sFd == -1) {
            // hint で family, socktype, protocol を指定しているので、致命的なエラー
            return Err(utils::format("failed to create socket: %s", std::strerror(errno)));
        }

        const int opt = 1;
        if (setsockopt(sFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1 ||
            bind(sFd, listNode->ai_addr, listNode->ai_addrlen) == -1) {
            // 他の候補を試す
            continue;
        }

        // socket(), bind() に成功
        // socket が close されないように、release で所有権を渡す
        sockaddr_in *addr = reinterpret_cast<sockaddr_in *>(listNode->ai_addr);
        return Ok(std::make_pair(sFd.release(), Address(inet_ntoa(addr->sin_addr), ntohs(addr->sin_port))));
    }

    // すべての候補で bind に失敗
    return Err(utils::format("failed to create and bind socket: %s", std::strerror(errno)));
}

// andThen で繋げるために lint を無効化
// ReSharper disable once CppPassValueParameterByConstReference
Result<FdAddressPair, std::string> setNonBlocking(const FdAddressPair fdAddrPair) { // NOLINT(*-unnecessary-value-param)
    // close 漏れを防ぐため、AutoFd で wrap
    AutoFd fd(fdAddrPair.first);

    const Result<void, error::AppError> result = utils::setNonBlocking(fd);
    if (result.isErr()) {
        return Err(utils::format("failed to set non blocking mode: %s", std::strerror(errno)));
    }

    // 成功したら fd が close されないように、所有権を放棄
    fd.release();
    return Ok(fdAddrPair);
}

void Listener::setupSocket(const std::string &host, const std::string &port, const int backlog) {
    const Result<FdAddressPair, std::string> result =
        resolveAddress(host, port).andThen(tryCreateAndBindSocket).andThen(setNonBlocking);
    if (result.isErr()) {
        const std::string &err = result.unwrapErr();
        LOG_ERRORF("failed to setup socket: %s", err.c_str());
        throw std::runtime_error("failed to setup socket");
    }

    serverFd_.reset(result.unwrap().first);
    bindAddress_ = result.unwrap().second;
    LOG_DEBUGF("server socket created (fd: %d)", serverFd_.get());

    if (listen(serverFd_, backlog) == -1) {
        LOG_ERRORF("failed to listen on socket: %s", std::strerror(errno));
        throw std::runtime_error("failed to listen on socket");
    }

    LOG_DEBUGF("listening on %s (fd: %d)", bindAddress_.toString().c_str(), serverFd_.get());
}
