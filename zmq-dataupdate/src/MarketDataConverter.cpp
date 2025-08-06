#include "../include/MarketDataConverter.h"
#include <sstream>
#include <iostream>
#include <iomanip>

bool MarketDataConverter::isCSVFormat(const std::string& data) {
    // 检查是否为CSV格式的行情数据
    // 行情数据通常包含特定的格式：EL/CTP_TICKER/合约/T/1 || CSV数据
    if (data.find("||") != std::string::npos && 
        data.find("EL/CTP_TICKER") != std::string::npos) {
        return true;
    }
    return false;
}

CTPMarketDataField MarketDataConverter::parseCSV(const std::string& csvData) {
    CTPMarketDataField marketData = {};
    
    try {
        // 分割 header || csv_content
        size_t separatorPos = csvData.find("||");
        if (separatorPos == std::string::npos) {
            std::cerr << "行情数据格式错误：找不到分隔符 ||" << std::endl;
            return marketData;
        }
        
        std::string csvContent = csvData.substr(separatorPos + 2);
        std::vector<std::string> fields = split(csvContent, ',');
        
        if (fields.size() < 44) {  // 至少需要44个字段
            std::cerr << "行情数据字段不足，需要至少44个字段，实际: " << fields.size() << std::endl;
            return marketData;
        }
        
        // 按照CTPQuote.cpp中OnRtnDepthMarketData的输出顺序解析
        int idx = 0;
        marketData.TradingDay = fields[idx++];          // 0: TradingDay
        marketData.InstrumentID = fields[idx++];        // 1: InstrumentID
        marketData.ExchangeID = fields[idx++];          // 2: ExchangeID
        marketData.ExchangeInstID = fields[idx++];      // 3: ExchangeInstID
        marketData.LastPrice = safeStringToDouble(fields[idx++]);           // 4: LastPrice
        marketData.PreSettlementPrice = safeStringToDouble(fields[idx++]);  // 5: PreSettlementPrice
        marketData.PreClosePrice = safeStringToDouble(fields[idx++]);       // 6: PreClosePrice
        marketData.PreOpenInterest = safeStringToDouble(fields[idx++]);     // 7: PreOpenInterest
        marketData.OpenPrice = safeStringToDouble(fields[idx++]);           // 8: OpenPrice
        marketData.HighestPrice = safeStringToDouble(fields[idx++]);        // 9: HighestPrice
        marketData.LowestPrice = safeStringToDouble(fields[idx++]);         // 10: LowestPrice
        marketData.Volume = safeStringToInt(fields[idx++]);                 // 11: Volume
        marketData.Turnover = safeStringToDouble(fields[idx++]);            // 12: Turnover
        marketData.OpenInterest = safeStringToDouble(fields[idx++]);        // 13: OpenInterest
        marketData.ClosePrice = safeStringToDouble(fields[idx++]);          // 14: ClosePrice
        marketData.SettlementPrice = safeStringToDouble(fields[idx++]);     // 15: SettlementPrice
        marketData.UpperLimitPrice = safeStringToDouble(fields[idx++]);     // 16: UpperLimitPrice
        marketData.LowerLimitPrice = safeStringToDouble(fields[idx++]);     // 17: LowerLimitPrice
        marketData.PreDelta = safeStringToDouble(fields[idx++]);            // 18: PreDelta
        marketData.CurrDelta = safeStringToDouble(fields[idx++]);           // 19: CurrDelta
        marketData.UpdateTime = fields[idx++];          // 20: UpdateTime
        marketData.UpdateMillisec = safeStringToInt(fields[idx++]);         // 21: UpdateMillisec
        marketData.BidPrice1 = safeStringToDouble(fields[idx++]);           // 22: BidPrice1
        marketData.BidVolume1 = safeStringToInt(fields[idx++]);             // 23: BidVolume1
        marketData.AskPrice1 = safeStringToDouble(fields[idx++]);           // 24: AskPrice1
        marketData.AskVolume1 = safeStringToInt(fields[idx++]);             // 25: AskVolume1
        marketData.BidPrice2 = safeStringToDouble(fields[idx++]);           // 26: BidPrice2
        marketData.BidVolume2 = safeStringToInt(fields[idx++]);             // 27: BidVolume2
        marketData.AskPrice2 = safeStringToDouble(fields[idx++]);           // 28: AskPrice2
        marketData.AskVolume2 = safeStringToInt(fields[idx++]);             // 29: AskVolume2
        marketData.BidPrice3 = safeStringToDouble(fields[idx++]);           // 30: BidPrice3
        marketData.BidVolume3 = safeStringToInt(fields[idx++]);             // 31: BidVolume3
        marketData.AskPrice3 = safeStringToDouble(fields[idx++]);           // 32: AskPrice3
        marketData.AskVolume3 = safeStringToInt(fields[idx++]);             // 33: AskVolume3
        marketData.BidPrice4 = safeStringToDouble(fields[idx++]);           // 34: BidPrice4
        marketData.BidVolume4 = safeStringToInt(fields[idx++]);             // 35: BidVolume4
        marketData.AskPrice4 = safeStringToDouble(fields[idx++]);           // 36: AskPrice4
        marketData.AskVolume4 = safeStringToInt(fields[idx++]);             // 37: AskVolume4
        marketData.BidPrice5 = safeStringToDouble(fields[idx++]);           // 38: BidPrice5
        marketData.BidVolume5 = safeStringToInt(fields[idx++]);             // 39: BidVolume5
        marketData.AskPrice5 = safeStringToDouble(fields[idx++]);           // 40: AskPrice5
        marketData.AskVolume5 = safeStringToInt(fields[idx++]);             // 41: AskVolume5
        marketData.AveragePrice = safeStringToDouble(fields[idx++]);        // 42: AveragePrice
        marketData.ActionDay = fields[idx++];           // 43: ActionDay
        
    } catch (const std::exception& e) {
        std::cerr << "解析行情数据时出错: " << e.what() << std::endl;
    }
    
    return marketData;
}

