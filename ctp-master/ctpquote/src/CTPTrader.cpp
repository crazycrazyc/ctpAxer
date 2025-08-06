#include <string>
#include <cstring>
#include <linux/limits.h>
#include <chrono>
#include <thread>
#include <vector>
#include <condition_variable>
#include <mutex>
#include "zlog.h"
#include "CTPTrader.h"

using namespace std;

// global variable
vector<string>      contracts;
condition_variable  cv;
mutex               m;
bool                isReady = false;

// external global variable
extern zlog_category_t *cat;

void CTPTraderSpi::OnFrontConnected()
{
    zlog_info(cat, "[CTPTraderSpi::OnFrontConnected] .");
    Authenticate();
}

void CTPTraderSpi::OnFrontDisconnected(int nReason)
{
    zlog_info(cat, "[CTPTraderSpi::OnFrontDisconnected] Reason: %d", nReason);
    chrono::seconds dura(10);
    this_thread::sleep_for(dura);
}

void CTPTraderSpi::OnHeartBeatWarning(int nTimeLapse)
{
    zlog_info(cat, "[CTPTraderSpi::OnHeartBeatWarning] %d", nTimeLapse);
}

void CTPTraderSpi::OnRspAuthenticate(CThostFtdcRspAuthenticateField *pRspAuthenticateField,
                                     CThostFtdcRspInfoField         *pRspInfo,
                                     int                            nRequestID,
                                     bool                           bIsLast)
{
    zlog_info(cat, "[CTPTraderSpi::OnRspAuthenticate] .");
    if (pRspInfo->ErrorID != 0)
    {
        zlog_error(cat, "[CTPTraderSpi::OnRspAuthenticate] 客户端认证回报 错误代码: [%d]", pRspInfo->ErrorID);
        zlog_error(cat, "[CTPTraderSpi::OnRspAuthenticate] 客户端认证回报 错误信息: [%s]", pRspInfo->ErrorMsg);
        return;
    }
    Login();
}

void CTPTraderSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
                                  CThostFtdcRspInfoField      *pRspInfo,
                                  int                         nRequestID,
                                  bool                        bIsLast)
{
    zlog_info(cat, "[CTPTraderSpi::OnRspUserLogin] .");
    if (pRspInfo->ErrorID != 0)
    {
        zlog_error(cat, "[CTPTraderSpi::OnRspUserLogin] 交易服务器登陆出错，错误码:%d", pRspInfo->ErrorID);
        zlog_error(cat, "[CTPTraderSpi::OnRspUserLogin] 交易服务器登陆出错，错误信息:%s", pRspInfo->ErrorMsg);
        return;
    }

    zlog_info(cat, "[CTPTraderSpi::OnRspUserLogin] 交易服务器登陆成功");
    zlog_info(cat, "[CTPTraderSpi::OnRspUserLogin] 当前交易日: %s", api->GetTradingDay());

    zlog_info(cat, "[CTPTraderSpi::OnRspUserLogin] 查询合约");
    ReqInstruments();
}

void CTPTraderSpi::OnRspUserPasswordUpdate(CThostFtdcUserPasswordUpdateField *pUserPasswordUpdate,
                                           CThostFtdcRspInfoField            *pRspInfo,
                                           int                               nRequestID,
                                           bool                              bIsLast)
{
    zlog_info(cat, "[CTPTraderSpi::OnRspUserPasswordUpdate] .");
}

void CTPTraderSpi::OnRspTradingAccountPasswordUpdate(
        CThostFtdcTradingAccountPasswordUpdateField *pTradingAccountPasswordUpdate,
        CThostFtdcRspInfoField                      *pRspInfo,
        int                                         nRequestID,
        bool                                        bIsLast)
{
    zlog_info(cat, "[CTPTraderSpi::OnRspTradingAccountPasswordUpdate] .");
}

