#ifndef CTPTRADER_H
#define CTPTRADER_H

#include "ThostFtdcTraderApi.h"
#include "appConfig.h"

class CTPTraderSpi:public CThostFtdcTraderSpi
{
public:
    void OnFrontConnected();
    void OnFrontDisconnected(int nReason);
    void OnHeartBeatWarning(int nTimeLapse);

    void OnRspAuthenticate(CThostFtdcRspAuthenticateField *pRspAuthenticateField,
                           CThostFtdcRspInfoField         *pRspInfo,
                           int                            nRequestID,
                           bool                           bIsLast);

    void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
                        CThostFtdcRspInfoField      *pRspInfo,
                        int                         nRequestID,
                        bool                        bIsLast);

    void OnRspUserPasswordUpdate(CThostFtdcUserPasswordUpdateField *pUserPasswordUpdate,
                                 CThostFtdcRspInfoField            *pRspInfo,
                                 int                               nRequestID,
                                 bool                              bIsLast);

    void OnRspTradingAccountPasswordUpdate(
        CThostFtdcTradingAccountPasswordUpdateField *pTradingAccountPasswordUpdate,
        CThostFtdcRspInfoField                      *pRspInfo,
        int                                         nRequestID,
        bool                                        bIsLast);

    void OnRspUserAuthMethod(CThostFtdcRspUserAuthMethodField *pRspUserAuthMethod,
                             CThostFtdcRspInfoField           *pRspInfo,
                             int                              nRequestID,
                             bool                             bIsLast);

    void OnRspGenUserCaptcha(CThostFtdcRspGenUserCaptchaField *pRspGenUserCaptcha,
                             CThostFtdcRspInfoField           *pRspInfo,
                             int                              nRequestID,
                             bool                             bIsLast);

    void OnRspGenUserText(CThostFtdcRspGenUserTextField *pRspGenUserText,
                          CThostFtdcRspInfoField        *pRspInfo,
                          int                           nRequestID,
                          bool                          bIsLast);

    void OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder,
                          CThostFtdcRspInfoField    *pRspInfo,
                          int                       nRequestID,
                          bool                      bIsLast);

    void OnRspParkedOrderInsert(CThostFtdcParkedOrderField *pParkedOrder,
                                CThostFtdcRspInfoField     *pRspInfo,
                                int                        nRequestID,
                                bool                       bIsLast);

    void OnRspParkedOrderAction(CThostFtdcParkedOrderActionField *pParkedOrderAction,
                                CThostFtdcRspInfoField           *pRspInfo,
                                int                              nRequestID,
                                bool                             bIsLast);

    void OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction,
                          CThostFtdcRspInfoField          *pRspInfo,
                          int                             nRequestID,
                          bool                            bIsLast);

    void OnRspQueryMaxOrderVolume(CThostFtdcQueryMaxOrderVolumeField *pQueryMaxOrderVolume,
                                  CThostFtdcRspInfoField             *pRspInfo,
                                  int                                nRequestID,
                                  bool                               bIsLast);

    void OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm,
                                    CThostFtdcRspInfoField               *pRspInfo,
                                    int                                  nRequestID,
                                    bool                                 bIsLast);

    void OnRspRemoveParkedOrder(CThostFtdcRemoveParkedOrderField *pRemoveParkedOrder,
                                CThostFtdcRspInfoField           *pRspInfo,
                                int                              nRequestID,
                                bool                             bIsLast);

    void OnRspRemoveParkedOrderAction(CThostFtdcRemoveParkedOrderActionField *pRemoveParkedOrderAction,
                                      CThostFtdcRspInfoField                 *pRspInfo,
                                      int                                    nRequestID,
                                      bool                                   bIsLast);

    void OnRspBatchOrderAction(CThostFtdcInputBatchOrderActionField *pInputBatchOrderAction,
                               CThostFtdcRspInfoField               *pRspInfo,
                               int                                  nRequestID,
                               bool                                 bIsLast);

    void OnRspQryOrder(CThostFtdcOrderField     *pOrder,
                       CThostFtdcRspInfoField   *pRspInfo,
                       int                      nRequestID,
                       bool                     bIsLast);

    void OnRspQryTrade(CThostFtdcTradeField     *pTrade,
                       CThostFtdcRspInfoField   *pRspInfo,
                       int                      nRequestID,
                       bool                     bIsLast);

    void OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition,
                                  CThostFtdcRspInfoField          *pRspInfo,
                                  int                             nRequestID,
                                  bool                            bIsLast);

    void OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount,
                                CThostFtdcRspInfoField        *pRspInfo,
                                int                           nRequestID,
                                bool                          bIsLast);

    void OnRspQryInvestor(CThostFtdcInvestorField   *pInvestor,
                          CThostFtdcRspInfoField    *pRspInfo,
                          int                       nRequestID,
                          bool                      bIsLast);

    void OnRspQryInstrumentMarginRate(CThostFtdcInstrumentMarginRateField *pInstrumentMarginRate,
                                      CThostFtdcRspInfoField              *pRspInfo,
                                      int                                 nRequestID,
                                      bool                                bIsLast);

    void OnRspQryInstrumentCommissionRate(CThostFtdcInstrumentCommissionRateField *pInstrumentCommissionRate,
                                          CThostFtdcRspInfoField                  *pRspInfo,
                                          int                                     nRequestID,
                                          bool                                    bIsLast);

    void OnRspQryExchange(CThostFtdcExchangeField *pExchange, CThostFtdcRspInfoField *pRspInfo,
        int nRequestID, bool bIsLast);

    void OnRspQryProduct(CThostFtdcProductField *pProduct, CThostFtdcRspInfoField *pRspInfo,
        int nRequestID, bool bIsLast);

    void OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo,
        int nRequestID, bool bIsLast);

    void OnRspQryDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData,
                                 CThostFtdcRspInfoField         *pRspInfo,
                                 int                            nRequestID,
                                 bool                           bIsLast);

    void OnRspQryInvestorPositionDetail(CThostFtdcInvestorPositionDetailField *pInvestorPositionDetail,
                                        CThostFtdcRspInfoField                *pRspInfo,
                                        int                                   nRequestID,
                                        bool                                  bIsLast);

    void OnRspQryExchangeMarginRate(CThostFtdcExchangeMarginRateField *pExchangeMarginRate,
                                    CThostFtdcRspInfoField            *pRspInfo,
                                    int                               nRequestID,
                                    bool                              bIsLast);

    void OnRspQryProductGroup(CThostFtdcProductGroupField *pProductGroup,
                              CThostFtdcRspInfoField      *pRspInfo,
                              int                         nRequestID,
                              bool                        bIsLast);

    void OnRspQryInstrumentOrderCommRate(CThostFtdcInstrumentOrderCommRateField *pInstrumentOrderCommRate,
                                         CThostFtdcRspInfoField                 *pRspInfo,
                                        int                                     nRequestID,
                                        bool                                    bIsLast);

    void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    void OnRtnOrder(CThostFtdcOrderField *pOrder);

    void OnRtnTrade(CThostFtdcTradeField *pTrade);

    void OnErrRtnOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo);

    void OnErrRtnOrderAction(CThostFtdcOrderActionField *pOrderAction, CThostFtdcRspInfoField *pRspInfo);

    void OnRtnInstrumentStatus(CThostFtdcInstrumentStatusField *pInstrumentStatus);

    void OnRtnExecOrder(CThostFtdcExecOrderField *pExecOrder);


public:
    // user define function
    bool Create(const CAppConfig&);
    void Authenticate();
    void Login();
    void ReqInstruments();
    void ReqInvestorPositions();
    void ReqTradingAccount();
    void Destroy();

private:
    CThostFtdcTraderApi *api;
    std::string         brokerid;
    std::string         userid;
    std::string         password;
    std::string         appid;
    std::string         authcode;
    int                 requestid;
    
    // 数据库管理器
};

#endif
