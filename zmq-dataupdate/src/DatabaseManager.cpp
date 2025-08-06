#include "../include/DatabaseManager.h"
#include <iostream>
#include <stdexcept>
#include <sstream>

DatabaseManager::DatabaseManager(const std::string& host, int port, 
                               const std::string& user, const std::string& password, 
                               const std::string& database)
    : host(host), port(port), user(user), password(password), database(database), connected(false) {
    try {
        driver = std::unique_ptr<sql::mysql::MySQL_Driver>(sql::mysql::get_mysql_driver_instance());
    } catch (const sql::SQLException& e) {
        std::cerr << "MySQL驱动初始化失败: " << e.what() << std::endl;
    }
}

DatabaseManager::~DatabaseManager() {
    disconnect();
}

bool DatabaseManager::connect() {
    if (!driver) {
        std::cerr << "MySQL驱动未初始化" << std::endl;
        return false;
    }

    try {
        std::string url = "tcp://" + host + ":" + std::to_string(port);
        connection = std::unique_ptr<sql::Connection>(driver->connect(url, user, password));
        
        if (connection) {
            connection->setSchema(database);
            connected = true;
            std::cout << "数据库连接成功" << std::endl;
            return true;
        }
    } catch (const sql::SQLException& e) {
        std::cerr << "数据库连接失败: " << e.what() << std::endl;
        connected = false;
    }
    
    return false;
}

void DatabaseManager::disconnect() {
    if (connection) {
        connection->close();
        connection.reset();
    }
    connected = false;
}

bool DatabaseManager::insertMessage(const std::string& messageType, const std::string& messageContent) {
    if (!connected) {
        std::cerr << "数据库未连接" << std::endl;
        return false;
    }

    try {
        std::unique_ptr<sql::Statement> stmt(connection->createStatement());
        std::string sql = "INSERT INTO messages (message_type, message_content, created_at) VALUES (?, ?, NOW())";
        
        std::unique_ptr<sql::PreparedStatement> pstmt(connection->prepareStatement(sql));
        pstmt->setString(1, messageType);
        pstmt->setString(2, messageContent);
        
        pstmt->executeUpdate();
        return true;
    } catch (const sql::SQLException& e) {
        std::cerr << "插入消息失败: " << e.what() << std::endl;
        return false;
    }
}

bool DatabaseManager::insertTickData(const std::string& symbol, double price, int volume, const std::string& timestamp) {
    if (!connected) {
        std::cerr << "数据库未连接" << std::endl;
        return false;
    }

    try {
        std::string sql = "INSERT INTO tick_data (symbol, price, volume, timestamp, created_at) VALUES (?, ?, ?, ?, NOW())";
        
        std::unique_ptr<sql::PreparedStatement> pstmt(connection->prepareStatement(sql));
        pstmt->setString(1, symbol);
        pstmt->setDouble(2, price);
        pstmt->setInt(3, volume);
        pstmt->setString(4, timestamp);
        
        pstmt->executeUpdate();
        return true;
    } catch (const sql::SQLException& e) {
        std::cerr << "插入Tick数据失败: " << e.what() << std::endl;
        return false;
    }
}

std::unique_ptr<sql::ResultSet> DatabaseManager::query(const std::string& sql) {
    if (!connected) {
        std::cerr << "数据库未连接" << std::endl;
        return nullptr;
    }

    try {
        std::unique_ptr<sql::Statement> stmt(connection->createStatement());
        return std::unique_ptr<sql::ResultSet>(stmt->executeQuery(sql));
    } catch (const sql::SQLException& e) {
        std::cerr << "查询失败: " << e.what() << std::endl;
        return nullptr;
    }
}

bool DatabaseManager::execute(const std::string& sql) {
    if (!connected) {
        std::cerr << "数据库未连接" << std::endl;
        return false;
    }

    try {
        std::unique_ptr<sql::Statement> stmt(connection->createStatement());
        stmt->execute(sql);
        return true;
    } catch (const sql::SQLException& e) {
        std::cerr << "执行SQL失败: " << e.what() << std::endl;
        return false;
    }
}

bool DatabaseManager::insertInstrument(const std::string& instrumentId, const std::string& exchangeId, 
                                     const std::string& instrumentName, const std::string& exchangeInstId,
                                     const std::string& productId, char productClass, int deliveryYear, 
                                     int deliveryMonth, int maxMarketOrderVolume, int minMarketOrderVolume,
                                     int maxLimitOrderVolume, int minLimitOrderVolume, int volumeMultiple,
                                     double priceTick, const std::string& createDate, const std::string& openDate,
                                     const std::string& expireDate, const std::string& startDelivDate,
                                     const std::string& endDelivDate, char instLifePhase, bool isTrading,
                                     char positionType, char positionDateType, double longMarginRatio,
                                     double shortMarginRatio, char maxMarginSideAlgorithm,
                                     const std::string& underlyingInstrId, double strikePrice, char optionsType,
                                     double underlyingMultiple, char combinationType) {
    // 检查连接状态，如果断开则尝试重连
    if (!connected || !connection) {
        std::cerr << "数据库连接已断开，尝试重连..." << std::endl;
        if (!connect()) {
            std::cerr << "数据库重连失败" << std::endl;
            return false;
        }
    }

    try {
        std::string sql = "INSERT INTO test_update_instrument ("
                         "InstrumentID, ExchangeID, InstrumentName, ExchangeInstID, ProductID, "
                         "ProductClass, DeliveryYear, DeliveryMonth, MaxMarketOrderVolume, "
                         "MinMarketOrderVolume, MaxLimitOrderVolume, MinLimitOrderVolume, "
                         "VolumeMultiple, PriceTick, CreateDate, OpenDate, ExpireDate, "
                         "StartDelivDate, EndDelivDate, InstLifePhase, IsTrading, PositionType, "
                         "PositionDateType, LongMarginRatio, ShortMarginRatio, MaxMarginSideAlgorithm, "
                         "UnderlyingInstrID, StrikePrice, OptionsType, UnderlyingMultiple, CombinationType) "
                         "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?) "
                         "ON DUPLICATE KEY UPDATE "
                         "InstrumentName=VALUES(InstrumentName), ExchangeInstID=VALUES(ExchangeInstID), "
                         "ProductID=VALUES(ProductID), ProductClass=VALUES(ProductClass), "
                         "DeliveryYear=VALUES(DeliveryYear), DeliveryMonth=VALUES(DeliveryMonth), "
                         "MaxMarketOrderVolume=VALUES(MaxMarketOrderVolume), "
                         "MinMarketOrderVolume=VALUES(MinMarketOrderVolume), "
                         "MaxLimitOrderVolume=VALUES(MaxLimitOrderVolume), "
                         "MinLimitOrderVolume=VALUES(MinLimitOrderVolume), "
                         "VolumeMultiple=VALUES(VolumeMultiple), PriceTick=VALUES(PriceTick), "
                         "CreateDate=VALUES(CreateDate), OpenDate=VALUES(OpenDate), "
                         "ExpireDate=VALUES(ExpireDate), StartDelivDate=VALUES(StartDelivDate), "
                         "EndDelivDate=VALUES(EndDelivDate), InstLifePhase=VALUES(InstLifePhase), "
                         "IsTrading=VALUES(IsTrading), PositionType=VALUES(PositionType), "
                         "PositionDateType=VALUES(PositionDateType), "
                         "LongMarginRatio=VALUES(LongMarginRatio), "
                         "ShortMarginRatio=VALUES(ShortMarginRatio), "
                         "MaxMarginSideAlgorithm=VALUES(MaxMarginSideAlgorithm), "
                         "UnderlyingInstrID=VALUES(UnderlyingInstrID), "
                         "StrikePrice=VALUES(StrikePrice), OptionsType=VALUES(OptionsType), "
                         "UnderlyingMultiple=VALUES(UnderlyingMultiple), "
                         "CombinationType=VALUES(CombinationType)";
        
        std::unique_ptr<sql::PreparedStatement> pstmt(connection->prepareStatement(sql));
        
        int paramIndex = 1;
        pstmt->setString(paramIndex++, instrumentId);
        pstmt->setString(paramIndex++, exchangeId);
        pstmt->setString(paramIndex++, instrumentName);
        pstmt->setString(paramIndex++, exchangeInstId);
        pstmt->setString(paramIndex++, productId);
        pstmt->setString(paramIndex++, std::string(1, productClass));
        pstmt->setInt(paramIndex++, deliveryYear);
        pstmt->setInt(paramIndex++, deliveryMonth);
        pstmt->setInt(paramIndex++, maxMarketOrderVolume);
        pstmt->setInt(paramIndex++, minMarketOrderVolume);
        pstmt->setInt(paramIndex++, maxLimitOrderVolume);
        pstmt->setInt(paramIndex++, minLimitOrderVolume);
        pstmt->setInt(paramIndex++, volumeMultiple);
        pstmt->setDouble(paramIndex++, priceTick);
        pstmt->setString(paramIndex++, createDate);
        pstmt->setString(paramIndex++, openDate);
        pstmt->setString(paramIndex++, expireDate);
        pstmt->setString(paramIndex++, startDelivDate);
        pstmt->setString(paramIndex++, endDelivDate);
        pstmt->setString(paramIndex++, std::string(1, instLifePhase));
        pstmt->setBoolean(paramIndex++, isTrading);
        pstmt->setString(paramIndex++, std::string(1, positionType));
        pstmt->setString(paramIndex++, std::string(1, positionDateType));
        pstmt->setDouble(paramIndex++, longMarginRatio);
        pstmt->setDouble(paramIndex++, shortMarginRatio);
        pstmt->setString(paramIndex++, std::string(1, maxMarginSideAlgorithm));
        pstmt->setString(paramIndex++, underlyingInstrId);
        pstmt->setDouble(paramIndex++, strikePrice);
        pstmt->setString(paramIndex++, std::string(1, optionsType));
        pstmt->setDouble(paramIndex++, underlyingMultiple);
        pstmt->setString(paramIndex++, std::string(1, combinationType));
        
        pstmt->executeUpdate();
        return true;
    } catch (const sql::SQLException& e) {
        std::cerr << "插入合约信息失败: " << e.what() << std::endl;
        return false;
    }
}

