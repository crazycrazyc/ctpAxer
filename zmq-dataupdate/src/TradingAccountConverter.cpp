#include "../include/TradingAccountConverter.h"
#include <sstream>
#include <iostream>
#include <iomanip>

bool TradingAccountConverter::isCSVFormat(const std::string& data) {
    // 检查是否为CSV格式的资金账户数据
    // 资金账户数据通常包含特定的标识
    if (data.find("CTP_TRADING_ACCOUNT") != std::string::npos) {
        return true;
    }
    return false;
}

CTPTradingAccountField TradingAccountConverter::parseCSV(const std::string& csvData) {
    CTPTradingAccountField account = {};
    
    try {
        std::vector<std::string> fields = split(csvData, ',');
        
        if (fields.size() < 46) {  // 至少需要46个字段
            std::cerr << "资金账户数据字段不足，需要至少46个字段，实际: " << fields.size() << std::endl;
            return account;
        }
        
        // 按照CTP资金账户字段顺序解析
        // 跳过第一个字段"CTP_TRADING_ACCOUNT"标识符
        int idx = 1;
        account.BrokerID = fields[idx++];                     // 1: BrokerID
        account.AccountID = fields[idx++];                    // 2: AccountID
        account.PreMortgage = safeStringToDouble(fields[idx++]);         // 2: PreMortgage
        account.PreCredit = safeStringToDouble(fields[idx++]);           // 3: PreCredit
        account.PreDeposit = safeStringToDouble(fields[idx++]);          // 4: PreDeposit
        account.PreBalance = safeStringToDouble(fields[idx++]);          // 5: PreBalance
        account.PreMargin = safeStringToDouble(fields[idx++]);           // 6: PreMargin
        account.InterestBase = safeStringToDouble(fields[idx++]);        // 7: InterestBase
        account.Interest = safeStringToDouble(fields[idx++]);            // 8: Interest
        account.Deposit = safeStringToDouble(fields[idx++]);             // 9: Deposit
        account.Withdraw = safeStringToDouble(fields[idx++]);            // 10: Withdraw
        account.FrozenMargin = safeStringToDouble(fields[idx++]);        // 11: FrozenMargin
        account.FrozenCash = safeStringToDouble(fields[idx++]);          // 12: FrozenCash
        account.FrozenCommission = safeStringToDouble(fields[idx++]);    // 13: FrozenCommission
        account.CurrMargin = safeStringToDouble(fields[idx++]);          // 14: CurrMargin
        account.CashIn = safeStringToDouble(fields[idx++]);              // 15: CashIn
        account.Commission = safeStringToDouble(fields[idx++]);          // 16: Commission
        account.CloseProfit = safeStringToDouble(fields[idx++]);         // 17: CloseProfit
        account.PositionProfit = safeStringToDouble(fields[idx++]);      // 18: PositionProfit
        account.Balance = safeStringToDouble(fields[idx++]);             // 19: Balance
        account.Available = safeStringToDouble(fields[idx++]);           // 20: Available
        account.WithdrawQuota = safeStringToDouble(fields[idx++]);       // 21: WithdrawQuota
        account.Reserve = safeStringToDouble(fields[idx++]);             // 22: Reserve
        account.TradingDay = fields[idx++];                   // 23: TradingDay
        account.SettlementID = safeStringToInt(fields[idx++]);           // 24: SettlementID
        account.Credit = safeStringToDouble(fields[idx++]);              // 25: Credit
        account.Mortgage = safeStringToDouble(fields[idx++]);            // 26: Mortgage
        account.ExchangeMargin = safeStringToDouble(fields[idx++]);      // 27: ExchangeMargin
        account.DeliveryMargin = safeStringToDouble(fields[idx++]);      // 28: DeliveryMargin
        account.ExchangeDeliveryMargin = safeStringToDouble(fields[idx++]); // 29: ExchangeDeliveryMargin
        account.ReserveBalance = safeStringToDouble(fields[idx++]);      // 30: ReserveBalance
        account.CurrencyID = fields[idx++];                   // 31: CurrencyID
        account.PreFundMortgageIn = safeStringToDouble(fields[idx++]);   // 32: PreFundMortgageIn
        account.PreFundMortgageOut = safeStringToDouble(fields[idx++]);  // 33: PreFundMortgageOut
        account.FundMortgageIn = safeStringToDouble(fields[idx++]);      // 34: FundMortgageIn
        account.FundMortgageOut = safeStringToDouble(fields[idx++]);     // 35: FundMortgageOut
        account.FundMortgageAvailable = safeStringToDouble(fields[idx++]); // 36: FundMortgageAvailable
        account.MortgageableFund = safeStringToDouble(fields[idx++]);    // 37: MortgageableFund
        account.SpecProductMargin = safeStringToDouble(fields[idx++]);   // 38: SpecProductMargin
        account.SpecProductFrozenMargin = safeStringToDouble(fields[idx++]); // 39: SpecProductFrozenMargin
        account.SpecProductCommission = safeStringToDouble(fields[idx++]); // 40: SpecProductCommission
        account.SpecProductFrozenCommission = safeStringToDouble(fields[idx++]); // 41: SpecProductFrozenCommission
        account.SpecProductPositionProfit = safeStringToDouble(fields[idx++]); // 42: SpecProductPositionProfit
        account.SpecProductCloseProfit = safeStringToDouble(fields[idx++]); // 43: SpecProductCloseProfit
        account.SpecProductPositionProfitByAlg = safeStringToDouble(fields[idx++]); // 44: SpecProductPositionProfitByAlg
        account.SpecProductExchangeMargin = safeStringToDouble(fields[idx++]); // 45: SpecProductExchangeMargin
        
        if (fields.size() > 46) {
            account.BizType = fields[idx++];                  // 46: BizType
        }
        if (fields.size() > 47) {
            account.FrozenSwap = safeStringToDouble(fields[idx++]); // 47: FrozenSwap
        }
        if (fields.size() > 48) {
            account.RemainSwap = safeStringToDouble(fields[idx++]); // 48: RemainSwap
        }
        
    } catch (const std::exception& e) {
        std::cerr << "解析资金账户数据时出错: " << e.what() << std::endl;
    }
    
    return account;
}