void CTPTraderSpi::OnRspUserAuthMethod(CThostFtdcRspUserAuthMethodField *pRspUserAuthMethod,
                                       CThostFtdcRspInfoField           *pRspInfo,
                                       int                              nRequestID,
                                       bool                             bIsLast)
{
    zlog_info(cat, "[CTPTraderSpi::OnRspUserAuthMethod] .");
}

void CTPTraderSpi::OnRspGenUserCaptcha(CThostFtdcRspGenUserCaptchaField *pRspGenUserCaptcha,
                                       CThostFtdcRspInfoField           *pRspInfo,
                                       int                              nRequestID,
                                       bool                             bIsLast)
{
    zlog_info(cat, "[CTPTraderSpi::OnRspGenUserCaptcha] .");
}

void CTPTraderSpi::OnRspGenUserText(CThostFtdcRspGenUserTextField *pRspGenUserText,
                                    CThostFtdcRspInfoField        *pRspInfo,
                                    int                           nRequestID,
                                    bool                          bIsLast)
{
    zlog_info(cat, "[CTPTraderSpi::OnRspGenUserText] .");
}

void CTPTraderSpi::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder,
                                    CThostFtdcRspInfoField    *pRspInfo,
                                    int                       nRequestID,
                                    bool                      bIsLast)
{
    zlog_info(cat, "[CTPTraderSpi::OnRspGenUserText] .");
}

void CTPTraderSpi::OnRspParkedOrderInsert(CThostFtdcParkedOrderField *pParkedOrder,
                                          CThostFtdcRspInfoField     *pRspInfo,
                                          int                        nRequestID,
                                          bool                       bIsLast)
{
    zlog_info(cat, "[CTPTraderSpi::OnRspParkedOrderInsert] .");
}

void CTPTraderSpi::OnRspParkedOrderAction(CThostFtdcParkedOrderActionField *pParkedOrderAction,
                                          CThostFtdcRspInfoField           *pRspInfo,
                                          int                              nRequestID,
                                          bool                             bIsLast)
{
    zlog_info(cat, "[CTPTraderSpi::OnRspParkedOrderAction] .");
}

void CTPTraderSpi::OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction,
                                    CThostFtdcRspInfoField          *pRspInfo,
                                    int                             nRequestID,
                                    bool                            bIsLast)
{
    zlog_info(cat, "[CTPTraderSpi::OnRspOrderAction] .");
}

void CTPTraderSpi::OnRspQueryMaxOrderVolume(CThostFtdcQueryMaxOrderVolumeField *pQueryMaxOrderVolume,
                                            CThostFtdcRspInfoField             *pRspInfo,
                                            int                                nRequestID,
                                            bool                               bIsLast)
{
    zlog_info(cat, "[CTPTraderSpi::OnRspQueryMaxOrderVolume] .");
}

void CTPTraderSpi::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm,
                                              CThostFtdcRspInfoField               *pRspInfo,
                                              int                                  nRequestID,
                                              bool                                 bIsLast)
{
    zlog_info(cat, "[CTPTraderSpi::OnRspSettlementInfoConfirm] .");
}

void CTPTraderSpi::OnRspRemoveParkedOrder(CThostFtdcRemoveParkedOrderField *pRemoveParkedOrder,
                                          CThostFtdcRspInfoField           *pRspInfo,
                                          int                              nRequestID,
                                          bool                             bIsLast)
{
    zlog_info(cat, "[CTPTraderSpi::OnRspRemoveParkedOrder] .");
}

void CTPTraderSpi::OnRspRemoveParkedOrderAction(
    CThostFtdcRemoveParkedOrderActionField *pRemoveParkedOrderAction,
    CThostFtdcRspInfoField                 *pRspInfo,
    int                                    nRequestID,
    bool                                   bIsLast)
{
    zlog_info(cat, "[CTPTraderSpi::OnRspRemoveParkedOrderAction] .");
}

void CTPTraderSpi::OnRspBatchOrderAction(CThostFtdcInputBatchOrderActionField *pInputBatchOrderAction,
                                         CThostFtdcRspInfoField               *pRspInfo,
                                         int                                  nRequestID,
                                         bool                                 bIsLast)
{
    zlog_info(cat, "[CTPTraderSpi::OnRspBatchOrderAction] .");
}