bool DatabaseManager::insertInstrumentBatch(const std::vector<std::vector<std::string>>& instruments) {
    // 检查连接状态，如果断开则尝试重连
    if (!connected || !connection) {
        std::cerr << "数据库连接已断开，尝试重连..." << std::endl;
        if (!connect()) {
            std::cerr << "数据库重连失败" << std::endl;
            return false;
        }
    }

    try {
        // 开始事务
        connection->setAutoCommit(false);
        
        std::string sql = "INSERT INTO test_update_instrument ("
                         "InstrumentID, ExchangeID, InstrumentName, ExchangeInstID, ProductID, "
                         "ProductClass, DeliveryYear, DeliveryMonth, MaxMarketOrderVolume, "
                         "MinMarketOrderVolume, MaxLimitOrderVolume, MinLimitOrderVolume, "
                         "VolumeMultiple, PriceTick, CreateDate, OpenDate, ExpireDate, "
                         "StartDelivDate, EndDelivDate, InstLifePhase, IsTrading, PositionType, "
                         "PositionDateType, LongMarginRatio, ShortMarginRatio, MaxMarginSideAlgorithm, "
                         "UnderlyingInstrID, StrikePrice, OptionsType, UnderlyingMultiple, CombinationType) "
                         "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?) "
                         "ON DUPLICATE KEY UPDATE "
                         "InstrumentName=VALUES(InstrumentName), ExchangeInstID=VALUES(ExchangeInstID), "
                         "ProductID=VALUES(ProductID), ProductClass=VALUES(ProductClass), "
                         "DeliveryYear=VALUES(DeliveryYear), DeliveryMonth=VALUES(DeliveryMonth), "
                         "MaxMarketOrderVolume=VALUES(MaxMarketOrderVolume), "
                         "MinMarketOrderVolume=VALUES(MinMarketOrderVolume), "
                         "MaxLimitOrderVolume=VALUES(MaxLimitOrderVolume), "
                         "MinLimitOrderVolume=VALUES(MinLimitOrderVolume), "
                         "VolumeMultiple=VALUES(VolumeMultiple), PriceTick=VALUES(PriceTick), "
                         "CreateDate=VALUES(CreateDate), OpenDate=VALUES(OpenDate), "
                         "ExpireDate=VALUES(ExpireDate), StartDelivDate=VALUES(StartDelivDate), "
                         "EndDelivDate=VALUES(EndDelivDate), InstLifePhase=VALUES(InstLifePhase), "
                         "IsTrading=VALUES(IsTrading), PositionType=VALUES(PositionType), "
                         "PositionDateType=VALUES(PositionDateType), "
                         "LongMarginRatio=VALUES(LongMarginRatio), "
                         "ShortMarginRatio=VALUES(ShortMarginRatio), "
                         "MaxMarginSideAlgorithm=VALUES(MaxMarginSideAlgorithm), "
                         "UnderlyingInstrID=VALUES(UnderlyingInstrID), "
                         "StrikePrice=VALUES(StrikePrice), OptionsType=VALUES(OptionsType), "
                         "UnderlyingMultiple=VALUES(UnderlyingMultiple), "
                         "CombinationType=VALUES(CombinationType)";
        
        std::unique_ptr<sql::PreparedStatement> pstmt(connection->prepareStatement(sql));
        
        int successCount = 0;
        for (const auto& instrument : instruments) {
            if (instrument.size() < 30) {
                std::cerr << "合约数据字段不足，跳过" << std::endl;
                continue;
            }
            
            try {
                int paramIndex = 1;
                pstmt->setString(paramIndex++, instrument[0]);  // instrument_id
                pstmt->setString(paramIndex++, instrument[1]);  // exchange_id
                pstmt->setString(paramIndex++, instrument[2]);  // instrument_name
                pstmt->setString(paramIndex++, instrument[3]);  // exchange_inst_id
                pstmt->setString(paramIndex++, instrument[4]);  // product_id
                pstmt->setString(paramIndex++, instrument[5]);  // product_class
                pstmt->setInt(paramIndex++, std::stoi(instrument[6]));      // delivery_year
                pstmt->setInt(paramIndex++, std::stoi(instrument[7]));      // delivery_month
                pstmt->setInt(paramIndex++, std::stoi(instrument[8]));      // max_market_order_volume
                pstmt->setInt(paramIndex++, std::stoi(instrument[9]));      // min_market_order_volume
                pstmt->setInt(paramIndex++, std::stoi(instrument[10]));     // max_limit_order_volume
                pstmt->setInt(paramIndex++, std::stoi(instrument[11]));     // min_limit_order_volume
                pstmt->setInt(paramIndex++, std::stoi(instrument[12]));     // volume_multiple
                pstmt->setDouble(paramIndex++, std::stod(instrument[13]));  // price_tick
                pstmt->setString(paramIndex++, instrument[14]); // create_date
                pstmt->setString(paramIndex++, instrument[15]); // open_date
                pstmt->setString(paramIndex++, instrument[16]); // expire_date
                pstmt->setString(paramIndex++, instrument[17]); // start_deliv_date
                pstmt->setString(paramIndex++, instrument[18]); // end_deliv_date
                pstmt->setString(paramIndex++, instrument[19]); // inst_life_phase
                pstmt->setBoolean(paramIndex++, instrument[20] == "1"); // is_trading
                pstmt->setString(paramIndex++, instrument[21]); // position_type
                pstmt->setString(paramIndex++, instrument[22]); // position_date_type
                pstmt->setDouble(paramIndex++, std::stod(instrument[23])); // long_margin_ratio
                pstmt->setDouble(paramIndex++, std::stod(instrument[24])); // short_margin_ratio
                pstmt->setString(paramIndex++, instrument[25]); // max_margin_side_algorithm
                pstmt->setString(paramIndex++, instrument[26]); // underlying_instr_id
                pstmt->setDouble(paramIndex++, std::stod(instrument[27])); // strike_price
                pstmt->setString(paramIndex++, instrument[28]); // options_type
                pstmt->setDouble(paramIndex++, std::stod(instrument[29])); // underlying_multiple
                pstmt->setString(paramIndex++, instrument[30]); // combination_type
                
                pstmt->executeUpdate();
                successCount++;
            } catch (const std::exception& e) {
                std::cerr << "插入合约数据失败: " << e.what() << std::endl;
            }
        }
        
        // 提交事务
        connection->commit();
        connection->setAutoCommit(true);
        
        std::cout << "批量插入合约完成，成功: " << successCount << "/" << instruments.size() << std::endl;
        return successCount > 0;
        
    } catch (const sql::SQLException& e) {
        // 回滚事务
        try {
            connection->rollback();
            connection->setAutoCommit(true);
        } catch (...) {}
        
        std::cerr << "批量插入合约失败: " << e.what() << std::endl;
        return false;
    }
}

