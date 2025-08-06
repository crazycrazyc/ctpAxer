#include <iostream>
#include <linux/limits.h>
#include <string>
#include <cstring>
#include <sstream>
#include <chrono>
#include <thread>
#include <vector>
#include <algorithm>
#include "zlog.h"
#include "CTPQuote.h"
#include "ThostFtdcMdApi.h"
#include "readerwriterqueue.h"

using namespace std;
using namespace moodycamel;
using Sec = chrono::seconds;
using Ms  = chrono::microseconds;

// external global variable
extern zlog_category_t *cat;
extern vector<string>  contracts;
extern vector<string> DECSymbols;
// lockfree threadsafe queue
extern ReaderWriterQueue<string> q;

void CTPMarketSpi::OnFrontConnected()
{
    zlog_info(cat, "[CTPMarketSpi::OnFrontConnected] .");
    login();
}

void CTPMarketSpi::OnFrontDisconnected(int nReason)
{
    zlog_error(cat, "[CTPMarketSpi::OnFrontDisconnected] entered. Reason: %d", nReason);
    zlog_info(cat, "[CTPMarketSpi::OnFrontDisconnected] sleep for 10 seconds.");
    chrono::seconds dura(10);
    this_thread::sleep_for(dura);
}

void CTPMarketSpi::OnHeartBeatWarning(int nTimeLapse)
{
    zlog_info(cat, "[CTPMarketSpi::OnHeartBeatWarning] [%d]", nTimeLapse);
}

void CTPMarketSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
                                  CThostFtdcRspInfoField      *pRspInfo,
                                  int                         nRequestID,
                                  bool                        bIsLast)
{
    zlog_info(cat, "[CTPMarketSpi::OnRspUserLogin] .");
    if (pRspInfo->ErrorID != 0)
    {
        zlog_error(cat, "[CTPMarketSpi::OnRspUserLogin] 行情服务器登陆出错，错误码:%d", pRspInfo->ErrorID);
        zlog_error(cat, "[CTPMarketSpi::OnRspUserLogin] 行情服务器登陆出错，错误信息:%s", pRspInfo->ErrorMsg);
        return;
    }

    zlog_info(cat, "[CTPMarketSpi::OnRspUserLogin] 行情服务器登陆成功");
    zlog_info(cat, "[CTPMarketSpi::OnRspUserLogin] 当前交易日: %s", api->GetTradingDay());

    subscribe(contracts);
}

void CTPMarketSpi::OnRspUserLogout(CThostFtdcUserLogoutField  *pUserLogout,
                                   CThostFtdcRspInfoField     *pRspInfo,
                                   int                        nRequestID,
                                   bool                       bIsLast)
{
    zlog_info(cat, "[CTPMarketSpi::OnRspUserLogout] .");
}

void CTPMarketSpi::OnRspQryMulticastInstrument(CThostFtdcMulticastInstrumentField *pMulticastInstrument,
                                               CThostFtdcRspInfoField             *pRspInfo,
                                               int                                nRequestID,
                                               bool                               bIsLast)
{
    zlog_info(cat, "[CTPMarketSpi::OnRspQryMulticastInstrument]");
}

void CTPMarketSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    zlog_info(cat, "[CTPMarketSpi::OnRspError]");
    if (pRspInfo->ErrorID != 0)
    {
        zlog_error(cat, "[CTPMarketSpi::OnRspError]行情错误回报 错误代码: [%d]", pRspInfo->ErrorID);
        zlog_error(cat, "[CTPMarketSpi::OnRspError]行情错误回报 错误信息: [%s]", pRspInfo->ErrorMsg);
    }
}

void CTPMarketSpi::OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument,
                                      CThostFtdcRspInfoField            *pRspInfo,
                                      int                               nRequestID,
                                      bool                              bIsLast)
{
    if (pRspInfo->ErrorID != 0)
    {
        zlog_error(cat, "[CTPMarketSpi::OnRspSubMarketData]行情订阅回报 错误代码: [%d]", pRspInfo->ErrorID);
        zlog_error(cat, "[CTPMarketSpi::OnRspSubMarketData]行情订阅回报 错误信息: [%s]", pRspInfo->ErrorMsg);
        return;
    }
    zlog_info(cat, "[CTPMarketSpi::OnRspSubMarketData] 订阅合约[%s]成功", pSpecificInstrument->InstrumentID);
}