std::string TradingAccountConverter::convertToDatabaseFormat(const CTPTradingAccountField& account) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(4);
    
    // 按照trading_account表的字段顺序（除了id和RecordTime）
    oss << "'" << account.BrokerID << "',"
        << "'" << account.AccountID << "',"
        << account.PreMortgage << ","
        << account.PreCredit << ","
        << account.PreDeposit << ","
        << account.PreBalance << ","
        << account.PreMargin << ","
        << account.InterestBase << ","
        << account.Interest << ","
        << account.Deposit << ","
        << account.Withdraw << ","
        << account.FrozenMargin << ","
        << account.FrozenCash << ","
        << account.FrozenCommission << ","
        << account.CurrMargin << ","
        << account.CashIn << ","
        << account.Commission << ","
        << account.CloseProfit << ","
        << account.PositionProfit << ","
        << account.Balance << ","
        << account.Available << ","
        << account.WithdrawQuota << ","
        << account.Reserve << ","
        << "'" << account.TradingDay << "',"
        << account.SettlementID << ","
        << account.Credit << ","
        << account.Mortgage << ","
        << account.ExchangeMargin << ","
        << account.DeliveryMargin << ","
        << account.ExchangeDeliveryMargin << ","
        << account.ReserveBalance << ","
        << "'" << account.CurrencyID << "',"
        << account.PreFundMortgageIn << ","
        << account.PreFundMortgageOut << ","
        << account.FundMortgageIn << ","
        << account.FundMortgageOut << ","
        << account.FundMortgageAvailable << ","
        << account.MortgageableFund << ","
        << account.SpecProductMargin << ","
        << account.SpecProductFrozenMargin << ","
        << account.SpecProductCommission << ","
        << account.SpecProductFrozenCommission << ","
        << account.SpecProductPositionProfit << ","
        << account.SpecProductCloseProfit << ","
        << account.SpecProductPositionProfitByAlg << ","
        << account.SpecProductExchangeMargin << ","
        << "'" << account.BizType << "',"
        << account.FrozenSwap << ","
        << account.RemainSwap;
    
    return oss.str();
}

std::vector<std::string> TradingAccountConverter::split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    
    return tokens;
}

double TradingAccountConverter::safeStringToDouble(const std::string& str) {
    if (str.empty() || str == "nan" || str == "inf" || str == "-inf") {
        return 0.0;
    }
    try {
        return std::stod(str);
    } catch (const std::exception&) {
        return 0.0;
    }
}

int TradingAccountConverter::safeStringToInt(const std::string& str) {
    if (str.empty()) {
        return 0;
    }
    try {
        return std::stoi(str);
    } catch (const std::exception&) {
        return 0;
    }
} 