#ifndef INSTRUMENT_CONVERTER_H
#define INSTRUMENT_CONVERTER_H

#include <string>
#include <vector>
#include <memory>

// CTP合约字段结构（简化版，对应CThostFtdcInstrumentField）
struct CTPInstrumentField {
    std::string InstrumentID;           // 合约代码
    std::string ExchangeID;             // 交易所代码
    std::string InstrumentName;         // 合约名称
    std::string ExchangeInstID;         // 合约在交易所的代码
    std::string ProductID;              // 产品代码
    char ProductClass;                  // 产品类型
    int DeliveryYear;                   // 交割年份
    int DeliveryMonth;                  // 交割月份
    int MaxMarketOrderVolume;           // 市价单最大下单量
    int MinMarketOrderVolume;           // 市价单最小下单量
    int MaxLimitOrderVolume;            // 限价单最大下单量
    int MinLimitOrderVolume;            // 限价单最小下单量
    int VolumeMultiple;                 // 合约数量乘数
    double PriceTick;                   // 最小变动价位
    std::string CreateDate;             // 创建日
    std::string OpenDate;               // 上市日
    std::string ExpireDate;             // 到期日
    std::string StartDelivDate;         // 开始交割日
    std::string EndDelivDate;           // 结束交割日
    char InstLifePhase;                 // 合约生命周期状态
    bool IsTrading;                     // 当前是否交易
    char PositionType;                  // 持仓类型
    char PositionDateType;              // 持仓日期类型
    double LongMarginRatio;             // 多头保证金率
    double ShortMarginRatio;            // 空头保证金率
    char MaxMarginSideAlgorithm;        // 最大保证金算法
    std::string UnderlyingInstrID;      // 基础商品代码
    double StrikePrice;                 // 执行价
    char OptionsType;                   // 期权类型
    double UnderlyingMultiple;          // 基础商品乘数
    char CombinationType;               // 组合类型
};

class InstrumentConverter {
public:
    // 将CTP合约字段转换为CSV格式字符串
    static std::string convertToCSV(const CTPInstrumentField& instrument);
    
    // 将CTP合约字段列表转换为CSV格式字符串
    static std::string convertBatchToCSV(const std::vector<CTPInstrumentField>& instruments);
    
    // 将CTP合约字段转换为数据库插入格式
    static std::vector<std::string> convertToDatabaseFormat(const CTPInstrumentField& instrument);
    
    // 将CTP合约字段列表转换为数据库批量插入格式
    static std::vector<std::vector<std::string>> convertBatchToDatabaseFormat(const std::vector<CTPInstrumentField>& instruments);
    
    // 检查数据是否为CSV格式
    static bool isCSVFormat(const std::string& data);
    
    // 解析CSV格式数据
    static std::vector<CTPInstrumentField> parseCSV(const std::string& csvData);
    
    // 检查是否为protobuf格式
    static bool isProtobufFormat(const std::string& data);
    
    // 解析protobuf格式的合约数据
    static std::vector<CTPInstrumentField> parseProtobufBatch(const std::string& protobufData);
};

#endif // INSTRUMENT_CONVERTER_H 