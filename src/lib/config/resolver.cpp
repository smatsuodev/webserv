#include "resolver.hpp"

namespace config {
    Resolver::Resolver(const Config &config) : config_(config) {}

    Option<ServerContext> Resolver::resolve(const Address &address, const std::string &host) const {
        // 先に address で絞り込む
        ServerContextList servers = config_.getServers();
        ServerContextList candidates;
        ServerContextList wildcards;
        for (ServerContextList::const_iterator it = servers.begin(); it != servers.end(); ++it) {
            // TODO: host を ip に解決しておかないと、正しく比較できない
            if (it->getPort() == address.getPort()) {
                if (it->getHost() == "0.0.0.0") {
                    wildcards.push_back(*it);
                } else if (it->getHost() == address.getIp()) {
                    candidates.push_back(*it);
                }
            }
        }

        // NOTE: 具体的なアドレスがあるときは、0.0.0.0 にはマッチしない
        if (candidates.empty()) {
            candidates = wildcards;
        }

        // Host ヘッダーで絞り込む
        // nginx は server_name が同じものが複数ある場合、最初にマッチしたものを選択する
        for (ServerContextList::const_iterator it = candidates.begin(); it != candidates.end(); ++it) {
            const std::vector<std::string> &serverName = it->getServerName();
            const std::vector<std::string>::const_iterator found =
                std::find(serverName.begin(), serverName.end(), host);
            if (found != serverName.end()) {
                return Some(*it);
            }
        }

        // Host で一意に決まらなかった場合、最初のものが default として選択される
        if (!candidates.empty()) {
            return Some(candidates.front());
        }

        return None;
    }
}
