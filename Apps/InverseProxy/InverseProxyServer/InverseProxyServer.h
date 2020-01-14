#pragma once

#include <string>
#include <map>

#include <spdlog/common.h>


typedef std::map<const std::string, const std::string> AuthTokens;

struct InverseProxyServerConfig
{
    spdlog::level::level_enum logLevel = spdlog::level::info;

    std::string serverName;
    std::string certificate;
    std::string key;

    unsigned short frontPort;
    unsigned short secureFrontPort;

    unsigned short backPort;
    unsigned short secureBackPort;

    std::string turnServer;
    std::string turnAuthSecret;

    AuthTokens backAuthTokens;
};

int InverseProxyServerMain(const InverseProxyServerConfig&);