void CTPTraderSpi::OnRspQryOrder(CThostFtdcOrderField     *pOrder,
                                 CThostFtdcRspInfoField   *pRspInfo,
                                 int                      nRequestID,
                                 bool                     bIsLast)
{
    zlog_info(cat, "[CTPTraderSpi::OnRspQryOrder] .");
}

void CTPTraderSpi::OnRspQryTrade(CThostFtdcTradeField     *pTrade,
                                 CThostFtdcRspInfoField   *pRspInfo,
                                 int                      nRequestID,
                                 bool                     bIsLast)
{
    zlog_info(cat, "[CTPTraderSpi::OnRspQryTrade] .");
}

void CTPTraderSpi::OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition,
                                            CThostFtdcRspInfoField          *pRspInfo,
                                            int                             nRequestID,
                                            bool                            bIsLast)
{
    zlog_info(cat, "[CTPTraderSpi::OnRspQryInvestorPosition] .");
}

void CTPTraderSpi::OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount,
                                            CThostFtdcRspInfoField        *pRspInfo,
                                            int                           nRequestID,
                                            bool                          bIsLast)
{
    zlog_info(cat, "[CTPTraderSpi::OnRspQryTradingAccount] .");
}

void CTPTraderSpi::OnRspQryInvestor(CThostFtdcInvestorField   *pInvestor,
                                    CThostFtdcRspInfoField    *pRspInfo,
                                    int                       nRequestID,
                                    bool                      bIsLast)
{
    zlog_info(cat, "[CTPTraderSpi::OnRspQryInvestor] .");
}

void CTPTraderSpi::OnRspQryInstrumentMarginRate(
    CThostFtdcInstrumentMarginRateField *pInstrumentMarginRate,
    CThostFtdcRspInfoField              *pRspInfo,
    int                                 nRequestID,
    bool                                bIsLast)
{
    zlog_info(cat, "[CTPTraderSpi::OnRspQryInstrumentMarginRate] .");
}

void CTPTraderSpi::OnRspQryInstrumentCommissionRate(
    CThostFtdcInstrumentCommissionRateField *pInstrumentCommissionRate,
    CThostFtdcRspInfoField                  *pRspInfo,
    int                                     nRequestID,
    bool                                    bIsLast)
{
    zlog_info(cat, "[CTPTraderSpi::OnRspQryInstrumentCommissionRate] .");
}

void CTPTraderSpi::OnRspQryExchange(CThostFtdcExchangeField *pExchange,
                                    CThostFtdcRspInfoField  *pRspInfo,
                                    int                     nRequestID,
                                    bool                    bIsLast)
{
    zlog_info(cat, "[CTPTraderSpi::OnRspQryExchange] .");
}

void CTPTraderSpi::OnRspQryProduct(CThostFtdcProductField *pProduct,
                                   CThostFtdcRspInfoField *pRspInfo,
                                   int                    nRequestID,
                                   bool                   bIsLast)
{
    zlog_info(cat, "[CTPTraderSpi::OnRspQryProduct] .");
}

void CTPTraderSpi::OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument,
                                      CThostFtdcRspInfoField    *pRspInfo,
                                      int                       nRequestID,
                                      bool                      bIsLast)
{
    if (pRspInfo)
    {
        zlog_error(cat, "[CTPTraderSpi::OnRspQryInstrument] 错误码:%d", pRspInfo->ErrorID);
        zlog_error(cat, "[CTPTraderSpi::OnRspQryInstrument] 错误信息:%s", pRspInfo->ErrorMsg);
        return;
    }

    if (pInstrument)
    {
        string symbol(pInstrument->ExchangeInstID);
        if (symbol.find("efp") != string::npos || symbol.find("EFP") != string::npos) {

        }
        else if (symbol.length() > 6)
        {

        }
        else
        {
            // zlog_info(cat, "[CTPTraderSpi::OnRspQryInstrument] 合约代码:%s, 交易所代码:%s",
            //     pInstrument->ExchangeInstID, pInstrument->ExchangeID);
            contracts.push_back(pInstrument->ExchangeInstID);
        }


        if (bIsLast)
        {
            {
                std::lock_guard<std::mutex> lock(m);
                isReady = true;
            }

            cv.notify_one();
        }
    }
}

