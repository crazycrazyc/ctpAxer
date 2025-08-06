#include "../include/InstrumentConverter.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstring>

std::string InstrumentConverter::convertToCSV(const CTPInstrumentField& instrument) {
    std::ostringstream csv;
    
    csv << instrument.InstrumentID << ","
        << instrument.ExchangeID << ","
        << instrument.InstrumentName << ","
        << instrument.ExchangeInstID << ","
        << instrument.ProductID << ","
        << instrument.ProductClass << ","
        << instrument.DeliveryYear << ","
        << instrument.DeliveryMonth << ","
        << instrument.MaxMarketOrderVolume << ","
        << instrument.MinMarketOrderVolume << ","
        << instrument.MaxLimitOrderVolume << ","
        << instrument.MinLimitOrderVolume << ","
        << instrument.VolumeMultiple << ","
        << std::fixed << std::setprecision(8) << instrument.PriceTick << ","
        << instrument.CreateDate << ","
        << instrument.OpenDate << ","
        << instrument.ExpireDate << ","
        << instrument.StartDelivDate << ","
        << instrument.EndDelivDate << ","
        << instrument.InstLifePhase << ","
        << (instrument.IsTrading ? "1" : "0") << ","
        << instrument.PositionType << ","
        << instrument.PositionDateType << ","
        << std::fixed << std::setprecision(4) << instrument.LongMarginRatio << ","
        << std::fixed << std::setprecision(4) << instrument.ShortMarginRatio << ","
        << instrument.MaxMarginSideAlgorithm << ","
        << instrument.UnderlyingInstrID << ","
        << std::fixed << std::setprecision(4) << instrument.StrikePrice << ","
        << instrument.OptionsType << ","
        << std::fixed << std::setprecision(4) << instrument.UnderlyingMultiple << ","
        << instrument.CombinationType;
    
    return csv.str();
}

std::string InstrumentConverter::convertBatchToCSV(const std::vector<CTPInstrumentField>& instruments) {
    std::ostringstream csv;
    
    for (const auto& instrument : instruments) {
        csv << convertToCSV(instrument) << "\n";
    }
    
    return csv.str();
}

std::vector<std::string> InstrumentConverter::convertToDatabaseFormat(const CTPInstrumentField& instrument) {
    std::vector<std::string> fields;
    
    fields.push_back(instrument.InstrumentID);
    fields.push_back(instrument.ExchangeID);
    fields.push_back(instrument.InstrumentName);
    fields.push_back(instrument.ExchangeInstID);
    fields.push_back(instrument.ProductID);
    fields.push_back(std::string(1, instrument.ProductClass));
    fields.push_back(std::to_string(instrument.DeliveryYear));
    fields.push_back(std::to_string(instrument.DeliveryMonth));
    fields.push_back(std::to_string(instrument.MaxMarketOrderVolume));
    fields.push_back(std::to_string(instrument.MinMarketOrderVolume));
    fields.push_back(std::to_string(instrument.MaxLimitOrderVolume));
    fields.push_back(std::to_string(instrument.MinLimitOrderVolume));
    fields.push_back(std::to_string(instrument.VolumeMultiple));
    fields.push_back(std::to_string(instrument.PriceTick));
    fields.push_back(instrument.CreateDate);
    fields.push_back(instrument.OpenDate);
    fields.push_back(instrument.ExpireDate);
    fields.push_back(instrument.StartDelivDate);
    fields.push_back(instrument.EndDelivDate);
    fields.push_back(std::string(1, instrument.InstLifePhase));
    fields.push_back(instrument.IsTrading ? "1" : "0");
    fields.push_back(std::string(1, instrument.PositionType));
    fields.push_back(std::string(1, instrument.PositionDateType));
    fields.push_back(std::to_string(instrument.LongMarginRatio));
    fields.push_back(std::to_string(instrument.ShortMarginRatio));
    fields.push_back(std::string(1, instrument.MaxMarginSideAlgorithm));
    fields.push_back(instrument.UnderlyingInstrID);
    fields.push_back(std::to_string(instrument.StrikePrice));
    fields.push_back(std::string(1, instrument.OptionsType));
    fields.push_back(std::to_string(instrument.UnderlyingMultiple));
    fields.push_back(std::string(1, instrument.CombinationType));
    
    return fields;
}

std::vector<std::vector<std::string>> InstrumentConverter::convertBatchToDatabaseFormat(const std::vector<CTPInstrumentField>& instruments) {
    std::vector<std::vector<std::string>> result;
    
    for (const auto& instrument : instruments) {
        result.push_back(convertToDatabaseFormat(instrument));
    }
    
    return result;
}

