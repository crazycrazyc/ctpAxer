#ifndef ZMQ_SUBSCRIBER_H
#define ZMQ_SUBSCRIBER_H

#include <string>
#include <functional>
#include <memory>
#include <thread>
#include <atomic>
#include <vector>
#include <mutex>
#include <chrono>
#include <zmq.hpp>
#include "Logger.h"
#include "DatabaseManager.h"
#include "MarketDataConverter.h"
#include "../proto/market_data.pb.h"

// 消息处理函数类型定义
using MessageHandler = std::function<void(const std::string&, const std::string&)>;

class ZMQSubscriber {
private:
    std::unique_ptr<zmq::context_t> context;
    std::unique_ptr<zmq::socket_t> socket;
    std::string address;
    std::atomic<bool> running;
    std::thread subscriberThread;
    
    std::shared_ptr<Logger> logger;
    std::shared_ptr<DatabaseManager> dbManager;
    MessageHandler messageHandler;

    // 批量写入相关成员
    std::vector<CTPMarketDataField> marketDataBuffer;  // 行情数据缓冲区
    std::mutex bufferMutex;                            // 缓冲区互斥锁
    std::thread batchWriterThread;                     // 批量写入线程
    std::chrono::steady_clock::time_point lastWriteTime; // 上次写入时间
    const std::chrono::seconds batchWriteInterval{30}; // 批量写入间隔（30秒）
    const size_t maxBufferSize{1000};                 // 最大缓冲区大小

    void subscriberLoop();
    void processMessage(const std::string& messageType, const std::string& messageContent);
    
    // 批量写入相关方法
    void batchWriterLoop();                           // 批量写入线程循环
    void flushMarketDataBuffer();                     // 刷新缓冲区到数据库
    void addMarketDataToBuffer(const CTPMarketDataField& marketData); // 添加数据到缓冲区
    
    // 默认消息处理函数
    void defaultMessageHandler(const std::string& messageType, const std::string& messageContent);
    
    // 处理合约消息
    void processInstrumentMessage(const std::string& messageContent);
    void processCSVInstrumentMessage(const std::string& messageContent);
    void processProtobufInstrumentMessage(const std::string& messageContent);
    
    // 处理投资者持仓消息
    void processInvestorPositionMessage(const std::string& messageContent);
    void processCSVInvestorPositionMessage(const std::string& messageContent);
    void processProtobufInvestorPositionMessage(const std::string& messageContent);

    // 处理行情数据消息
    void processMarketDataMessage(const std::string& messageContent);
    void processCSVMarketDataMessage(const std::string& messageContent);
    void processProtobufMarketDataMessage(const std::string& messageContent);
    
    // 处理资金账户数据消息
    void processTradingAccountMessage(const std::string& messageContent);
    void processCSVTradingAccountMessage(const std::string& messageContent);

public:
    ZMQSubscriber(const std::string& address = "tcp://localhost:8888");
    ~ZMQSubscriber();

    bool initialize();
    void start();
    void stop();
    bool isRunning() const { return running; }
    
    // 设置依赖
    void setLogger(std::shared_ptr<Logger> log) { logger = log; }
    void setDatabaseManager(std::shared_ptr<DatabaseManager> db) { dbManager = db; }
    void setMessageHandler(MessageHandler handler) { messageHandler = handler; }
    
    // 获取地址
    std::string getAddress() const { return address; }
    
    // 获取缓冲区状态
    size_t getBufferSize() const;
};

#endif // ZMQ_SUBSCRIBER_H 