bool DatabaseManager::insertInvestorPosition(const std::string& instrumentId, const std::string& brokerId,
                                           const std::string& investorId, char posiDirection, char hedgeFlag,
                                           char positionDate, int ydPosition, int position, int longFrozen,
                                           int shortFrozen, double longFrozenAmount, double shortFrozenAmount,
                                           int openVolume, int closeVolume, double openAmount, double closeAmount,
                                           double positionCost, double preMargin, double useMargin, double frozenMargin,
                                           double frozenCash, double frozenCommission, double cashIn, double commission,
                                           double closeProfit, double positionProfit, double preSettlementPrice,
                                           double settlementPrice, const std::string& tradingDay, int settlementId,
                                           double openCost, double exchangeMargin, int combPosition, int combLongFrozen,
                                           int combShortFrozen, double closeProfitByDate, double closeProfitByTrade,
                                           int todayPosition, double marginRateByMoney, double marginRateByVolume,
                                           int strikeFrozen, double strikeFrozenAmount, int abandonFrozen,
                                           const std::string& exchangeId, int ydStrikeFrozen,
                                           const std::string& investUnitId, double positionCostOffset,
                                           double tasPosition, double tasCost) {
    if (!connected) {
        std::cerr << "数据库未连接" << std::endl;
        return false;
    }

    try {
        std::string sql = "INSERT INTO investor_positions ("
                         "BrokerID, InvestorID, InstrumentID, HedgeFlag, PosiDirection, TradingDay, "
                         "Position, LongFrozen, ShortFrozen, LongFrozenAmount, ShortFrozenAmount, "
                         "OpenVolume, CloseVolume, OpenAmount, CloseAmount, PositionCost, PreMargin, "
                         "UseMargin, FrozenMargin, FrozenCash, FrozenCommission, CashIn, Commission, "
                         "CloseProfit, PositionProfit, PreSettlementPrice, SettlementPrice, SettlementID, "
                         "OpenCost, ExchangeMargin, CombPosition, CombLongFrozen, CombShortFrozen, "
                         "CloseProfitByDate, CloseProfitByTrade, TodayPosition, MarginRateByMoney, "
                         "MarginRateByVolume, StrikeFrozen, StrikeFrozenAmount, AbandonFrozen, "
                         "ExchangeID, YdStrikeFrozen, InvestUnitID, PositionCostOffset, TasPosition, TasCost) "
                         "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?) "
                         "ON DUPLICATE KEY UPDATE "
                         "Position=VALUES(Position), LongFrozen=VALUES(LongFrozen), "
                         "ShortFrozen=VALUES(ShortFrozen), LongFrozenAmount=VALUES(LongFrozenAmount), "
                         "ShortFrozenAmount=VALUES(ShortFrozenAmount), OpenVolume=VALUES(OpenVolume), "
                         "CloseVolume=VALUES(CloseVolume), OpenAmount=VALUES(OpenAmount), "
                         "CloseAmount=VALUES(CloseAmount), PositionCost=VALUES(PositionCost), "
                         "PreMargin=VALUES(PreMargin), UseMargin=VALUES(UseMargin), "
                         "FrozenMargin=VALUES(FrozenMargin), FrozenCash=VALUES(FrozenCash), "
                         "FrozenCommission=VALUES(FrozenCommission), CashIn=VALUES(CashIn), "
                         "Commission=VALUES(Commission), CloseProfit=VALUES(CloseProfit), "
                         "PositionProfit=VALUES(PositionProfit), PreSettlementPrice=VALUES(PreSettlementPrice), "
                         "SettlementPrice=VALUES(SettlementPrice), SettlementID=VALUES(SettlementID), "
                         "OpenCost=VALUES(OpenCost), ExchangeMargin=VALUES(ExchangeMargin), "
                         "CombPosition=VALUES(CombPosition), CombLongFrozen=VALUES(CombLongFrozen), "
                         "CombShortFrozen=VALUES(CombShortFrozen), CloseProfitByDate=VALUES(CloseProfitByDate), "
                         "CloseProfitByTrade=VALUES(CloseProfitByTrade), TodayPosition=VALUES(TodayPosition), "
                         "MarginRateByMoney=VALUES(MarginRateByMoney), MarginRateByVolume=VALUES(MarginRateByVolume), "
                         "StrikeFrozen=VALUES(StrikeFrozen), StrikeFrozenAmount=VALUES(StrikeFrozenAmount), "
                         "AbandonFrozen=VALUES(AbandonFrozen), YdStrikeFrozen=VALUES(YdStrikeFrozen), "
                         "PositionCostOffset=VALUES(PositionCostOffset), TasPosition=VALUES(TasPosition), "
                         "TasCost=VALUES(TasCost)";

        std::unique_ptr<sql::PreparedStatement> pstmt(connection->prepareStatement(sql));
        
        int paramIndex = 1;
        pstmt->setString(paramIndex++, instrumentId);
        pstmt->setString(paramIndex++, brokerId);
        pstmt->setString(paramIndex++, investorId);
        pstmt->setString(paramIndex++, std::string(1, posiDirection));
        pstmt->setString(paramIndex++, std::string(1, hedgeFlag));
        pstmt->setString(paramIndex++, std::string(1, positionDate));
        pstmt->setInt(paramIndex++, ydPosition);
        pstmt->setInt(paramIndex++, position);
        pstmt->setInt(paramIndex++, longFrozen);
        pstmt->setInt(paramIndex++, shortFrozen);
        pstmt->setDouble(paramIndex++, longFrozenAmount);
        pstmt->setDouble(paramIndex++, shortFrozenAmount);
        pstmt->setInt(paramIndex++, openVolume);
        pstmt->setInt(paramIndex++, closeVolume);
        pstmt->setDouble(paramIndex++, openAmount);
        pstmt->setDouble(paramIndex++, closeAmount);
        pstmt->setDouble(paramIndex++, positionCost);
        pstmt->setDouble(paramIndex++, preMargin);
        pstmt->setDouble(paramIndex++, useMargin);
        pstmt->setDouble(paramIndex++, frozenMargin);
        pstmt->setDouble(paramIndex++, frozenCash);
        pstmt->setDouble(paramIndex++, frozenCommission);
        pstmt->setDouble(paramIndex++, cashIn);
        pstmt->setDouble(paramIndex++, commission);
        pstmt->setDouble(paramIndex++, closeProfit);
        pstmt->setDouble(paramIndex++, positionProfit);
        pstmt->setDouble(paramIndex++, preSettlementPrice);
        pstmt->setDouble(paramIndex++, settlementPrice);
        pstmt->setString(paramIndex++, tradingDay);
        pstmt->setInt(paramIndex++, settlementId);
        pstmt->setDouble(paramIndex++, openCost);
        pstmt->setDouble(paramIndex++, exchangeMargin);
        pstmt->setInt(paramIndex++, combPosition);
        pstmt->setInt(paramIndex++, combLongFrozen);
        pstmt->setInt(paramIndex++, combShortFrozen);
        pstmt->setDouble(paramIndex++, closeProfitByDate);
        pstmt->setDouble(paramIndex++, closeProfitByTrade);
        pstmt->setInt(paramIndex++, todayPosition);
        pstmt->setDouble(paramIndex++, marginRateByMoney);
        pstmt->setDouble(paramIndex++, marginRateByVolume);
        pstmt->setInt(paramIndex++, strikeFrozen);
        pstmt->setDouble(paramIndex++, strikeFrozenAmount);
        pstmt->setInt(paramIndex++, abandonFrozen);
        pstmt->setString(paramIndex++, exchangeId);
        pstmt->setInt(paramIndex++, ydStrikeFrozen);
        pstmt->setString(paramIndex++, investUnitId);
        pstmt->setDouble(paramIndex++, positionCostOffset);
        pstmt->setDouble(paramIndex++, tasPosition);
        pstmt->setDouble(paramIndex++, tasCost);
        
        pstmt->executeUpdate();
        return true;
        
    } catch (const sql::SQLException& e) {
        std::cerr << "插入投资者持仓失败: " << e.what() << std::endl;
        return false;
    }
}

