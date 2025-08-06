#ifndef PROTOBUF_CONVERTER_H
#define PROTOBUF_CONVERTER_H

#include <vector>
#include <string>
#include <memory>
#include <chrono>
#include "ThostFtdcUserApiStruct.h"
#include "../proto/instrument.pb.h"
#include "../proto/investor_position.pb.h"
#include "../proto/market_data.pb.h"

class ProtobufConverter {
public:
    // ==================== 合约信息转换 ====================
    // 将CTP合约结构转换为protobuf消息
    static ctp::InstrumentMessage convertToProtobuf(const CThostFtdcInstrumentField& instrument);
    
    // 将protobuf消息转换为CTP合约结构
    static CThostFtdcInstrumentField convertFromProtobuf(const ctp::InstrumentMessage& message);
    
    // 将合约列表转换为批量protobuf消息
    static ctp::InstrumentBatchMessage convertBatchToProtobuf(const std::vector<CThostFtdcInstrumentField>& instruments);
    
    // 将批量protobuf消息转换为合约列表
    static std::vector<CThostFtdcInstrumentField> convertBatchFromProtobuf(const ctp::InstrumentBatchMessage& batchMessage);
    
    // 序列化protobuf消息为字符串
    static std::string serializeToString(const ctp::InstrumentMessage& message);
    static std::string serializeToString(const ctp::InstrumentBatchMessage& batchMessage);
    
    // 从字符串反序列化protobuf消息
    static bool deserializeFromString(const std::string& data, ctp::InstrumentMessage& message);
    static bool deserializeFromString(const std::string& data, ctp::InstrumentBatchMessage& batchMessage);
    
    // 序列化protobuf消息为字节数组
    static std::vector<uint8_t> serializeToBytes(const ctp::InstrumentMessage& message);
    static std::vector<uint8_t> serializeToBytes(const ctp::InstrumentBatchMessage& batchMessage);
    
    // 从字节数组反序列化protobuf消息
    static bool deserializeFromBytes(const std::vector<uint8_t>& data, ctp::InstrumentMessage& message);
    static bool deserializeFromBytes(const std::vector<uint8_t>& data, ctp::InstrumentBatchMessage& batchMessage);
    
    // ==================== 投资者持仓信息转换 ====================
    // 将CTP投资者持仓结构转换为protobuf消息
    static ctp::InvestorPositionMessage convertToProtobuf(const CThostFtdcInvestorPositionField& position);
    
    // 将投资者持仓列表转换为批量protobuf消息
    static ctp::InvestorPositionBatchMessage convertBatchToProtobuf(const std::vector<CThostFtdcInvestorPositionField>& positions);
    
    // 序列化投资者持仓protobuf消息为字符串
    static std::string serializeToString(const ctp::InvestorPositionMessage& message);
    static std::string serializeToString(const ctp::InvestorPositionBatchMessage& batchMessage);
    
    // ==================== 行情数据转换 (从MarketDataProtobufConverter迁移) ====================
    // 将CTP行情数据转换为protobuf格式
    static ctp::MarketDataMessage convertToProtobuf(const CThostFtdcDepthMarketDataField& marketData, 
                                                    const std::string& localTimestamp);
    
    // 序列化行情protobuf消息为字符串
    static std::string serializeToString(const ctp::MarketDataMessage& message);
    
    // ==================== 工具方法 (从MarketDataProtobufConverter迁移) ====================
    // 生成本地时间戳字符串
    static std::string generateLocalTimestamp();
    
    // 生成时间戳（毫秒）
    static int64_t generateTimestamp();
    
    // 安全的double值转换（处理DBL_MAX等特殊值）
    static double safeDouble(double value);
    
    // 安全的int值转换
    static int32_t safeInt(int value);
};

#endif // PROTOBUF_CONVERTER_H 