void CTPMarketSpi::OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument,
                                        CThostFtdcRspInfoField            *pRspInfo,
                                        int                               nRequestID,
                                        bool                              bIsLast)
{
    if (pRspInfo->ErrorID != 0)
    {
        zlog_error(cat, "[CTPMarketSpi::OnRspUnSubMarketData] 行情取消订阅回报 错误代码: [%d]", pRspInfo->ErrorID);
        zlog_error(cat, "[CTPMarketSpi::OnRspUnSubMarketData] 行情取消订阅回报 错误信息: [%s]", pRspInfo->ErrorMsg);
        return;
    }
    zlog_info(cat, "[CTPMarketSpi::OnRspUnSubMarketData] 取消订阅合约[%s]成功", pSpecificInstrument->InstrumentID);
}

void CTPMarketSpi::OnRspSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument,
                                       CThostFtdcRspInfoField            *pRspInfo,
                                       int                               nRequestID,
                                       bool                              bIsLast)
{
    zlog_info(cat, "[CTPMarketSpi::OnRspSubForQuoteRsp] .");
}

void CTPMarketSpi::OnRspUnSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument,
                                         CThostFtdcRspInfoField            *pRspInfo,
                                         int                               nRequestID,
                                         bool                              bIsLast)
{
    zlog_info(cat, "[CTPMarketSpi::OnRspUnSubForQuoteRsp] .");
}

string get_prefix(string instrID)
{
    string ret;
    for (auto& c: instrID)
    {
        if (c >= '0' && c <= '9')
        {
            break;
        }
        ret.push_back(c);
    }
    return ret;
}

void CTPMarketSpi::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData)
{
    // 本地时间戳
    auto localnow = chrono::high_resolution_clock::now();
    auto duration = localnow.time_since_epoch();
    Sec sss = chrono::duration_cast<Sec>(duration);
    Ms mss = chrono::duration_cast<Ms>(duration);
    time_t now_l = chrono::system_clock::to_time_t(localnow);
    char tmstr[32] = { 0 };
    char tmres[32] = { 0 };
    char dcetm[32] = { 0 };
    strftime(tmstr, sizeof(tmstr), "%Y-%m-%d %H:%M:%S.%%06u", localtime(&now_l));
    snprintf(tmres, sizeof(tmres), tmstr, mss.count() - sss.count() * 1000000);
    vector<string>::iterator it = find(DECSymbols.begin(), DECSymbols.end(), get_prefix(pDepthMarketData->InstrumentID));
    string actionday(pDepthMarketData->ActionDay);
    if (it != DECSymbols.end())
    {
        strftime(dcetm, sizeof(dcetm), "%Y%m%d", localtime(&now_l));
        actionday = dcetm;
    }

    ostringstream oss;
    oss.precision(12);
    oss << "EL/CTP_XZ/" << pDepthMarketData->InstrumentID << "/O/1 || "
        << pDepthMarketData->InstrumentID << ","
        << actionday << " " << pDepthMarketData->UpdateTime << "." << pDepthMarketData->UpdateMillisec << ","
        << (pDepthMarketData->BidPrice1 > 1000000.0 ? 0 : pDepthMarketData->BidPrice1) << ","
        << (pDepthMarketData->AskPrice1 > 1000000.0 ? 0 : pDepthMarketData->AskPrice1) << ","
        << pDepthMarketData->BidVolume1 << ","
        << pDepthMarketData->AskVolume1 << ","
        << pDepthMarketData->LastPrice << ","
        << pDepthMarketData->Volume << ",,"
        << pDepthMarketData->OpenInterest << ","
        << pDepthMarketData->Turnover << ","
        << tmres << "," << pDepthMarketData->TradingDay;
    // zlog_info(cat, "%s", oss.str().c_str());
    if (!q.try_enqueue(oss.str()))
    {
         cerr << oss.str() << endl;
    }
}

void CTPMarketSpi::OnRtnForQuoteRsp(CThostFtdcForQuoteRspField *pForQuoteRsp)
{
    zlog_info(cat, "[CTPMarketSpi::OnRtnForQuoteRsp] .");
}

