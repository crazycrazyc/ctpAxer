#include "../include/InvestorPositionConverter.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstring>

std::string InvestorPositionConverter::convertToCSV(const CTPInvestorPositionField& position) {
    std::ostringstream csv;
    
    csv << position.InstrumentID << ","
        << position.BrokerID << ","
        << position.InvestorID << ","
        << position.PosiDirection << ","
        << position.HedgeFlag << ","
        << position.TradingDay << ","
        << position.Position << ","
        << position.LongFrozen << ","
        << position.ShortFrozen << ","
        << std::fixed << std::setprecision(4) << position.LongFrozenAmount << ","
        << std::fixed << std::setprecision(4) << position.ShortFrozenAmount << ","
        << position.OpenVolume << ","
        << position.CloseVolume << ","
        << std::fixed << std::setprecision(4) << position.OpenAmount << ","
        << std::fixed << std::setprecision(4) << position.CloseAmount << ","
        << std::fixed << std::setprecision(4) << position.PositionCost << ","
        << std::fixed << std::setprecision(4) << position.PreMargin << ","
        << std::fixed << std::setprecision(4) << position.UseMargin << ","
        << std::fixed << std::setprecision(4) << position.FrozenMargin << ","
        << std::fixed << std::setprecision(4) << position.FrozenCash << ","
        << std::fixed << std::setprecision(4) << position.FrozenCommission << ","
        << std::fixed << std::setprecision(4) << position.CashIn << ","
        << std::fixed << std::setprecision(4) << position.Commission << ","
        << std::fixed << std::setprecision(4) << position.CloseProfit << ","
        << std::fixed << std::setprecision(4) << position.PositionProfit << ","
        << std::fixed << std::setprecision(4) << position.PreSettlementPrice << ","
        << std::fixed << std::setprecision(4) << position.SettlementPrice << ","
        << position.SettlementID << ","
        << std::fixed << std::setprecision(4) << position.OpenCost << ","
        << std::fixed << std::setprecision(4) << position.ExchangeMargin << ","
        << position.CombPosition << ","
        << position.CombLongFrozen << ","
        << position.CombShortFrozen << ","
        << std::fixed << std::setprecision(4) << position.CloseProfitByDate << ","
        << std::fixed << std::setprecision(4) << position.CloseProfitByTrade << ","
        << position.TodayPosition << ","
        << std::fixed << std::setprecision(4) << position.MarginRateByMoney << ","
        << std::fixed << std::setprecision(4) << position.MarginRateByVolume << ","
        << position.StrikeFrozen << ","
        << std::fixed << std::setprecision(4) << position.StrikeFrozenAmount << ","
        << position.AbandonFrozen << ","
        << position.ExchangeID << ","
        << position.YdStrikeFrozen << ","
        << position.InvestUnitID << ","
        << std::fixed << std::setprecision(4) << position.PositionCostOffset << ","
        << std::fixed << std::setprecision(4) << position.TasPosition << ","
        << std::fixed << std::setprecision(4) << position.TasCost;
    
    return csv.str();
}

std::string InvestorPositionConverter::convertBatchToCSV(const std::vector<CTPInvestorPositionField>& positions) {
    std::ostringstream csv;
    
    for (const auto& position : positions) {
        csv << convertToCSV(position) << "\n";
    }
    
    return csv.str();
}

