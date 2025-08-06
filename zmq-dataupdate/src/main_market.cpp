#include "../include/ZMQSubscriber.h"
#include "../include/DatabaseManager.h"
#include "../include/Logger.h"
#include "../include/ConfigManager.h"
#include <iostream>
#include <csignal>
#include <memory>
#include <thread>
#include <chrono>

std::shared_ptr<ZMQSubscriber> marketSubscriber = nullptr;

void signalHandler(int signal) {
    std::cout << "\n收到信号 " << signal << "，正在停止行情数据订阅..." << std::endl;
    if (marketSubscriber) {
        marketSubscriber->stop();
    }
}

int main() {
    // 设置信号处理
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    try {
        // 创建日志记录器
        auto logger = std::make_shared<Logger>("market_data.log");
        logger->info("启动行情数据订阅器");

        // 读取配置
        ConfigManager config("config.ini");
        if (!config.loadConfig()) {
            logger->error("配置文件加载失败");
            return -1;
        }
        
        // 数据库配置
        std::string dbHost = config.getDBHost();
        int dbPort = config.getDBPort();
        std::string dbUser = config.getDBUser();
        std::string dbPassword = config.getDBPassword();
        std::string dbName = config.getDBName();
        
        // ZMQ配置 - 连接到行情数据端口
        std::string zmqAddress = config.getValue("zmq.market_address", "tcp://localhost:9999");
        
        logger->info("配置信息:");
        logger->info("  数据库: " + dbHost + ":" + std::to_string(dbPort) + "/" + dbName);
        logger->info("  ZMQ地址: " + zmqAddress);

        // 创建数据库管理器
        auto dbManager = std::make_shared<DatabaseManager>(dbHost, dbPort, dbUser, dbPassword, dbName);
        
        if (!dbManager->connect()) {
            logger->error("数据库连接失败");
            return -1;
        }
        
        // 创建行情数据表
        std::string createTableSQL = R"(
            CREATE TABLE IF NOT EXISTS market_data (
                id BIGINT AUTO_INCREMENT PRIMARY KEY,
                TradingDay CHAR(9) NOT NULL COMMENT '交易日',
                InstrumentID CHAR(31) NOT NULL COMMENT '合约代码',
                ExchangeID CHAR(9) NOT NULL COMMENT '交易所代码',
                ExchangeInstID CHAR(31) COMMENT '交易所合约代码',
                LastPrice DOUBLE COMMENT '最新价',
                PreSettlementPrice DOUBLE COMMENT '上次结算价',
                PreClosePrice DOUBLE COMMENT '昨收盘',
                PreOpenInterest DOUBLE COMMENT '昨持仓量',
                OpenPrice DOUBLE COMMENT '今开盘',
                HighestPrice DOUBLE COMMENT '最高价',
                LowestPrice DOUBLE COMMENT '最低价',
                Volume INT COMMENT '数量',
                Turnover DOUBLE COMMENT '成交金额',
                OpenInterest DOUBLE COMMENT '持仓量',
                ClosePrice DOUBLE COMMENT '今收盘',
                SettlementPrice DOUBLE COMMENT '本次结算价',
                UpperLimitPrice DOUBLE COMMENT '涨停板价',
                LowerLimitPrice DOUBLE COMMENT '跌停板价',
                PreDelta DOUBLE COMMENT '昨虚实度',
                CurrDelta DOUBLE COMMENT '今虚实度',
                UpdateTime CHAR(9) COMMENT '最后修改时间',
                UpdateMillisec INT COMMENT '最后修改毫秒',
                BidPrice1 DOUBLE COMMENT '申买价一',
                BidVolume1 INT COMMENT '申买量一',
                AskPrice1 DOUBLE COMMENT '申卖价一',
                AskVolume1 INT COMMENT '申卖量一',
                BidPrice2 DOUBLE COMMENT '申买价二',
                BidVolume2 INT COMMENT '申买量二',
                AskPrice2 DOUBLE COMMENT '申卖价二',
                AskVolume2 INT COMMENT '申卖量二',
                BidPrice3 DOUBLE COMMENT '申买价三',
                BidVolume3 INT COMMENT '申买量三',
                AskPrice3 DOUBLE COMMENT '申卖价三',
                AskVolume3 INT COMMENT '申卖量三',
                BidPrice4 DOUBLE COMMENT '申买价四',
                BidVolume4 INT COMMENT '申买量四',
                AskPrice4 DOUBLE COMMENT '申卖价四',
                AskVolume4 INT COMMENT '申卖量四',
                BidPrice5 DOUBLE COMMENT '申买价五',
                BidVolume5 INT COMMENT '申买量五',
                AskPrice5 DOUBLE COMMENT '申卖价五',
                AskVolume5 INT COMMENT '申卖量五',
                AveragePrice DOUBLE COMMENT '当日均价',
                ActionDay CHAR(9) COMMENT '业务日期',
                RecordTime TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '入库时间',
                
                INDEX idx_instrument_time (InstrumentID, TradingDay, UpdateTime),
                INDEX idx_trading_day (TradingDay),
                INDEX idx_update_time (UpdateTime),
                INDEX idx_record_time (RecordTime)
            ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='行情数据表'
        )";
        
        if (dbManager->execute(createTableSQL)) {
            logger->info("行情数据表检查/创建成功");
        } else {
            logger->warning("行情数据表创建失败，可能已存在");
        }

        // 创建ZMQ订阅器
        marketSubscriber = std::make_shared<ZMQSubscriber>(zmqAddress);
        marketSubscriber->setLogger(logger);
        marketSubscriber->setDatabaseManager(dbManager);

        if (!marketSubscriber->initialize()) {
            logger->error("ZMQ订阅器初始化失败");
            return -1;
        }

        logger->info("开始订阅行情数据...");
        marketSubscriber->start();

        // 主循环 - 添加缓冲区状态监控
        int monitorCount = 0;
        while (marketSubscriber->isRunning()) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            
            // 每10秒输出一次缓冲区状态
            monitorCount++;
            if (monitorCount >= 10) {
                size_t bufferSize = marketSubscriber->getBufferSize();
                logger->info("缓冲区状态 - 当前大小: " + std::to_string(bufferSize) + 
                           " 条记录，批量写入间隔: 30秒");
                monitorCount = 0;
            }
        }

        logger->info("行情数据订阅器正常退出");

    } catch (const std::exception& e) {
        std::cerr << "程序异常: " << e.what() << std::endl;
        return -1;
    }

    return 0;
} 