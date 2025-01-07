#include "virtual_server_resolver.hpp"
#include "utils/logger.hpp"
#include <algorithm>

VirtualServerResolver::VirtualServerResolver(const VirtualServerList &virtualServers, const Ref<const Connection> &conn)
    : virtualServers_(virtualServers), conn_(conn) {}

Option<Ref<VirtualServer> > VirtualServerResolver::resolve(const std::string &hostHeader) const {
    // Host は host(:port) で指定される。port は無視する。
    const std::string host = hostHeader.substr(0, hostHeader.find(':'));

    VirtualServerList candidates;
    VirtualServerList wildcards;
    // address で絞り込み
    for (VirtualServerList::const_iterator it = virtualServers_.begin(); it != virtualServers_.end(); ++it) {
        if (!(*it)->isMatch(conn_.get().getLocalAddress())) {
            continue;
        }

        // TODO: getHost() で得られるものは IP アドレスじゃないかもしれない
        if ((*it)->getServerConfig().getHost() == "0.0.0.0") {
            wildcards.push_back(*it);
        } else {
            candidates.push_back(*it);
        }
    }

    // NOTE: 具体的なアドレスがあるときは、0.0.0.0 にはマッチしない
    if (candidates.empty()) {
        candidates = wildcards;
    }

    // Host ヘッダーで絞り込み
    for (VirtualServerList::const_iterator it = candidates.begin(); it != candidates.end(); ++it) {
        const std::vector<std::string> &serverName = (*it)->getServerConfig().getServerName();
        // NOTE: 引数の hostHeader ではなく、port を取り除いたものを使ってる
        const std::vector<std::string>::const_iterator found = std::find(serverName.begin(), serverName.end(), host);
        if (found != serverName.end()) {
            return Some(utils::ref(**it));
        }
    }

    // Host ヘッダーで決まらない場合、candidates の最初のものが使われる
    if (!candidates.empty()) {
        return Some(utils::ref(*candidates.front()));
    }

    return None;
}

VirtualServerResolverFactory::VirtualServerResolverFactory(const VirtualServerList &virtualServers)
    : virtualServers_(virtualServers) {}

VirtualServerResolver VirtualServerResolverFactory::create(const Ref<Connection> &conn) {
    return VirtualServerResolver(virtualServers_, conn);
}
