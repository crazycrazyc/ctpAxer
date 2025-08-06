#ifndef APPCONFIG_H
#define APPCONFIG_H

#include <string>

struct CAppConfig
{
    std::string brokerid;
    std::string userid;
    std::string password;
    std::string md_server;
    std::string td_server;
    std::string appid;
    std::string authcode;
    std::string dayend;
    std::string nightend;

    CAppConfig();
};

#endif