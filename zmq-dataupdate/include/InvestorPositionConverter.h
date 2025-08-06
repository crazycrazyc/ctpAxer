#ifndef INVESTOR_POSITION_CONVERTER_H
#define INVESTOR_POSITION_CONVERTER_H

#include <string>
#include <vector>
#include <memory>

// CTP投资者持仓字段结构（简化版，对应CThostFtdcInvestorPositionField）
struct CTPInvestorPositionField {
    std::string BrokerID;               // 经纪公司代码
    std::string InvestorID;             // 投资者代码
    std::string InstrumentID;           // 合约代码
    char HedgeFlag;                     // 投机套保标志
    char PosiDirection;                 // 持仓多空方向
    char PositionDate;                  // 持仓日期
    int YdPosition;                     // 上日持仓
    int Position;                       // 今日持仓
    std::string TradingDay;             // 交易日
    int LongFrozen;                     // 多头冻结
    int ShortFrozen;                    // 空头冻结
    double LongFrozenAmount;            // 多头冻结金额
    double ShortFrozenAmount;           // 空头冻结金额
    int OpenVolume;                     // 开仓量
    int CloseVolume;                    // 平仓量
    double OpenAmount;                  // 开仓金额
    double CloseAmount;                 // 平仓金额
    double PositionCost;                // 持仓成本
    double PreMargin;                   // 上次占用的保证金
    double UseMargin;                   // 占用的保证金
    double FrozenMargin;                // 冻结的保证金
    double FrozenCash;                  // 冻结的资金
    double FrozenCommission;            // 冻结的手续费
    double CashIn;                      // 资金差额
    double Commission;                  // 手续费
    double CloseProfit;                 // 平仓盈亏
    double PositionProfit;              // 持仓盈亏
    double PreSettlementPrice;          // 上次结算价
    double SettlementPrice;             // 本次结算价
    int SettlementID;                   // 结算编号
    double OpenCost;                    // 开仓成本
    double ExchangeMargin;              // 交易所保证金
    int CombPosition;                   // 组合成交形成的持仓
    int CombLongFrozen;                 // 组合多头冻结
    int CombShortFrozen;                // 组合空头冻结
    double CloseProfitByDate;           // 逐日盯市平仓盈亏
    double CloseProfitByTrade;          // 逐笔对冲平仓盈亏
    int TodayPosition;                  // 今日持仓
    double MarginRateByMoney;           // 保证金率
    double MarginRateByVolume;          // 保证金率(按手数)
    int StrikeFrozen;                   // 执行冻结
    double StrikeFrozenAmount;          // 执行冻结金额
    int AbandonFrozen;                  // 放弃执行冻结
    std::string ExchangeID;             // 交易所代码
    int YdStrikeFrozen;                 // 执行冻结的昨仓
    std::string InvestUnitID;           // 投资单元代码
    double PositionCostOffset;          // 持仓成本差值
    double TasPosition;                 // tas持仓手数
    double TasCost;                     // tas持仓成本
};

class InvestorPositionConverter {
public:
    // 将单个持仓转换为CSV格式
    static std::string convertToCSV(const CTPInvestorPositionField& position);
    
    // 将持仓列表转换为CSV格式
    static std::string convertBatchToCSV(const std::vector<CTPInvestorPositionField>& positions);
    
    // 将单个持仓转换为数据库格式
    static std::vector<std::string> convertToDatabaseFormat(const CTPInvestorPositionField& position);
    
    // 将持仓列表转换为数据库格式
    static std::vector<std::vector<std::string>> convertBatchToDatabaseFormat(const std::vector<CTPInvestorPositionField>& positions);
    
    // 检查是否为CSV格式
    static bool isCSVFormat(const std::string& data);
    
    // 解析CSV格式的持仓数据
    static std::vector<CTPInvestorPositionField> parseCSV(const std::string& csvData);
    
    // 解析protobuf格式的持仓数据
    static std::vector<CTPInvestorPositionField> parseProtobufBatch(const std::string& protobufData);
};

#endif // INVESTOR_POSITION_CONVERTER_H 