/// user function
bool CTPMarketSpi::Create(const CAppConfig& appConfig)
{
    api = CThostFtdcMdApi::CreateFtdcMdApi();
    if (api)
    {
        char addrBuf[PATH_MAX] = {0};
        snprintf(addrBuf, sizeof(addrBuf), "tcp://%s", appConfig.md_server.c_str());
        brokerid = appConfig.brokerid;
        userid   = appConfig.userid;
        password = appConfig.password;

        struct tm etm;
        time_t now = time(nullptr);
        strptime(appConfig.dayend.c_str(), "%H:%M:%S", &etm);
        tm *pExitTM = localtime(&now);
        pExitTM->tm_hour = etm.tm_hour;
        pExitTM->tm_min = etm.tm_min+5;
        pExitTM->tm_sec = 0;
        exitTs = mktime(pExitTM);

        if (now > exitTs)
        {
            struct tm nightm;
            strptime(appConfig.nightend.c_str(), "%H:%M:%S", &nightm);
            time_t exitTM = now + 24 * 3600;
            pExitTM = localtime(&exitTM);
            pExitTM->tm_hour = nightm.tm_hour;
            pExitTM->tm_min = nightm.tm_min;
            pExitTM->tm_sec = 59;
            exitTs = mktime(pExitTM);
        }

        zlog_info(cat, "[CTPMarketSpi::Create] API Version: %s", api->GetApiVersion());
        zlog_info(cat, "[CTPMarketSpi::Create] Server Addr: %s", addrBuf);
        zlog_info(cat, "[CTPMarketSpi::Create] exit at %ld", exitTs);

        api->RegisterSpi(this);
        api->RegisterFront(addrBuf);
        api->Init();
        requestid = 0;
        return true;
    }
    return false;
}

void CTPMarketSpi::login()
{
    zlog_info(cat, "[CTPMarketSpi::login] .");

    CThostFtdcReqUserLoginField req = {0};

    strncpy(req.BrokerID, brokerid.c_str(), sizeof(TThostFtdcBrokerIDType));
    strncpy(req.UserID, userid.c_str(), sizeof(TThostFtdcUserIDType));
    strncpy(req.Password, password.c_str(), sizeof(TThostFtdcPasswordType));

    if (api)
    {
        int ret = api->ReqUserLogin(&req, ++requestid);
        switch (ret)
        {
        case 0:
            zlog_info(cat, "[CTPMarketSpi::login] 登陆请求发送成功. [0]");
            break;
        case -1:
            zlog_error(cat, "[CTPMarketSpi::login] 因为网络原因，登陆请求发送失败 [-1]");
            break;
        case -2:
            zlog_error(cat, "[CTPMarketSpi::login] 未处理，请求队列总数量超限 [-2]");
            break;
        case -3:
            zlog_error(cat, "[CTPMarketSpi::login] 每秒发送请求数量超限 [-3]");
            break;
        default:
            zlog_error(cat, "[CTPMarketSpi::login] 未知返回错误码 [%d]", ret);
            break;
        }
    }
    else
    {
        zlog_error(cat, "[CTPMarketSpi::login] api is invalid.");
    }
}

bool CTPMarketSpi::inner_sub(char pp[][32], int num)
{
    char **ppInstruments = new char* [num];
    for (int i=0; i < num; ++i)
    {
        ppInstruments[i] = new char [32];
        memset(ppInstruments[i], 0, 32);
        strncpy(ppInstruments[i], pp[i], 32);
    }

    int ret = api->SubscribeMarketData(ppInstruments, num);
    for (int j = 0; j < num; ++j)
    {
        delete ppInstruments[j];
    }
    delete [] ppInstruments;
    switch (ret)
    {
    case 0:
        zlog_info(cat, "[CTPMarketSpi::subscribe] 订阅请求成功");
        return true;
    case -1:
        zlog_error(cat, "[CTPMarketSpi::subscribe] 网络连接失败");
        break;
    case -2:
        zlog_error(cat, "[CTPMarketSpi::subscribe] 未处理请求超过许可数");
        break;
    case -3:
        zlog_error(cat, "[CTPMarketSpi::subscribe] 每秒发送请求数超过许可数");
        break;

    default:
        zlog_error(cat, "[CTPMarketSpi::subscribe] 未知错误码: %d", ret);
        break;
    }

    return false;
}

void CTPMarketSpi::subscribe(std::vector<string>& contracts)
{
    zlog_info(cat, "[CTPMarketSpi::subscribe] 合约数量:%ld", contracts.size());
    char buf[100][32] = {0};
    int idx = 0;

    for (auto& m: contracts)
    {
        if (idx == 100)
        {
            idx = 0;
            if (!inner_sub(buf, 100))
            {
                zlog_error(cat, "[CTPMarketSpi::subscribe] 订阅错误");
            }
            for (int j=0; j < 100; ++j)
            {
                memset(buf[j], 0, 32);
            }
        }
        snprintf(buf[idx], 32, "%s", m.c_str());
        ++idx;
    }
    // zlog_info(cat, "[CTPMarketSpi::subscribe] lastone: %s", buf[idx-1]);
    if (!inner_sub(buf, idx))
    {
        zlog_error(cat, "[CTPMarketSpi::subscribe] 订阅错误");
    }
}

void CTPMarketSpi::Destroy()
{
    zlog_info(cat, "[CTPMarketSpi::Destroy] .");
    if (api)
    {
        api->Release();
        api = nullptr;
    }
    zlog_info(cat, "[CTPMarketSpi::Destroy] api released.");
}