bool InstrumentConverter::isCSVFormat(const std::string& data) {
    if (data.empty()) return false;
    
    // 检查是否包含CSV格式的特征（逗号分隔）
    return data.find(',') != std::string::npos;
}

std::vector<CTPInstrumentField> InstrumentConverter::parseCSV(const std::string& csvData) {
    std::vector<CTPInstrumentField> instruments;
    
    std::istringstream stream(csvData);
    std::string line;
    
    while (std::getline(stream, line)) {
        if (line.empty()) continue;
        
        CTPInstrumentField instrument;
        std::istringstream lineStream(line);
        std::string field;
        
        // 解析CSV字段
        std::vector<std::string> fields;
        while (std::getline(lineStream, field, ',')) {
            fields.push_back(field);
        }
        
        if (fields.size() >= 31) {
            // 填充结构体字段
            instrument.InstrumentID = fields[0];
            instrument.ExchangeID = fields[1];
            instrument.InstrumentName = fields[2];
            instrument.ExchangeInstID = fields[3];
            instrument.ProductID = fields[4];
            instrument.ProductClass = fields[5].empty() ? '\0' : fields[5][0];
            instrument.DeliveryYear = std::stoi(fields[6]);
            instrument.DeliveryMonth = std::stoi(fields[7]);
            instrument.MaxMarketOrderVolume = std::stoi(fields[8]);
            instrument.MinMarketOrderVolume = std::stoi(fields[9]);
            instrument.MaxLimitOrderVolume = std::stoi(fields[10]);
            instrument.MinLimitOrderVolume = std::stoi(fields[11]);
            instrument.VolumeMultiple = std::stoi(fields[12]);
            instrument.PriceTick = std::stod(fields[13]);
            instrument.CreateDate = fields[14];
            instrument.OpenDate = fields[15];
            instrument.ExpireDate = fields[16];
            instrument.StartDelivDate = fields[17];
            instrument.EndDelivDate = fields[18];
            instrument.InstLifePhase = fields[19].empty() ? '\0' : fields[19][0];
            instrument.IsTrading = (fields[20] == "1");
            instrument.PositionType = fields[21].empty() ? '\0' : fields[21][0];
            instrument.PositionDateType = fields[22].empty() ? '\0' : fields[22][0];
            instrument.LongMarginRatio = std::stod(fields[23]);
            instrument.ShortMarginRatio = std::stod(fields[24]);
            instrument.MaxMarginSideAlgorithm = fields[25].empty() ? '\0' : fields[25][0];
            instrument.UnderlyingInstrID = fields[26];
            instrument.StrikePrice = std::stod(fields[27]);
            instrument.OptionsType = fields[28].empty() ? '\0' : fields[28][0];
            instrument.UnderlyingMultiple = std::stod(fields[29]);
            instrument.CombinationType = fields[30].empty() ? '\0' : fields[30][0];
            
            instruments.push_back(instrument);
        }
    }
    
    return instruments;
}

bool InstrumentConverter::isProtobufFormat(const std::string& data) {
    // 简单的protobuf格式检测：检查是否包含二进制数据特征
    if (data.empty()) return false;
    
    // 检查是否包含非ASCII字符（protobuf通常是二进制）
    bool hasNonAscii = false;
    for (char c : data) {
        if (c < 0 || c > 127) {
            hasNonAscii = true;
            break;
        }
    }
    
    // 如果包含非ASCII字符，很可能是protobuf
    if (hasNonAscii) {
        return true;
    }
    
    // 检查是否以protobuf特征开头（字段号通常很小）
    if (data.length() > 0) {
        unsigned char firstByte = static_cast<unsigned char>(data[0]);
        // protobuf字段号通常很小（1-15），且wire type为0-5
        if (firstByte <= 0x7F) {
            return true;
        }
    }
    
    return false;
}

std::vector<CTPInstrumentField> InstrumentConverter::parseProtobufBatch(const std::string& protobufData) {
    std::vector<CTPInstrumentField> instruments;
    
    try {
        // 这里应该解析protobuf数据
        // 由于protobuf文件可能还没有编译，我们先返回空结果
        // 后续需要添加protobuf解析逻辑
        
        std::cout << "Protobuf合约数据解析功能待实现，数据长度: " << protobufData.length() << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "解析protobuf合约数据异常: " << e.what() << std::endl;
    }
    
    return instruments;
} 