std::vector<std::string> InvestorPositionConverter::convertToDatabaseFormat(const CTPInvestorPositionField& position) {
    std::vector<std::string> fields;
    
    fields.push_back(position.InstrumentID);
    fields.push_back(position.BrokerID);
    fields.push_back(position.InvestorID);
    fields.push_back(std::string(1, position.PosiDirection));
    fields.push_back(std::string(1, position.HedgeFlag));
    fields.push_back(std::string(1, position.PositionDate));
    fields.push_back(std::to_string(position.YdPosition));
    fields.push_back(std::to_string(position.Position));
    fields.push_back(position.TradingDay);
    fields.push_back(std::to_string(position.LongFrozen));
    fields.push_back(std::to_string(position.ShortFrozen));
    fields.push_back(std::to_string(position.LongFrozenAmount));
    fields.push_back(std::to_string(position.ShortFrozenAmount));
    fields.push_back(std::to_string(position.OpenVolume));
    fields.push_back(std::to_string(position.CloseVolume));
    fields.push_back(std::to_string(position.OpenAmount));
    fields.push_back(std::to_string(position.CloseAmount));
    fields.push_back(std::to_string(position.PositionCost));
    fields.push_back(std::to_string(position.PreMargin));
    fields.push_back(std::to_string(position.UseMargin));
    fields.push_back(std::to_string(position.FrozenMargin));
    fields.push_back(std::to_string(position.FrozenCash));
    fields.push_back(std::to_string(position.FrozenCommission));
    fields.push_back(std::to_string(position.CashIn));
    fields.push_back(std::to_string(position.Commission));
    fields.push_back(std::to_string(position.CloseProfit));
    fields.push_back(std::to_string(position.PositionProfit));
    fields.push_back(std::to_string(position.PreSettlementPrice));
    fields.push_back(std::to_string(position.SettlementPrice));
    fields.push_back(std::to_string(position.SettlementID));
    fields.push_back(std::to_string(position.OpenCost));
    fields.push_back(std::to_string(position.ExchangeMargin));
    fields.push_back(std::to_string(position.CombPosition));
    fields.push_back(std::to_string(position.CombLongFrozen));
    fields.push_back(std::to_string(position.CombShortFrozen));
    fields.push_back(std::to_string(position.CloseProfitByDate));
    fields.push_back(std::to_string(position.CloseProfitByTrade));
    fields.push_back(std::to_string(position.TodayPosition));
    fields.push_back(std::to_string(position.MarginRateByMoney));
    fields.push_back(std::to_string(position.MarginRateByVolume));
    fields.push_back(std::to_string(position.StrikeFrozen));
    fields.push_back(std::to_string(position.StrikeFrozenAmount));
    fields.push_back(std::to_string(position.AbandonFrozen));
    fields.push_back(position.ExchangeID);
    fields.push_back(std::to_string(position.YdStrikeFrozen));
    fields.push_back(position.InvestUnitID);
    fields.push_back(std::to_string(position.PositionCostOffset));
    fields.push_back(std::to_string(position.TasPosition));
    fields.push_back(std::to_string(position.TasCost));
    
    return fields;
}

std::vector<std::vector<std::string>> InvestorPositionConverter::convertBatchToDatabaseFormat(const std::vector<CTPInvestorPositionField>& positions) {
    std::vector<std::vector<std::string>> result;
    
    for (const auto& position : positions) {
        result.push_back(convertToDatabaseFormat(position));
    }
    
    return result;
}

bool InvestorPositionConverter::isCSVFormat(const std::string& data) {
    if (data.empty()) return false;
    
    // 检查是否包含非ASCII字符（如果是二进制数据，不是CSV）
    for (char c : data) {
        if (c < 0 || c > 127) {
            return false;
        }
    }
    
    // 检查是否包含CSV格式的特征（逗号分隔）
    return data.find(',') != std::string::npos;
}

