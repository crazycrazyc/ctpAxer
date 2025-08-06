#ifndef MARKETDATACONVERTER_H
#define MARKETDATACONVERTER_H

#include <string>
#include <vector>

// 行情数据结构，对应CThostFtdcDepthMarketDataField
struct CTPMarketDataField {
    std::string TradingDay;         // 交易日
    std::string InstrumentID;       // 合约代码
    std::string ExchangeID;         // 交易所代码
    std::string ExchangeInstID;     // 交易所合约代码
    double LastPrice;               // 最新价
    double PreSettlementPrice;      // 上次结算价
    double PreClosePrice;           // 昨收盘
    double PreOpenInterest;         // 昨持仓量
    double OpenPrice;               // 今开盘
    double HighestPrice;            // 最高价
    double LowestPrice;             // 最低价
    int Volume;                     // 数量
    double Turnover;                // 成交金额
    double OpenInterest;            // 持仓量
    double ClosePrice;              // 今收盘
    double SettlementPrice;         // 本次结算价
    double UpperLimitPrice;         // 涨停板价
    double LowerLimitPrice;         // 跌停板价
    double PreDelta;                // 昨虚实度
    double CurrDelta;               // 今虚实度
    std::string UpdateTime;         // 最后修改时间
    int UpdateMillisec;             // 最后修改毫秒
    double BidPrice1;               // 申买价一
    int BidVolume1;                 // 申买量一
    double AskPrice1;               // 申卖价一
    int AskVolume1;                 // 申卖量一
    double BidPrice2;               // 申买价二
    int BidVolume2;                 // 申买量二
    double AskPrice2;               // 申卖价二
    int AskVolume2;                 // 申卖量二
    double BidPrice3;               // 申买价三
    int BidVolume3;                 // 申买量三
    double AskPrice3;               // 申卖价三
    int AskVolume3;                 // 申卖量三
    double BidPrice4;               // 申买价四
    int BidVolume4;                 // 申买量四
    double AskPrice4;               // 申卖价四
    int AskVolume4;                 // 申卖量四
    double BidPrice5;               // 申买价五
    int BidVolume5;                 // 申买量五
    double AskPrice5;               // 申卖价五
    int AskVolume5;                 // 申卖量五
    double AveragePrice;            // 当日均价
    std::string ActionDay;          // 业务日期
};

class MarketDataConverter {
public:
    static bool isCSVFormat(const std::string& data);
    static CTPMarketDataField parseCSV(const std::string& csvData);
    static std::string convertToDatabaseFormat(const CTPMarketDataField& marketData);
    static std::vector<std::string> split(const std::string& str, char delimiter);
    static double safeStringToDouble(const std::string& str);
    static int safeStringToInt(const std::string& str);
};

#endif // MARKETDATACONVERTER_H 