bool DatabaseManager::insertInvestorPositionBatch(const std::vector<std::vector<std::string>>& positions) {
    if (!connected) {
        std::cerr << "数据库未连接" << std::endl;
        return false;
    }

    try {
        // 开始事务
        connection->setAutoCommit(false);
        
        std::string sql = "INSERT INTO investor_positions ("
                         "InstrumentID, BrokerID, InvestorID, PosiDirection, HedgeFlag, PositionDate, "
                         "YdPosition, Position, LongFrozen, ShortFrozen, LongFrozenAmount, ShortFrozenAmount, "
                         "OpenVolume, CloseVolume, OpenAmount, CloseAmount, PositionCost, PreMargin, "
                         "UseMargin, FrozenMargin, FrozenCash, FrozenCommission, CashIn, Commission, "
                         "CloseProfit, PositionProfit, PreSettlementPrice, SettlementPrice, TradingDay, "
                         "SettlementID, OpenCost, ExchangeMargin, CombPosition, CombLongFrozen, CombShortFrozen, "
                         "CloseProfitByDate, CloseProfitByTrade, TodayPosition, MarginRateByMoney, "
                         "MarginRateByVolume, StrikeFrozen, StrikeFrozenAmount, AbandonFrozen, "
                         "ExchangeID, YdStrikeFrozen, InvestUnitID, PositionCostOffset, TasPosition, TasCost) "
                         "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?) "
                         "ON DUPLICATE KEY UPDATE "
                         "Position=VALUES(Position), LongFrozen=VALUES(LongFrozen), "
                         "ShortFrozen=VALUES(ShortFrozen), LongFrozenAmount=VALUES(LongFrozenAmount), "
                         "ShortFrozenAmount=VALUES(ShortFrozenAmount), OpenVolume=VALUES(OpenVolume), "
                         "CloseVolume=VALUES(CloseVolume), OpenAmount=VALUES(OpenAmount), "
                         "CloseAmount=VALUES(CloseAmount), PositionCost=VALUES(PositionCost), "
                         "PreMargin=VALUES(PreMargin), UseMargin=VALUES(UseMargin), "
                         "FrozenMargin=VALUES(FrozenMargin), FrozenCash=VALUES(FrozenCash), "
                         "FrozenCommission=VALUES(FrozenCommission), CashIn=VALUES(CashIn), "
                         "Commission=VALUES(Commission), CloseProfit=VALUES(CloseProfit), "
                         "PositionProfit=VALUES(PositionProfit), PreSettlementPrice=VALUES(PreSettlementPrice), "
                         "SettlementPrice=VALUES(SettlementPrice), SettlementID=VALUES(SettlementID), "
                         "OpenCost=VALUES(OpenCost), ExchangeMargin=VALUES(ExchangeMargin), "
                         "CombPosition=VALUES(CombPosition), CombLongFrozen=VALUES(CombLongFrozen), "
                         "CombShortFrozen=VALUES(CombShortFrozen), CloseProfitByDate=VALUES(CloseProfitByDate), "
                         "CloseProfitByTrade=VALUES(CloseProfitByTrade), TodayPosition=VALUES(TodayPosition), "
                         "MarginRateByMoney=VALUES(MarginRateByMoney), MarginRateByVolume=VALUES(MarginRateByVolume), "
                         "StrikeFrozen=VALUES(StrikeFrozen), StrikeFrozenAmount=VALUES(StrikeFrozenAmount), "
                         "AbandonFrozen=VALUES(AbandonFrozen), YdStrikeFrozen=VALUES(YdStrikeFrozen), "
                         "PositionCostOffset=VALUES(PositionCostOffset), TasPosition=VALUES(TasPosition), "
                         "TasCost=VALUES(TasCost)";
        
        std::unique_ptr<sql::PreparedStatement> pstmt(connection->prepareStatement(sql));
        
        int successCount = 0;
        for (const auto& position : positions) {
            if (position.size() < 49) {
                std::cerr << "持仓数据字段不足，跳过" << std::endl;
                continue;
            }
            
            try {
                int paramIndex = 1;
                pstmt->setString(paramIndex++, position[0]);  // InstrumentID
                pstmt->setString(paramIndex++, position[1]);  // BrokerID
                pstmt->setString(paramIndex++, position[2]);  // InvestorID
                pstmt->setString(paramIndex++, position[3]);  // PosiDirection
                pstmt->setString(paramIndex++, position[4]);  // HedgeFlag
                pstmt->setString(paramIndex++, position[5]);  // PositionDate
                pstmt->setInt(paramIndex++, std::stoi(position[6]));      // YdPosition
                pstmt->setInt(paramIndex++, std::stoi(position[7]));      // Position
                pstmt->setInt(paramIndex++, std::stoi(position[8]));      // LongFrozen
                pstmt->setInt(paramIndex++, std::stoi(position[9]));      // ShortFrozen
                pstmt->setDouble(paramIndex++, std::stod(position[10]));  // LongFrozenAmount
                pstmt->setDouble(paramIndex++, std::stod(position[11]));  // ShortFrozenAmount
                pstmt->setInt(paramIndex++, std::stoi(position[12]));     // OpenVolume
                pstmt->setInt(paramIndex++, std::stoi(position[13]));     // CloseVolume
                pstmt->setDouble(paramIndex++, std::stod(position[14]));  // OpenAmount
                pstmt->setDouble(paramIndex++, std::stod(position[15]));  // CloseAmount
                pstmt->setDouble(paramIndex++, std::stod(position[16]));  // PositionCost
                pstmt->setDouble(paramIndex++, std::stod(position[17]));  // PreMargin
                pstmt->setDouble(paramIndex++, std::stod(position[18]));  // UseMargin
                pstmt->setDouble(paramIndex++, std::stod(position[19]));  // FrozenMargin
                pstmt->setDouble(paramIndex++, std::stod(position[20]));  // FrozenCash
                pstmt->setDouble(paramIndex++, std::stod(position[21]));  // FrozenCommission
                pstmt->setDouble(paramIndex++, std::stod(position[22]));  // CashIn
                pstmt->setDouble(paramIndex++, std::stod(position[23]));  // Commission
                pstmt->setDouble(paramIndex++, std::stod(position[24]));  // CloseProfit
                pstmt->setDouble(paramIndex++, std::stod(position[25]));  // PositionProfit
                pstmt->setDouble(paramIndex++, std::stod(position[26]));  // PreSettlementPrice
                pstmt->setDouble(paramIndex++, std::stod(position[27]));  // SettlementPrice
                pstmt->setString(paramIndex++, position[28]);             // TradingDay
                pstmt->setInt(paramIndex++, std::stoi(position[29]));     // SettlementID
                pstmt->setDouble(paramIndex++, std::stod(position[30]));  // OpenCost
                pstmt->setDouble(paramIndex++, std::stod(position[31]));  // ExchangeMargin
                pstmt->setInt(paramIndex++, std::stoi(position[32]));     // CombPosition
                pstmt->setInt(paramIndex++, std::stoi(position[33]));     // CombLongFrozen
                pstmt->setInt(paramIndex++, std::stoi(position[34]));     // CombShortFrozen
                pstmt->setDouble(paramIndex++, std::stod(position[35]));  // CloseProfitByDate
                pstmt->setDouble(paramIndex++, std::stod(position[36]));  // CloseProfitByTrade
                pstmt->setInt(paramIndex++, std::stoi(position[37]));     // TodayPosition
                pstmt->setDouble(paramIndex++, std::stod(position[38]));  // MarginRateByMoney
                pstmt->setDouble(paramIndex++, std::stod(position[39]));  // MarginRateByVolume
                pstmt->setInt(paramIndex++, std::stoi(position[40]));     // StrikeFrozen
                pstmt->setDouble(paramIndex++, std::stod(position[41]));  // StrikeFrozenAmount
                pstmt->setInt(paramIndex++, std::stoi(position[42]));     // AbandonFrozen
                pstmt->setString(paramIndex++, position[43]);             // ExchangeID
                pstmt->setInt(paramIndex++, std::stoi(position[44]));     // YdStrikeFrozen
                pstmt->setString(paramIndex++, position[45]);             // InvestUnitID
                pstmt->setDouble(paramIndex++, std::stod(position[46]));  // PositionCostOffset
                pstmt->setDouble(paramIndex++, std::stod(position[47]));  // TasPosition
                pstmt->setDouble(paramIndex++, std::stod(position[48]));  // TasCost
                
                pstmt->executeUpdate();
                successCount++;
            } catch (const std::exception& e) {
                std::cerr << "插入持仓数据失败: " << e.what() << std::endl;
            }
        }
        
        // 提交事务
        connection->commit();
        connection->setAutoCommit(true);
        
        std::cout << "批量插入持仓完成，成功: " << successCount << "/" << positions.size() << std::endl;
        return successCount > 0;
        
    } catch (const sql::SQLException& e) {
        // 回滚事务
        try {
            connection->rollback();
            connection->setAutoCommit(true);
        } catch (...) {}
        
        std::cerr << "批量插入持仓失败: " << e.what() << std::endl;
        return false;
    }
}