std::vector<CTPInvestorPositionField> InvestorPositionConverter::parseCSV(const std::string& csvData) {
    std::vector<CTPInvestorPositionField> positions;
    
    std::istringstream stream(csvData);
    std::string line;
    
    while (std::getline(stream, line)) {
        if (line.empty()) continue;
        
        CTPInvestorPositionField position;
        std::istringstream lineStream(line);
        std::string field;
        
        // 解析CSV字段
        std::vector<std::string> fields;
        while (std::getline(lineStream, field, ',')) {
            fields.push_back(field);
        }
        
        if (fields.size() >= 49) {
             // 填充结构体字段 - 根据实际CSV数据顺序
             position.InstrumentID = fields[0];           // CY509
             position.BrokerID = fields[1];               // 9999
             position.InvestorID = fields[2];             // 244158
             position.PosiDirection = fields[3].empty() ? '\0' : fields[3][0];  // 2
             position.HedgeFlag = fields[4].empty() ? '\0' : fields[4][0];      // 1
             position.PositionDate = fields[5].empty() ? '\0' : fields[5][0];   // 1
             position.YdPosition = std::stoi(fields[6]);  // 1
             position.Position = std::stoi(fields[7]);    // 1
             position.LongFrozen = std::stoi(fields[8]);  // 0
             position.ShortFrozen = std::stoi(fields[9]); // 0
             position.LongFrozenAmount = std::stod(fields[10]); // 0
             position.ShortFrozenAmount = std::stod(fields[11]); // 0
             position.OpenVolume = std::stoi(fields[12]); // 0
             position.CloseVolume = std::stoi(fields[13]); // 0
             position.OpenAmount = std::stod(fields[14]); // 0
             position.CloseAmount = std::stod(fields[15]); // 0
             position.PositionCost = std::stod(fields[16]); // 101750
             position.PreMargin = std::stod(fields[17]);   // 0
             position.UseMargin = std::stod(fields[18]);   // 25437.5
             position.FrozenMargin = std::stod(fields[19]); // 0
             position.FrozenCash = std::stod(fields[20]);  // 0
             position.FrozenCommission = std::stod(fields[21]); // 0
             position.CashIn = std::stod(fields[22]);      // 0
             position.Commission = std::stod(fields[23]);  // 0
             position.CloseProfit = std::stod(fields[24]); // 0
             position.PositionProfit = std::stod(fields[25]); // 350
             position.PreSettlementPrice = std::stod(fields[26]); // 20350
             position.SettlementPrice = std::stod(fields[27]); // 20420
             position.TradingDay = fields[28];             // 20250725
             position.SettlementID = std::stoi(fields[29]); // 1
             position.OpenCost = std::stod(fields[30]);    // 100150
             position.ExchangeMargin = std::stod(fields[31]); // 25437.5
             position.CombPosition = std::stoi(fields[32]); // 0
             position.CombLongFrozen = std::stoi(fields[33]); // 0
             position.CombShortFrozen = std::stoi(fields[34]); // 0
             position.CloseProfitByDate = std::stod(fields[35]); // 0
             position.CloseProfitByTrade = std::stod(fields[36]); // 0
             position.TodayPosition = std::stoi(fields[37]); // 0
             position.MarginRateByMoney = std::stod(fields[38]); // 0
             position.MarginRateByVolume = std::stod(fields[39]); // 0
             position.StrikeFrozen = std::stoi(fields[40]); // 0
             position.StrikeFrozenAmount = std::stod(fields[41]); // 0
             position.AbandonFrozen = std::stoi(fields[42]); // 0
             position.ExchangeID = fields[43];             // CZCE
             position.YdStrikeFrozen = std::stoi(fields[44]); // 0
             position.InvestUnitID = fields[45];           // ""
             position.PositionCostOffset = std::stod(fields[46]); // 0
             position.TasPosition = std::stod(fields[47]); // 0
             position.TasCost = std::stod(fields[48]);     // 0
            
            positions.push_back(position);
        }
    }
    
    return positions;
}

std::vector<CTPInvestorPositionField> InvestorPositionConverter::parseProtobufBatch(const std::string& protobufData) {
    std::vector<CTPInvestorPositionField> positions;
    
    try {
        // 这里应该解析protobuf数据
        // 由于protobuf文件可能还没有编译，我们先返回空结果
        // 后续需要添加protobuf解析逻辑
        
        std::cout << "Protobuf持仓数据解析功能待实现，数据长度: " << protobufData.length() << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "解析protobuf持仓数据异常: " << e.what() << std::endl;
    }
    
    return positions;
} 