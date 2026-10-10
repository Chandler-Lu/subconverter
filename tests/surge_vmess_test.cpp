#include <iostream>
#include <stdexcept>

#include "generator/config/subexport.h"
#include "parser/subparser.h"

namespace {
void require(bool condition, const char *message) {
    if (!condition)
        throw std::runtime_error(message);
}

std::string convert(const YAML::Node &source, bool fallback, int version = 5, bool list = true) {
    std::vector<Proxy> nodes;
    explodeSub(YAML::Dump(source), nodes);
    require(nodes.size() == 1, "Clash fixture must parse as one node");
    std::vector<RulesetContent> rules;
    ProxyGroupConfig group;
    group.Name = "Choose";
    group.Type = ProxyGroupType::Select;
    group.Proxies = {".*"};
    extra_settings ext;
    ext.nodelist = list;
    ext.enable_rule_generator = false;
    ext.surge_h2_to_tcp = fallback;
    return proxyToSurge(nodes, "[General]\nloglevel = notify\n[Proxy]\n[Proxy Group]\n[Rule]\nFINAL,Choose\n",
                        rules, {group}, version, ext);
}
}

int main() {
    try {
        auto source = YAML::Load(R"(proxies:
  - name: H2-test
    type: vmess
    server: example.com
    port: 27103
    uuid: 00000000-0000-4000-8000-000000000001
    cipher: auto
    network: h2
    tls: true
    h2-opts:
      host: [h2.example.com]
      path: /h2-only
)");
        require(!extra_settings().surge_h2_to_tcp, "Fallback must default to disabled");
        require(convert(source, false).empty(), "Default export must still skip H2");
        const auto output = convert(source, true);
        require(output == "H2-test = vmess, example.com, 27103, username=00000000-0000-4000-8000-000000000001, tls=true, vmess-aead=true\n",
                "Fallback must export the node as plain VMess over TLS without H2 options");
        const auto full = convert(source, true, 5, false);
        require(full.find("H2-test = vmess,") != std::string::npos, "Full profile must retain H2 node");
        require(full.find("Choose = select,H2-test") != std::string::npos, "Group must retain H2 node");
        require(full.find("FINAL,Choose") != std::string::npos, "Original rules must remain present");

        auto proxy = source["proxies"][0];
        proxy["servername"] = "tls.example.com";
        proxy["skip-cert-verify"] = false;
        proxy["udp"] = true;
        const auto with_sni = convert(source, true);
        require(with_sni.find(", sni=tls.example.com") != std::string::npos, "Explicit SNI must survive");
        require(with_sni.find(", skip-cert-verify=false") != std::string::npos, "Certificate verification must survive");
        require(with_sni.find(", udp-relay=true") != std::string::npos, "UDP setting must survive");
        require(with_sni.find("h2.example.com") == std::string::npos, "H2 Host must not replace TLS SNI");
        proxy["alterId"] = 64;
        require(convert(source, true).find("vmess-aead=false") != std::string::npos, "Legacy AEAD mapping must survive");

        for (const int version : {3, 4, -3})
            require(convert(source, true, version).empty(), "Fallback must not affect older Surge or Surfboard");
        proxy["tls"] = false;
        require(convert(source, true).empty(), "Fallback must not allow unencrypted H2");
        proxy["tls"] = true;
        proxy["network"] = "grpc";
        require(convert(source, true).empty(), "gRPC must not fall back to TCP");
        proxy["network"] = "tcp";
        require(!convert(source, true).empty(), "TCP must still export");
        require(convert(source, true) == convert(source, false), "TCP export must be unchanged");
        proxy["network"] = "ws";
        proxy["ws-opts"]["path"] = "/ws-test";
        proxy["ws-opts"]["headers"]["Host"] = "ws.example.com";
        require(convert(source, true) == convert(source, false), "WS export must be unchanged");
        require(convert(source, true).find("ws=true, ws-path=/ws-test") != std::string::npos,
                "WS transport options must survive");
        require(convert(source, true).find("ws-headers=Host:ws.example.com") != std::string::npos,
                "WS headers must survive");
        std::cout << "Surge VMess conversion checks passed\n";
    } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