bool DatabaseManager::insertMarketData(const std::string& tradingDay, const std::string& instrumentID,
                                     const std::string& exchangeID, const std::string& exchangeInstID,
                                     double lastPrice, double preSettlementPrice, double preClosePrice,
                                     double preOpenInterest, double openPrice, double highestPrice,
                                     double lowestPrice, int volume, double turnover, double openInterest,
                                     double closePrice, double settlementPrice, double upperLimitPrice,
                                     double lowerLimitPrice, double preDelta, double currDelta,
                                     const std::string& updateTime, int updateMillisec,
                                     double bidPrice1, int bidVolume1, double askPrice1, int askVolume1,
                                     double bidPrice2, int bidVolume2, double askPrice2, int askVolume2,
                                     double bidPrice3, int bidVolume3, double askPrice3, int askVolume3,
                                     double bidPrice4, int bidVolume4, double askPrice4, int askVolume4,
                                     double bidPrice5, int bidVolume5, double askPrice5, int askVolume5,
                                     double averagePrice, const std::string& actionDay) {
    if (!connected) {
        std::cerr << "数据库未连接" << std::endl;
        return false;
    }

    try {
        std::string sql = "INSERT INTO market_data ("
                         "TradingDay, InstrumentID, ExchangeID, ExchangeInstID, LastPrice, "
                         "PreSettlementPrice, PreClosePrice, PreOpenInterest, OpenPrice, "
                         "HighestPrice, LowestPrice, Volume, Turnover, OpenInterest, ClosePrice, "
                         "SettlementPrice, UpperLimitPrice, LowerLimitPrice, PreDelta, CurrDelta, "
                         "UpdateTime, UpdateMillisec, BidPrice1, BidVolume1, AskPrice1, AskVolume1, "
                         "BidPrice2, BidVolume2, AskPrice2, AskVolume2, BidPrice3, BidVolume3, "
                         "AskPrice3, AskVolume3, BidPrice4, BidVolume4, AskPrice4, AskVolume4, "
                         "BidPrice5, BidVolume5, AskPrice5, AskVolume5, AveragePrice, ActionDay) "
                         "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";

        std::unique_ptr<sql::PreparedStatement> pstmt(connection->prepareStatement(sql));
        
        int paramIndex = 1;
        pstmt->setString(paramIndex++, tradingDay);
        pstmt->setString(paramIndex++, instrumentID);
        pstmt->setString(paramIndex++, exchangeID);
        pstmt->setString(paramIndex++, exchangeInstID);
        pstmt->setDouble(paramIndex++, lastPrice);
        pstmt->setDouble(paramIndex++, preSettlementPrice);
        pstmt->setDouble(paramIndex++, preClosePrice);
        pstmt->setDouble(paramIndex++, preOpenInterest);
        pstmt->setDouble(paramIndex++, openPrice);
        pstmt->setDouble(paramIndex++, highestPrice);
        pstmt->setDouble(paramIndex++, lowestPrice);
        pstmt->setInt(paramIndex++, volume);
        pstmt->setDouble(paramIndex++, turnover);
        pstmt->setDouble(paramIndex++, openInterest);
        pstmt->setDouble(paramIndex++, closePrice);
        pstmt->setDouble(paramIndex++, settlementPrice);
        pstmt->setDouble(paramIndex++, upperLimitPrice);
        pstmt->setDouble(paramIndex++, lowerLimitPrice);
        pstmt->setDouble(paramIndex++, preDelta);
        pstmt->setDouble(paramIndex++, currDelta);
        pstmt->setString(paramIndex++, updateTime);
        pstmt->setInt(paramIndex++, updateMillisec);
        pstmt->setDouble(paramIndex++, bidPrice1);
        pstmt->setInt(paramIndex++, bidVolume1);
        pstmt->setDouble(paramIndex++, askPrice1);
        pstmt->setInt(paramIndex++, askVolume1);
        pstmt->setDouble(paramIndex++, bidPrice2);
        pstmt->setInt(paramIndex++, bidVolume2);
        pstmt->setDouble(paramIndex++, askPrice2);
        pstmt->setInt(paramIndex++, askVolume2);
        pstmt->setDouble(paramIndex++, bidPrice3);
        pstmt->setInt(paramIndex++, bidVolume3);
        pstmt->setDouble(paramIndex++, askPrice3);
        pstmt->setInt(paramIndex++, askVolume3);
        pstmt->setDouble(paramIndex++, bidPrice4);
        pstmt->setInt(paramIndex++, bidVolume4);
        pstmt->setDouble(paramIndex++, askPrice4);
        pstmt->setInt(paramIndex++, askVolume4);
        pstmt->setDouble(paramIndex++, bidPrice5);
        pstmt->setInt(paramIndex++, bidVolume5);
        pstmt->setDouble(paramIndex++, askPrice5);
        pstmt->setInt(paramIndex++, askVolume5);
        pstmt->setDouble(paramIndex++, averagePrice);
        pstmt->setString(paramIndex++, actionDay);
        
        pstmt->executeUpdate();
        return true;
        
    } catch (const sql::SQLException& e) {
        std::cerr << "插入行情数据失败: " << e.what() << std::endl;
        return false;
    }
}

