#ifndef TRADINGACCOUNTCONVERTER_H
#define TRADINGACCOUNTCONVERTER_H

#include <string>
#include <vector>

// 资金账户数据结构，对应CThostFtdcTradingAccountField
struct CTPTradingAccountField {
    std::string BrokerID;               // 经纪公司代码
    std::string AccountID;              // 投资者帐号
    double PreMortgage;                 // 上次质押金额
    double PreCredit;                   // 上次信用额度
    double PreDeposit;                  // 上次存款额
    double PreBalance;                  // 上次结算准备金
    double PreMargin;                   // 上次占用的保证金
    double InterestBase;                // 利息基数
    double Interest;                    // 利息收入
    double Deposit;                     // 入金金额
    double Withdraw;                    // 出金金额
    double FrozenMargin;                // 冻结的保证金
    double FrozenCash;                  // 冻结的资金
    double FrozenCommission;            // 冻结的手续费
    double CurrMargin;                  // 当前保证金总额
    double CashIn;                      // 资金差额
    double Commission;                  // 手续费
    double CloseProfit;                 // 平仓盈亏
    double PositionProfit;              // 持仓盈亏
    double Balance;                     // 期货结算准备金
    double Available;                   // 可用资金
    double WithdrawQuota;               // 可取资金
    double Reserve;                     // 基本准备金
    std::string TradingDay;             // 交易日
    int SettlementID;                   // 结算编号
    double Credit;                      // 信用额度
    double Mortgage;                    // 质押金额
    double ExchangeMargin;              // 交易所保证金
    double DeliveryMargin;              // 投资者交割保证金
    double ExchangeDeliveryMargin;      // 交易所交割保证金
    double ReserveBalance;              // 保底期货结算准备金
    std::string CurrencyID;             // 币种代码
    double PreFundMortgageIn;           // 上次货币质入金额
    double PreFundMortgageOut;          // 上次货币质出金额
    double FundMortgageIn;              // 货币质入金额
    double FundMortgageOut;             // 货币质出金额
    double FundMortgageAvailable;       // 货币质押余额
    double MortgageableFund;            // 可质押货币金额
    double SpecProductMargin;           // 特殊产品占用保证金
    double SpecProductFrozenMargin;     // 特殊产品冻结保证金
    double SpecProductCommission;       // 特殊产品手续费
    double SpecProductFrozenCommission; // 特殊产品冻结手续费
    double SpecProductPositionProfit;   // 特殊产品持仓盈亏
    double SpecProductCloseProfit;      // 特殊产品平仓盈亏
    double SpecProductPositionProfitByAlg; // 根据持仓盈亏算法计算的特殊产品持仓盈亏
    double SpecProductExchangeMargin;   // 特殊产品交易所保证金
    std::string BizType;                // 业务类型
    double FrozenSwap;                  // 延时换汇冻结金额
    double RemainSwap;                  // 剩余换汇额度
};

class TradingAccountConverter {
public:
    static bool isCSVFormat(const std::string& data);
    static CTPTradingAccountField parseCSV(const std::string& csvData);
    static std::string convertToDatabaseFormat(const CTPTradingAccountField& account);
    static std::vector<std::string> split(const std::string& str, char delimiter);
    static double safeStringToDouble(const std::string& str);
    static int safeStringToInt(const std::string& str);
};

#endif // TRADINGACCOUNTCONVERTER_H 