#include <string>
#include <vector>
#include "zlog.h"
//#include <Config.h>
#include "../include/JsonConfig.h"
#include "appConfig.h"

using namespace std;

// global variable for zmq server
string pushServer;
vector<string> DECSymbols;

// external global variable
extern string currentPath;
extern zlog_category_t *cat;

CAppConfig::CAppConfig()
{
    //string config_file = currentPath + "/config.cfg";
    //Config cfg(config_file);
    //string config_file = currentPath + "/config.json";
    string config_file = currentPath + "/zycConfig.json";
    JsonConfig cfg(config_file);
    
    md_server = cfg.Read<string>("md_server");
    td_server = cfg.Read<string>("td_server");
    brokerid  = cfg.Read<string>("brokerid");
    userid    = cfg.Read<string>("userid");
    password  = cfg.Read<string>("password");
    appid     = cfg.Read<string>("appid");
    authcode  = cfg.Read<string>("authcode");
    dayend    = cfg.Read<string>("dayend");
    nightend  = cfg.Read<string>("nightend");
    pushServer= cfg.Read<string>("ZMQServer");

    zlog_info(cat, "[CAppConfig] load config from config file successfully.");
    zlog_info(cat, "[CAppConfig] %9s: %-20s", "brokerid", brokerid.c_str());
    zlog_info(cat, "[CAppConfig] %9s: %-20s", "userid", userid.c_str());
    zlog_info(cat, "[CAppConfig] %9s: %-20s", "password", password.c_str());
    zlog_info(cat, "[CAppConfig] %9s: %-20s", "appid", appid.c_str());
    zlog_info(cat, "[CAppConfig] %9s: %-20s", "authcode", authcode.c_str());
    zlog_info(cat, "[CAppConfig] %9s: %-20s", "authcode", authcode.c_str());
    zlog_info(cat, "[CAppConfig] %9s: %-20s", "ZMQServer", pushServer.c_str());

    if (!md_server.empty())
    {
        zlog_info(cat, "[CAppConfig] %9s: %-20s", "md_server", md_server.c_str());
    }
    if (!td_server.empty())
    {
        zlog_info(cat, "[CAppConfig] %9s: %-20s", "td_server", td_server.c_str());
    }

    // string decSymbols = cfg.Read<string>("DEC");
    // string delim = ",";
    // size_t prepos = 0;
    // size_t findpos = 0;
    // while (1)
    // {
    //     findpos = decSymbols.find(delim, prepos);
    //     if (findpos != string::npos)
    //     {
    //         DECSymbols.push_back(decSymbols.substr(prepos, findpos-prepos));
    //         prepos = findpos + 1;
    //     }
    //     else
    //     {
    //         DECSymbols.push_back(decSymbols.substr(prepos));
    //         break;
    //     }
    // }
    DECSymbols = cfg.ReadArray("DEC");
    for (auto& m: DECSymbols)
    {
        zlog_info(cat, "[CAppConfig] %9s: %-20s", "DEC", m.c_str());
    }
}