bool DatabaseManager::insertMarketDataBatch(const std::vector<std::string>& marketDataList) {
    if (!connected) {
        std::cerr << "数据库未连接" << std::endl;
        return false;
    }

    try {
        // 开始事务
        connection->setAutoCommit(false);
        
        std::string sql = "INSERT INTO market_data ("
                         "TradingDay, InstrumentID, ExchangeID, ExchangeInstID, LastPrice, "
                         "PreSettlementPrice, PreClosePrice, PreOpenInterest, OpenPrice, "
                         "HighestPrice, LowestPrice, Volume, Turnover, OpenInterest, ClosePrice, "
                         "SettlementPrice, UpperLimitPrice, LowerLimitPrice, PreDelta, CurrDelta, "
                         "UpdateTime, UpdateMillisec, BidPrice1, BidVolume1, AskPrice1, AskVolume1, "
                         "BidPrice2, BidVolume2, AskPrice2, AskVolume2, BidPrice3, BidVolume3, "
                         "AskPrice3, AskVolume3, BidPrice4, BidVolume4, AskPrice4, AskVolume4, "
                         "BidPrice5, BidVolume5, AskPrice5, AskVolume5, AveragePrice, ActionDay) "
                         "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
        
        std::unique_ptr<sql::PreparedStatement> pstmt(connection->prepareStatement(sql));
        
        int successCount = 0;
        for (const auto& marketDataStr : marketDataList) {
            try {
                // 解析行情数据字符串，格式应该是逗号分隔的值
                std::vector<std::string> fields;
                std::string field;
                std::stringstream ss(marketDataStr);
                
                while (std::getline(ss, field, ',')) {
                    fields.push_back(field);
                }
                
                if (fields.size() < 44) {
                    std::cerr << "行情数据字段不足，需要44个字段，实际: " << fields.size() << std::endl;
                    continue;
                }
                
                int paramIndex = 1;
                pstmt->setString(paramIndex++, fields[0]);   // TradingDay
                pstmt->setString(paramIndex++, fields[1]);   // InstrumentID
                pstmt->setString(paramIndex++, fields[2]);   // ExchangeID
                pstmt->setString(paramIndex++, fields[3]);   // ExchangeInstID
                pstmt->setDouble(paramIndex++, std::stod(fields[4]));   // LastPrice
                pstmt->setDouble(paramIndex++, std::stod(fields[5]));   // PreSettlementPrice
                pstmt->setDouble(paramIndex++, std::stod(fields[6]));   // PreClosePrice
                pstmt->setDouble(paramIndex++, std::stod(fields[7]));   // PreOpenInterest
                pstmt->setDouble(paramIndex++, std::stod(fields[8]));   // OpenPrice
                pstmt->setDouble(paramIndex++, std::stod(fields[9]));   // HighestPrice
                pstmt->setDouble(paramIndex++, std::stod(fields[10]));  // LowestPrice
                pstmt->setInt(paramIndex++, std::stoi(fields[11]));     // Volume
                pstmt->setDouble(paramIndex++, std::stod(fields[12]));  // Turnover
                pstmt->setDouble(paramIndex++, std::stod(fields[13]));  // OpenInterest
                pstmt->setDouble(paramIndex++, std::stod(fields[14]));  // ClosePrice
                pstmt->setDouble(paramIndex++, std::stod(fields[15]));  // SettlementPrice
                pstmt->setDouble(paramIndex++, std::stod(fields[16]));  // UpperLimitPrice
                pstmt->setDouble(paramIndex++, std::stod(fields[17]));  // LowerLimitPrice
                pstmt->setDouble(paramIndex++, std::stod(fields[18]));  // PreDelta
                pstmt->setDouble(paramIndex++, std::stod(fields[19]));  // CurrDelta
                pstmt->setString(paramIndex++, fields[20]);  // UpdateTime
                pstmt->setInt(paramIndex++, std::stoi(fields[21]));     // UpdateMillisec
                pstmt->setDouble(paramIndex++, std::stod(fields[22]));  // BidPrice1
                pstmt->setInt(paramIndex++, std::stoi(fields[23]));     // BidVolume1
                pstmt->setDouble(paramIndex++, std::stod(fields[24]));  // AskPrice1
                pstmt->setInt(paramIndex++, std::stoi(fields[25]));     // AskVolume1
                pstmt->setDouble(paramIndex++, std::stod(fields[26]));  // BidPrice2
                pstmt->setInt(paramIndex++, std::stoi(fields[27]));     // BidVolume2
                pstmt->setDouble(paramIndex++, std::stod(fields[28]));  // AskPrice2
                pstmt->setInt(paramIndex++, std::stoi(fields[29]));     // AskVolume2
                pstmt->setDouble(paramIndex++, std::stod(fields[30]));  // BidPrice3
                pstmt->setInt(paramIndex++, std::stoi(fields[31]));     // BidVolume3
                pstmt->setDouble(paramIndex++, std::stod(fields[32]));  // AskPrice3
                pstmt->setInt(paramIndex++, std::stoi(fields[33]));     // AskVolume3
                pstmt->setDouble(paramIndex++, std::stod(fields[34]));  // BidPrice4
                pstmt->setInt(paramIndex++, std::stoi(fields[35]));     // BidVolume4
                pstmt->setDouble(paramIndex++, std::stod(fields[36]));  // AskPrice4
                pstmt->setInt(paramIndex++, std::stoi(fields[37]));     // AskVolume4
                pstmt->setDouble(paramIndex++, std::stod(fields[38]));  // BidPrice5
                pstmt->setInt(paramIndex++, std::stoi(fields[39]));     // BidVolume5
                pstmt->setDouble(paramIndex++, std::stod(fields[40]));  // AskPrice5
                pstmt->setInt(paramIndex++, std::stoi(fields[41]));     // AskVolume5
                pstmt->setDouble(paramIndex++, std::stod(fields[42]));  // AveragePrice
                pstmt->setString(paramIndex++, fields[43]);  // ActionDay
                
                pstmt->executeUpdate();
                successCount++;
                
            } catch (const std::exception& e) {
                std::cerr << "插入行情数据失败: " << e.what() << std::endl;
            }
        }
        
        // 提交事务
        connection->commit();
        connection->setAutoCommit(true);
        
        std::cout << "批量插入行情数据完成，成功: " << successCount << "/" << marketDataList.size() << std::endl;
        return successCount > 0;
        
    } catch (const sql::SQLException& e) {
        // 回滚事务
        try {
            connection->rollback();
            connection->setAutoCommit(true);
        } catch (...) {}
        
        std::cerr << "批量插入行情数据失败: " << e.what() << std::endl;
        return false;
    }
}