void CTPTraderSpi::OnRspQryDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData,
                                           CThostFtdcRspInfoField         *pRspInfo,
                                           int                            nRequestID,
                                           bool                           bIsLast)
{
    zlog_info(cat, "[CTPTraderSpi::OnRspQryDepthMarketData] .");
}

void CTPTraderSpi::OnRspQryInvestorPositionDetail(
    CThostFtdcInvestorPositionDetailField *pInvestorPositionDetail,
    CThostFtdcRspInfoField                *pRspInfo,
    int                                   nRequestID,
    bool                                  bIsLast)
{
    zlog_info(cat, "[CTPTraderSpi::OnRspQryInvestorPositionDetail] .");
}

void CTPTraderSpi::OnRspQryExchangeMarginRate(CThostFtdcExchangeMarginRateField *pExchangeMarginRate,
                                              CThostFtdcRspInfoField            *pRspInfo,
                                              int                               nRequestID,
                                              bool                              bIsLast)
{
    zlog_info(cat, "[CTPTraderSpi::OnRspQryExchangeMarginRate] .");
}

void CTPTraderSpi::OnRspQryProductGroup(CThostFtdcProductGroupField *pProductGroup,
                                        CThostFtdcRspInfoField      *pRspInfo,
                                        int                         nRequestID,
                                        bool                        bIsLast)
{
    zlog_info(cat, "[CTPTraderSpi::OnRspQryProductGroup] .");
}

void CTPTraderSpi::OnRspQryInstrumentOrderCommRate(
    CThostFtdcInstrumentOrderCommRateField *pInstrumentOrderCommRate,
    CThostFtdcRspInfoField                 *pRspInfo,
    int                                     nRequestID,
    bool                                    bIsLast)
{
    zlog_info(cat, "[CTPTraderSpi::OnRspQryInstrumentOrderCommRate] .");
}

void CTPTraderSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    zlog_info(cat, "[CTPTraderSpi::OnRspError] .");
    if (pRspInfo->ErrorID != 0)
    {
        zlog_error(cat, "[CTPTraderSpi::OnRspError]行情错误回报 错误代码: [%d]", pRspInfo->ErrorID);
        zlog_error(cat, "[CTPTraderSpi::OnRspError]行情错误回报 错误信息: [%s]", pRspInfo->ErrorMsg);
    }
}

void CTPTraderSpi::OnRtnOrder(CThostFtdcOrderField *pOrder)
{
    zlog_info(cat, "[CTPTraderSpi::OnRtnOrder] .");
}

void CTPTraderSpi::OnRtnTrade(CThostFtdcTradeField *pTrade)
{
    zlog_info(cat, "[CTPTraderSpi::OnRtnTrade] .");
}

void CTPTraderSpi::OnErrRtnOrderInsert(CThostFtdcInputOrderField *pInputOrder,
                                       CThostFtdcRspInfoField    *pRspInfo)
{
    zlog_info(cat, "[CTPTraderSpi::OnErrRtnOrderInsert] .");
}

void CTPTraderSpi::OnErrRtnOrderAction(CThostFtdcOrderActionField *pOrderAction,
                                       CThostFtdcRspInfoField     *pRspInfo)
{
    zlog_info(cat, "[CTPTraderSpi::OnErrRtnOrderAction] .");
}

void CTPTraderSpi::OnRtnInstrumentStatus(CThostFtdcInstrumentStatusField *pInstrumentStatus)
{
    zlog_info(cat, "[CTPTraderSpi::OnRtnInstrumentStatus] .");
    zlog_info(cat, "[CTPTraderSpi::OnRtnInstrumentStatus] 合约代码[%s],状态[%c],阶段编码[%d],时间[%s],原因[%c]",
        pInstrumentStatus->InstrumentID, pInstrumentStatus->InstrumentStatus, pInstrumentStatus->TradingSegmentSN,
        pInstrumentStatus->EnterTime, pInstrumentStatus->EnterReason);
}

