#ifndef CTPQUOTE_H
#define CTPQUOTE_H

#include <vector>
#include "ThostFtdcMdApi.h"
#include <appConfig.h>

class CTPMarketSpi:public CThostFtdcMdSpi
{
public:
    ///当客户端与交易后台建立起通信连接时（还未登录前），该方法被调用。
    void OnFrontConnected();

    ///当客户端与交易后台通信连接断开时，该方法被调用。当发生这个情况后，API会自动重新连接，客户端可不做处理。
    void OnFrontDisconnected(int nReason);

    ///心跳超时警告。当长时间未收到报文时，该方法被调用。
    void OnHeartBeatWarning(int nTimeLapse);

    ///登录请求响应
    void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
                        CThostFtdcRspInfoField      *pRspInfo,
                        int                         nRequestID,
                        bool                        bIsLast);

    ///登出请求响应
    void OnRspUserLogout(CThostFtdcUserLogoutField  *pUserLogout,
                         CThostFtdcRspInfoField     *pRspInfo,
                         int                        nRequestID,
                         bool                       bIsLast);

    ///请求查询组播合约响应
    void OnRspQryMulticastInstrument(CThostFtdcMulticastInstrumentField *pMulticastInstrument,
                                     CThostFtdcRspInfoField             *pRspInfo,
                                     int                                nRequestID,
                                     bool                               bIsLast);

    ///错误应答
    void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    ///订阅行情应答
    void OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument,
                            CThostFtdcRspInfoField            *pRspInfo,
                            int                               nRequestID,
                            bool                              bIsLast);

    ///取消订阅行情应答
    void OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument,
                              CThostFtdcRspInfoField            *pRspInfo,
                              int                               nRequestID,
                              bool                              bIsLast);

    ///订阅询价应答
    void OnRspSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument,
                             CThostFtdcRspInfoField            *pRspInfo,
                             int                               nRequestID,
                             bool                              bIsLast);

    ///取消订阅询价应答
    void OnRspUnSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument,
                               CThostFtdcRspInfoField            *pRspInfo,
                               int                               nRequestID,
                               bool                              bIsLast);

    ///深度行情通知
    void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData);

    ///询价通知
    void OnRtnForQuoteRsp(CThostFtdcForQuoteRspField *pForQuoteRsp);

public:
    bool Create(const CAppConfig&);
    void login();
    void subscribe(std::vector<std::string>&);
    bool inner_sub(char pp[][32], int );
    void Destroy();

public:
    std::time_t     exitTs;

private:

    CThostFtdcMdApi *api;
    std::string     brokerid;
    std::string     userid;
    std::string     password;
    int             requestid;

};

#endif