bool DatabaseManager::insertTradingAccount(const std::string& brokerID, const std::string& accountID,
                                          double preMortgage, double preCredit, double preDeposit, double preBalance,
                                          double preMargin, double interestBase, double interest, double deposit,
                                          double withdraw, double frozenMargin, double frozenCash, double frozenCommission,
                                          double currMargin, double cashIn, double commission, double closeProfit,
                                          double positionProfit, double balance, double available, double withdrawQuota,
                                          double reserve, const std::string& tradingDay, int settlementID, double credit,
                                          double mortgage, double exchangeMargin, double deliveryMargin,
                                          double exchangeDeliveryMargin, double reserveBalance, const std::string& currencyID,
                                          double preFundMortgageIn, double preFundMortgageOut, double fundMortgageIn,
                                          double fundMortgageOut, double fundMortgageAvailable, double mortgageableFund,
                                          double specProductMargin, double specProductFrozenMargin, double specProductCommission,
                                          double specProductFrozenCommission, double specProductPositionProfit,
                                          double specProductCloseProfit, double specProductPositionProfitByAlg,
                                          double specProductExchangeMargin, const std::string& bizType,
                                          double frozenSwap, double remainSwap) {
    if (!connected) {
        std::cerr << "数据库未连接" << std::endl;
        return false;
    }

    try {
        std::string sql = "INSERT INTO trading_account ("
                         "BrokerID, AccountID, PreMortgage, PreCredit, PreDeposit, PreBalance, PreMargin, "
                         "InterestBase, Interest, Deposit, Withdraw, FrozenMargin, FrozenCash, FrozenCommission, "
                         "CurrMargin, CashIn, Commission, CloseProfit, PositionProfit, Balance, Available, "
                         "WithdrawQuota, Reserve, TradingDay, SettlementID, Credit, Mortgage, ExchangeMargin, "
                         "DeliveryMargin, ExchangeDeliveryMargin, ReserveBalance, CurrencyID, PreFundMortgageIn, "
                         "PreFundMortgageOut, FundMortgageIn, FundMortgageOut, FundMortgageAvailable, "
                         "MortgageableFund, SpecProductMargin, SpecProductFrozenMargin, SpecProductCommission, "
                         "SpecProductFrozenCommission, SpecProductPositionProfit, SpecProductCloseProfit, "
                         "SpecProductPositionProfitByAlg, SpecProductExchangeMargin, BizType, FrozenSwap, RemainSwap) "
                         "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?) "
                         "ON DUPLICATE KEY UPDATE "
                         "PreMortgage=VALUES(PreMortgage), PreCredit=VALUES(PreCredit), PreDeposit=VALUES(PreDeposit), "
                         "PreBalance=VALUES(PreBalance), PreMargin=VALUES(PreMargin), InterestBase=VALUES(InterestBase), "
                         "Interest=VALUES(Interest), Deposit=VALUES(Deposit), Withdraw=VALUES(Withdraw), "
                         "FrozenMargin=VALUES(FrozenMargin), FrozenCash=VALUES(FrozenCash), "
                         "FrozenCommission=VALUES(FrozenCommission), CurrMargin=VALUES(CurrMargin), "
                         "CashIn=VALUES(CashIn), Commission=VALUES(Commission), CloseProfit=VALUES(CloseProfit), "
                         "PositionProfit=VALUES(PositionProfit), Balance=VALUES(Balance), Available=VALUES(Available), "
                         "WithdrawQuota=VALUES(WithdrawQuota), Reserve=VALUES(Reserve), SettlementID=VALUES(SettlementID), "
                         "Credit=VALUES(Credit), Mortgage=VALUES(Mortgage), ExchangeMargin=VALUES(ExchangeMargin), "
                         "DeliveryMargin=VALUES(DeliveryMargin), ExchangeDeliveryMargin=VALUES(ExchangeDeliveryMargin), "
                         "ReserveBalance=VALUES(ReserveBalance), CurrencyID=VALUES(CurrencyID), "
                         "PreFundMortgageIn=VALUES(PreFundMortgageIn), PreFundMortgageOut=VALUES(PreFundMortgageOut), "
                         "FundMortgageIn=VALUES(FundMortgageIn), FundMortgageOut=VALUES(FundMortgageOut), "
                         "FundMortgageAvailable=VALUES(FundMortgageAvailable), MortgageableFund=VALUES(MortgageableFund), "
                         "SpecProductMargin=VALUES(SpecProductMargin), SpecProductFrozenMargin=VALUES(SpecProductFrozenMargin), "
                         "SpecProductCommission=VALUES(SpecProductCommission), SpecProductFrozenCommission=VALUES(SpecProductFrozenCommission), "
                         "SpecProductPositionProfit=VALUES(SpecProductPositionProfit), SpecProductCloseProfit=VALUES(SpecProductCloseProfit), "
                         "SpecProductPositionProfitByAlg=VALUES(SpecProductPositionProfitByAlg), SpecProductExchangeMargin=VALUES(SpecProductExchangeMargin), "
                         "BizType=VALUES(BizType), FrozenSwap=VALUES(FrozenSwap), RemainSwap=VALUES(RemainSwap)";

        std::unique_ptr<sql::PreparedStatement> pstmt(connection->prepareStatement(sql));
        
        int paramIndex = 1;
        pstmt->setString(paramIndex++, brokerID);
        pstmt->setString(paramIndex++, accountID);
        pstmt->setDouble(paramIndex++, preMortgage);
        pstmt->setDouble(paramIndex++, preCredit);
        pstmt->setDouble(paramIndex++, preDeposit);
        pstmt->setDouble(paramIndex++, preBalance);
        pstmt->setDouble(paramIndex++, preMargin);
        pstmt->setDouble(paramIndex++, interestBase);
        pstmt->setDouble(paramIndex++, interest);
        pstmt->setDouble(paramIndex++, deposit);
        pstmt->setDouble(paramIndex++, withdraw);
        pstmt->setDouble(paramIndex++, frozenMargin);
        pstmt->setDouble(paramIndex++, frozenCash);
        pstmt->setDouble(paramIndex++, frozenCommission);
        pstmt->setDouble(paramIndex++, currMargin);
        pstmt->setDouble(paramIndex++, cashIn);
        pstmt->setDouble(paramIndex++, commission);
        pstmt->setDouble(paramIndex++, closeProfit);
        pstmt->setDouble(paramIndex++, positionProfit);
        pstmt->setDouble(paramIndex++, balance);
        pstmt->setDouble(paramIndex++, available);
        pstmt->setDouble(paramIndex++, withdrawQuota);
        pstmt->setDouble(paramIndex++, reserve);
        pstmt->setString(paramIndex++, tradingDay);
        pstmt->setInt(paramIndex++, settlementID);
        pstmt->setDouble(paramIndex++, credit);
        pstmt->setDouble(paramIndex++, mortgage);
        pstmt->setDouble(paramIndex++, exchangeMargin);
        pstmt->setDouble(paramIndex++, deliveryMargin);
        pstmt->setDouble(paramIndex++, exchangeDeliveryMargin);
        pstmt->setDouble(paramIndex++, reserveBalance);
        pstmt->setString(paramIndex++, currencyID);
        pstmt->setDouble(paramIndex++, preFundMortgageIn);
        pstmt->setDouble(paramIndex++, preFundMortgageOut);
        pstmt->setDouble(paramIndex++, fundMortgageIn);
        pstmt->setDouble(paramIndex++, fundMortgageOut);
        pstmt->setDouble(paramIndex++, fundMortgageAvailable);
        pstmt->setDouble(paramIndex++, mortgageableFund);
        pstmt->setDouble(paramIndex++, specProductMargin);
        pstmt->setDouble(paramIndex++, specProductFrozenMargin);
        pstmt->setDouble(paramIndex++, specProductCommission);
        pstmt->setDouble(paramIndex++, specProductFrozenCommission);
        pstmt->setDouble(paramIndex++, specProductPositionProfit);
        pstmt->setDouble(paramIndex++, specProductCloseProfit);
        pstmt->setDouble(paramIndex++, specProductPositionProfitByAlg);
        pstmt->setDouble(paramIndex++, specProductExchangeMargin);
        pstmt->setString(paramIndex++, bizType);
        pstmt->setDouble(paramIndex++, frozenSwap);
        pstmt->setDouble(paramIndex++, remainSwap);
        
        pstmt->executeUpdate();
        return true;
        
    } catch (const sql::SQLException& e) {
        std::cerr << "插入资金账户数据失败: " << e.what() << std::endl;
        return false;
    }
}

