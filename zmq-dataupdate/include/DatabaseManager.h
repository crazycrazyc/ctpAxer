#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include <string>
#include <memory>
#include <vector>
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

class DatabaseManager {
private:
    std::unique_ptr<sql::mysql::MySQL_Driver> driver;
    std::unique_ptr<sql::Connection> connection;
    std::string host;
    int port;
    std::string user;
    std::string password;
    std::string database;
    bool connected;

public:
    DatabaseManager(const std::string& host, int port, 
                   const std::string& user, const std::string& password, 
                   const std::string& database);
    ~DatabaseManager();

    bool connect();
    void disconnect();
    bool isConnected() const { return connected; }
    
    // 数据插入方法
    bool insertMessage(const std::string& messageType, const std::string& messageContent);
    bool insertTickData(const std::string& symbol, double price, int volume, const std::string& timestamp);
    
    // 合约信息插入方法
    bool insertInstrument(const std::string& instrumentId, const std::string& exchangeId, 
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
                         double underlyingMultiple, char combinationType);
    
    // 批量插入合约信息
    bool insertInstrumentBatch(const std::vector<std::vector<std::string>>& instruments);
    
    // 投资者持仓信息插入方法
    bool insertInvestorPosition(const std::string& instrumentId, const std::string& brokerId,
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
                               double tasPosition, double tasCost);
    
    // 批量插入投资者持仓信息
    bool insertInvestorPositionBatch(const std::vector<std::vector<std::string>>& positions);
    
    // 行情数据插入方法
    bool insertMarketData(const std::string& tradingDay, const std::string& instrumentID,
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
                         double averagePrice, const std::string& actionDay);
    
    // 批量插入行情数据
    bool insertMarketDataBatch(const std::vector<std::string>& marketDataList);
    
    // 资金账户数据插入方法
    bool insertTradingAccount(const std::string& brokerID, const std::string& accountID,
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
                             double frozenSwap, double remainSwap);
    
    // 批量插入资金账户数据
    bool insertTradingAccountBatch(const std::vector<std::string>& accountDataList);
    
    // 查询方法
    std::unique_ptr<sql::ResultSet> query(const std::string& sql);
    bool execute(const std::string& sql);
};

#endif // DATABASE_MANAGER_H 