std::string MarketDataConverter::convertToDatabaseFormat(const CTPMarketDataField& marketData) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(4);
    
    // 按照market_data表的字段顺序（除了id和RecordTime）
    oss << "'" << marketData.TradingDay << "',"
        << "'" << marketData.InstrumentID << "',"
        << "'" << marketData.ExchangeID << "',"
        << "'" << marketData.ExchangeInstID << "',"
        << marketData.LastPrice << ","
        << marketData.PreSettlementPrice << ","
        << marketData.PreClosePrice << ","
        << marketData.PreOpenInterest << ","
        << marketData.OpenPrice << ","
        << marketData.HighestPrice << ","
        << marketData.LowestPrice << ","
        << marketData.Volume << ","
        << marketData.Turnover << ","
        << marketData.OpenInterest << ","
        << marketData.ClosePrice << ","
        << marketData.SettlementPrice << ","
        << marketData.UpperLimitPrice << ","
        << marketData.LowerLimitPrice << ","
        << marketData.PreDelta << ","
        << marketData.CurrDelta << ","
        << "'" << marketData.UpdateTime << "',"
        << marketData.UpdateMillisec << ","
        << marketData.BidPrice1 << ","
        << marketData.BidVolume1 << ","
        << marketData.AskPrice1 << ","
        << marketData.AskVolume1 << ","
        << marketData.BidPrice2 << ","
        << marketData.BidVolume2 << ","
        << marketData.AskPrice2 << ","
        << marketData.AskVolume2 << ","
        << marketData.BidPrice3 << ","
        << marketData.BidVolume3 << ","
        << marketData.AskPrice3 << ","
        << marketData.AskVolume3 << ","
        << marketData.BidPrice4 << ","
        << marketData.BidVolume4 << ","
        << marketData.AskPrice4 << ","
        << marketData.AskVolume4 << ","
        << marketData.BidPrice5 << ","
        << marketData.BidVolume5 << ","
        << marketData.AskPrice5 << ","
        << marketData.AskVolume5 << ","
        << marketData.AveragePrice << ","
        << "'" << marketData.ActionDay << "'";
    
    return oss.str();
}

std::vector<std::string> MarketDataConverter::split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    
    return tokens;
}

double MarketDataConverter::safeStringToDouble(const std::string& str) {
    if (str.empty() || str == "nan" || str == "inf" || str == "-inf") {
        return 0.0;
    }
    try {
        return std::stod(str);
    } catch (const std::exception&) {
        return 0.0;
    }
}

int MarketDataConverter::safeStringToInt(const std::string& str) {
    if (str.empty()) {
        return 0;
    }
    try {
        return std::stoi(str);
    } catch (const std::exception&) {
        return 0;
    }
} 