bool DatabaseManager::insertTradingAccountBatch(const std::vector<std::string>& accountDataList) {
    if (!connected) {
        std::cerr << "数据库未连接" << std::endl;
        return false;
    }

    try {
        // 开始事务
        connection->setAutoCommit(false);
        
        std::string sql = "INSERT INTO trading_account ("
                         "BrokerID, AccountID, PreMortgage, PreCredit, PreDeposit, PreBalance, PreMargin, "
                         "InterestBase, Interest, Deposit, Withdraw, FrozenMargin, FrozenCash, FrozenCommission, "
                         "CurrMargin, CashIn, Commission, CloseProfit, PositionProfit, Balance, Available, "
                         "WithdrawQuota, Reserve, TradingDay, SettlementID, Credit, Mortgage, ExchangeMargin, "
                         "DeliveryMargin, ExchangeDeliveryMargin, ReserveBalance, CurrencyID, PreFundMortgageIn, "
                         "PreFundMortgageOut, FundMortgageIn, FundMortgageOut, FundMortgageAvailable, "
                         "MortgageableFund, SpecProductMargin, SpecProductFrozenMargin, SpecProductCommission, "
                         "SpecProductFrozenCommission, SpecProductPositionProfit, SpecProductCloseProfit, "
                         "SpecProductPositionProfitByAlg, SpecProductExchangeMargin, BizType, FrozenSwap, RemainSwap) "
                         "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?) "
                         "ON DUPLICATE KEY UPDATE "
                         "PreMortgage=VALUES(PreMortgage), PreCredit=VALUES(PreCredit), PreDeposit=VALUES(PreDeposit), "
                         "PreBalance=VALUES(PreBalance), PreMargin=VALUES(PreMargin), InterestBase=VALUES(InterestBase), "
                         "Interest=VALUES(Interest), Deposit=VALUES(Deposit), Withdraw=VALUES(Withdraw), "
                         "FrozenMargin=VALUES(FrozenMargin), FrozenCash=VALUES(FrozenCash), "
                         "FrozenCommission=VALUES(FrozenCommission), CurrMargin=VALUES(CurrMargin), "
                         "CashIn=VALUES(CashIn), Commission=VALUES(Commission), CloseProfit=VALUES(CloseProfit), "
                         "PositionProfit=VALUES(PositionProfit), Balance=VALUES(Balance), Available=VALUES(Available), "
                         "WithdrawQuota=VALUES(WithdrawQuota), Reserve=VALUES(Reserve), SettlementID=VALUES(SettlementID), "
                         "Credit=VALUES(Credit), Mortgage=VALUES(Mortgage), ExchangeMargin=VALUES(ExchangeMargin), "
                         "DeliveryMargin=VALUES(DeliveryMargin), ExchangeDeliveryMargin=VALUES(ExchangeDeliveryMargin), "
                         "ReserveBalance=VALUES(ReserveBalance), CurrencyID=VALUES(CurrencyID), "
                         "PreFundMortgageIn=VALUES(PreFundMortgageIn), PreFundMortgageOut=VALUES(PreFundMortgageOut), "
                         "FundMortgageIn=VALUES(FundMortgageIn), FundMortgageOut=VALUES(FundMortgageOut), "
                         "FundMortgageAvailable=VALUES(FundMortgageAvailable), MortgageableFund=VALUES(MortgageableFund), "
                         "SpecProductMargin=VALUES(SpecProductMargin), SpecProductFrozenMargin=VALUES(SpecProductFrozenMargin), "
                         "SpecProductCommission=VALUES(SpecProductCommission), SpecProductFrozenCommission=VALUES(SpecProductFrozenCommission), "
                         "SpecProductPositionProfit=VALUES(SpecProductPositionProfit), SpecProductCloseProfit=VALUES(SpecProductCloseProfit), "
                         "SpecProductPositionProfitByAlg=VALUES(SpecProductPositionProfitByAlg), SpecProductExchangeMargin=VALUES(SpecProductExchangeMargin), "
                         "BizType=VALUES(BizType), FrozenSwap=VALUES(FrozenSwap), RemainSwap=VALUES(RemainSwap)";
        
        std::unique_ptr<sql::PreparedStatement> pstmt(connection->prepareStatement(sql));
        
        int successCount = 0;
        for (const auto& accountDataStr : accountDataList) {
            try {
                // 解析资金账户数据字符串，格式应该是逗号分隔的值
                std::vector<std::string> fields;
                std::string field;
                std::stringstream ss(accountDataStr);
                
                while (std::getline(ss, field, ',')) {
                    fields.push_back(field);
                }
                
                if (fields.size() < 46) {
                    std::cerr << "资金账户数据字段不足，需要至少46个字段，实际: " << fields.size() << std::endl;
                    continue;
                }
                
                int paramIndex = 1;
                pstmt->setString(paramIndex++, fields[0]);   // BrokerID
                pstmt->setString(paramIndex++, fields[1]);   // AccountID
                pstmt->setDouble(paramIndex++, std::stod(fields[2]));   // PreMortgage
                pstmt->setDouble(paramIndex++, std::stod(fields[3]));   // PreCredit
                pstmt->setDouble(paramIndex++, std::stod(fields[4]));   // PreDeposit
                pstmt->setDouble(paramIndex++, std::stod(fields[5]));   // PreBalance
                pstmt->setDouble(paramIndex++, std::stod(fields[6]));   // PreMargin
                pstmt->setDouble(paramIndex++, std::stod(fields[7]));   // InterestBase
                pstmt->setDouble(paramIndex++, std::stod(fields[8]));   // Interest
                pstmt->setDouble(paramIndex++, std::stod(fields[9]));   // Deposit
                pstmt->setDouble(paramIndex++, std::stod(fields[10]));  // Withdraw
                pstmt->setDouble(paramIndex++, std::stod(fields[11]));  // FrozenMargin
                pstmt->setDouble(paramIndex++, std::stod(fields[12]));  // FrozenCash
                pstmt->setDouble(paramIndex++, std::stod(fields[13]));  // FrozenCommission
                pstmt->setDouble(paramIndex++, std::stod(fields[14]));  // CurrMargin
                pstmt->setDouble(paramIndex++, std::stod(fields[15]));  // CashIn
                pstmt->setDouble(paramIndex++, std::stod(fields[16]));  // Commission
                pstmt->setDouble(paramIndex++, std::stod(fields[17]));  // CloseProfit
                pstmt->setDouble(paramIndex++, std::stod(fields[18]));  // PositionProfit
                pstmt->setDouble(paramIndex++, std::stod(fields[19]));  // Balance
                pstmt->setDouble(paramIndex++, std::stod(fields[20]));  // Available
                pstmt->setDouble(paramIndex++, std::stod(fields[21]));  // WithdrawQuota
                pstmt->setDouble(paramIndex++, std::stod(fields[22]));  // Reserve
                pstmt->setString(paramIndex++, fields[23]);  // TradingDay
                pstmt->setInt(paramIndex++, std::stoi(fields[24]));     // SettlementID
                pstmt->setDouble(paramIndex++, std::stod(fields[25]));  // Credit
                pstmt->setDouble(paramIndex++, std::stod(fields[26]));  // Mortgage
                pstmt->setDouble(paramIndex++, std::stod(fields[27]));  // ExchangeMargin
                pstmt->setDouble(paramIndex++, std::stod(fields[28]));  // DeliveryMargin
                pstmt->setDouble(paramIndex++, std::stod(fields[29]));  // ExchangeDeliveryMargin
                pstmt->setDouble(paramIndex++, std::stod(fields[30]));  // ReserveBalance
                pstmt->setString(paramIndex++, fields[31]);  // CurrencyID
                pstmt->setDouble(paramIndex++, std::stod(fields[32]));  // PreFundMortgageIn
                pstmt->setDouble(paramIndex++, std::stod(fields[33]));  // PreFundMortgageOut
                pstmt->setDouble(paramIndex++, std::stod(fields[34]));  // FundMortgageIn
                pstmt->setDouble(paramIndex++, std::stod(fields[35]));  // FundMortgageOut
                pstmt->setDouble(paramIndex++, std::stod(fields[36]));  // FundMortgageAvailable
                pstmt->setDouble(paramIndex++, std::stod(fields[37]));  // MortgageableFund
                pstmt->setDouble(paramIndex++, std::stod(fields[38]));  // SpecProductMargin
                pstmt->setDouble(paramIndex++, std::stod(fields[39]));  // SpecProductFrozenMargin
                pstmt->setDouble(paramIndex++, std::stod(fields[40]));  // SpecProductCommission
                pstmt->setDouble(paramIndex++, std::stod(fields[41]));  // SpecProductFrozenCommission
                pstmt->setDouble(paramIndex++, std::stod(fields[42]));  // SpecProductPositionProfit
                pstmt->setDouble(paramIndex++, std::stod(fields[43]));  // SpecProductCloseProfit
                pstmt->setDouble(paramIndex++, std::stod(fields[44]));  // SpecProductPositionProfitByAlg
                pstmt->setDouble(paramIndex++, std::stod(fields[45]));  // SpecProductExchangeMargin
                
                // 可选字段
                if (fields.size() > 46) {
                    pstmt->setString(paramIndex++, fields[46]);  // BizType
                } else {
                    pstmt->setString(paramIndex++, "");
                }
                
                if (fields.size() > 47) {
                    pstmt->setDouble(paramIndex++, std::stod(fields[47]));  // FrozenSwap
                } else {
                    pstmt->setDouble(paramIndex++, 0.0);
                }
                
                if (fields.size() > 48) {
                    pstmt->setDouble(paramIndex++, std::stod(fields[48]));  // RemainSwap
                } else {
                    pstmt->setDouble(paramIndex++, 0.0);
                }
                
                pstmt->executeUpdate();
                successCount++;
                
            } catch (const std::exception& e) {
                std::cerr << "插入资金账户数据失败: " << e.what() << std::endl;
            }
        }
        
        // 提交事务
        connection->commit();
        connection->setAutoCommit(true);
        
        std::cout << "批量插入资金账户数据完成，成功: " << successCount << "/" << accountDataList.size() << std::endl;
        return successCount > 0;
        
    } catch (const sql::SQLException& e) {
        // 回滚事务
        try {
            connection->rollback();
            connection->setAutoCommit(true);
        } catch (...) {}
        
        std::cerr << "批量插入资金账户数据失败: " << e.what() << std::endl;
        return false;
    }
} 