void CTPTraderSpi::OnRtnExecOrder(CThostFtdcExecOrderField *pExecOrder)
{
    zlog_info(cat, "[CTPTraderSpi::OnRtnExecOrder] .");
}

/////////////////////////////////////////////////////////////////////////////////////////
// USER DEFINE FUNCTION
/////////////////////////////////////////////////////////////////////////////////////////

bool CTPTraderSpi::Create(const CAppConfig& appConfig)
{
    api = CThostFtdcTraderApi::CreateFtdcTraderApi();
    if (api)
    {
        char addrBuf[PATH_MAX] = {0};
        snprintf(addrBuf, sizeof(addrBuf), "tcp://%s", appConfig.td_server.c_str());
        brokerid = appConfig.brokerid;
        userid   = appConfig.userid;
        password = appConfig.password;
        appid    = appConfig.appid;
        authcode = appConfig.authcode;

        zlog_info(cat, "[CTPTraderSpi::Create] API Version: %s", api->GetApiVersion());
        zlog_info(cat, "[CTPTraderSpi::Create] Trade Server Addr: %s", addrBuf);

        api->RegisterSpi(this);
        api->SubscribePrivateTopic(THOST_TERT_QUICK);
        api->SubscribePublicTopic(THOST_TERT_QUICK);
        api->RegisterFront(addrBuf);
        api->Init();
        requestid = 0;
        return true;
    }
    return false;
}

void CTPTraderSpi::Authenticate()
{
    zlog_info(cat, "[CTPTraderSpi::Authenticate] .");
    CThostFtdcReqAuthenticateField req = {0};
    strncpy(req.BrokerID, brokerid.c_str(), sizeof(TThostFtdcBrokerIDType));
    strncpy(req.UserID, userid.c_str(), sizeof(TThostFtdcUserIDType));
    strncpy(req.AppID, appid.c_str(), sizeof(TThostFtdcAppIDType));
    strncpy(req.AuthCode, authcode.c_str(), sizeof(TThostFtdcAuthCodeType));
    int ret = api->ReqAuthenticate(&req, requestid++);
    zlog_info(cat, "[CTPTraderSpi::Authenticate] 客户端认证=%d", ret);
}

void CTPTraderSpi::Login()
{
    zlog_info(cat, "[CTPTraderSpi::login] .");

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
            zlog_info(cat, "[CTPTraderSpi::login] 登陆请求发送成功. [0]");
            break;
        case -1:
            zlog_error(cat, "[CTPTraderSpi::login] 因为网络原因，登陆请求发送失败 [-1]");
            break;
        case -2:
            zlog_error(cat, "[CTPTraderSpi::login] 未处理，请求队列总数量超限 [-2]");
            break;
        case -3:
            zlog_error(cat, "[CTPTraderSpi::login] 每秒发送请求数量超限 [-3]");
            break;
        default:
            zlog_error(cat, "[CTPTraderSpi::login] 未知返回错误码 [%d]", ret);
            break;
        }
    }
    else
    {
        zlog_error(cat, "[CTPTraderSpi::login] api is invalid.");
    }
}

void CTPTraderSpi::ReqInstruments()
{
    CThostFtdcQryInstrumentField req;
    memset(&req, 0, sizeof(CThostFtdcQryInstrumentField));
    int ret = api->ReqQryInstrument(&req, requestid++);
    zlog_info(cat, "[CTPTraderSpi::ReqInstruments] 发送请求合约=%d", ret);
}

void CTPTraderSpi::Destroy()
{
    zlog_info(cat, "[CTPTraderSpi::Destroy] .");
    if (api)
    {
        api->Release();
        api = nullptr;
    }
    zlog_info(cat, "[CTPTraderSpi::Destroy] api released.");
}
