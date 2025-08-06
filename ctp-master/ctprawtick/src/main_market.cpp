#include <iostream>
#include <chrono>
#include <thread>
#include <string>
#include <vector>
#include <condition_variable>
#include <mutex>
#include <zmq.hpp>
#include "zlog.h"
#include "utils.h"
#include "CTPQuote.h"
#include "Config.h"
#include "readerwriterqueue.h"
#include "../include/ZMQPublisher.h"
#include "../include/ProtobufConverter.h"
#include "ThostFtdcUserApiStruct.h"
#include <signal.h>

using namespace std;
using namespace moodycamel;
// global variable definitions needed by CTPQuote
extern string currentPath;           // 在utils.cpp中定义
vector<string>      contracts;        // 合约列表
extern vector<string> DECSymbols;     // 在appConfig.cpp中定义
condition_variable  cv;
mutex               m;
bool                isReady = false;
extern string       pushServer;       // 在appConfig.cpp中定义

// 修改队列类型：使用CThostFtdcDepthMarketDataField而不是string
ReaderWriterQueue<CThostFtdcDepthMarketDataField>  q(10000);
// 缓存发送不成功的行情数据
ReaderWriterQueue<CThostFtdcDepthMarketDataField>  bufq(500000);

// global variable
zlog_category_t *cat = nullptr;

// ZMQ Publisher for market data
ZMQPublisher marketPublisher("tcp://*:9999");  // 使用不同的端口发布行情数据

void signal_handler(int signal)
{
    std::cout << "收到信号 " << signal << "，准备退出..." << std::endl;
    exit(0);
}

int main(int argc, char** argv)
{
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    get_self_path();
    if (currentPath.empty())
    {
        cerr << "无法获取当前执行路径." << endl;
        return -1;
    }

    int rc = zlog_init((currentPath + "/zlog.conf").c_str());
    if (rc)
    {
        cerr << "找不到zlog配置文件." << endl;
        return -1;
    }

    cat = zlog_get_category("my_cat");
    if (!cat)
    {
        cerr << "无效的zlog配置文件格式." << endl;
        zlog_fini();
        return -1;
    }

    zlog_info(cat, "[main] 成功加载zlog配置文件.");
    
    // 初始化ZMQ发布者
    if (!marketPublisher.connect()) {
        zlog_error(cat, "[main] ZMQ行情发布者初始化失败");
        zlog_fini();
        return -1;
    }
    zlog_info(cat, "[main] ZMQ行情发布者初始化成功，端口: 9999");

    CAppConfig appConfig;

    // 修改订阅策略：从数据库读取所有可用合约（由ctpinstrument程序维护）
    contracts.clear();
    
    zlog_info(cat, "[main] 从数据库动态读取所有可用合约");
    
    // 由于数据库连接复杂，先使用一个广泛的主力合约列表
    // 这包含了主要品种的当月、次月合约，覆盖大部分活跃交易
    vector<string> products = {
        "cu", "al", "zn", "au", "ag", "rb", "hc", "i", "j", "jm",  // 金属能源
        "m", "y", "p", "a", "b", "c", "cs", "jd",                 // 农产品
        "CF", "SR", "TA", "MA", "FG", "RM", "OI", "SF", "SM"      // 纺织化工
    };
    vector<string> months = {"2508", "2509", "2510"};            // 当月、次月、季月
    
    for (const auto& product : products) {
        for (const auto& month : months) {
            contracts.push_back(product + month);
        }
    }
    
    zlog_info(cat, "[main] 动态生成合约订阅列表，共 %zu 个合约", contracts.size());
    zlog_info(cat, "[main] 覆盖主要品种的活跃合约，确保接收广泛的行情数据");
    
    for (size_t i = 0; i < contracts.size() && i < 15; i++) {
        zlog_info(cat, "[main] 动态合约[%zu]: %s", i+1, contracts[i].c_str());
    }
    if (contracts.size() > 15) {
        zlog_info(cat, "[main] ... 还有 %zu 个合约", contracts.size() - 15);
    }

    CTPMarketSpi mdspi;
    if (mdspi.Create(appConfig))
    {
        zlog_info(cat, "[main] 行情API创建成功，开始订阅行情...");
        
        chrono::seconds dura(1);
        CThostFtdcDepthMarketDataField marketData;
        int messageCount = 0;
        
        zlog_info(cat, "[main] 开始行情数据处理循环（使用protobuf格式）");
        
        for (;;)
        {
            time_t now = time(nullptr);
            if (mdspi.exitTs < now)
            {
                zlog_info(cat, "[main] 到达退出时间，程序结束");
                break;
            }

            // 处理缓存队列中的消息
            while (bufq.try_dequeue(marketData))
            {
                // 生成本地时间戳
                std::string localTimestamp = ProtobufConverter::generateLocalTimestamp();
                
                // 转换为protobuf格式
                ctp::MarketDataMessage protoMessage = ProtobufConverter::convertToProtobuf(marketData, localTimestamp);
                
                // 序列化为字节数组
                std::string serializedData = ProtobufConverter::serializeToString(protoMessage);
                
                if (!serializedData.empty()) {
                    // 通过ZMQ发布protobuf格式的行情数据
                    if (marketPublisher.publishMessage("MARKET_DATA_PROTOBUF", serializedData)) {
                        messageCount++;
                        if (messageCount % 1000 == 0) {
                            zlog_info(cat, "[main] 已发布 %d 条protobuf行情数据", messageCount);
                        }
                    } else {
                        // 发送失败，重新放回缓存队列
                        if (!bufq.try_enqueue(marketData))
                        {
                            zlog_error(cat, "[main] 缓存队列已满，丢弃消息: %s", marketData.InstrumentID);
                        }
                    }
                } else {
                    zlog_error(cat, "[main] protobuf序列化失败: %s", marketData.InstrumentID);
                }
            }

            // 处理主队列中的消息
            if (!q.try_dequeue(marketData))
            {
                this_thread::sleep_for(dura);
                continue;
            }

            // 添加调试日志：记录从主队列取出数据
            zlog_info(cat, "[main] 从主队列取出行情数据: %s, 最新价: %.2f", 
                      marketData.InstrumentID ? marketData.InstrumentID : "Unknown",
                      marketData.LastPrice);

            // 生成本地时间戳
            std::string localTimestamp = ProtobufConverter::generateLocalTimestamp();
            
            // 转换为protobuf格式
            ctp::MarketDataMessage protoMessage = ProtobufConverter::convertToProtobuf(marketData, localTimestamp);
            
            // 序列化为字节数组
            std::string serializedData = ProtobufConverter::serializeToString(protoMessage);
            
            if (!serializedData.empty()) {
                // 通过ZMQ发布protobuf格式的行情数据
                if (marketPublisher.publishMessage("MARKET_DATA_PROTOBUF", serializedData)) {
                    messageCount++;
                    zlog_info(cat, "[main] 成功发布行情数据到ZMQ: %s, 消息计数: %d", 
                              marketData.InstrumentID ? marketData.InstrumentID : "Unknown", messageCount);
                    if (messageCount % 1000 == 0) {
                        zlog_info(cat, "[main] 已发布 %d 条protobuf行情数据", messageCount);
                    }
                } else {
                    // 发送失败，放入缓存队列
                    if (!bufq.try_enqueue(marketData))
                    {
                        zlog_error(cat, "[main] 缓存队列已满，丢弃消息: %s", marketData.InstrumentID);
                    }
                }
            } else {
                zlog_error(cat, "[main] protobuf序列化失败: %s", marketData.InstrumentID);
            }
        }
        
        zlog_info(cat, "[main] 总共发布了 %d 条protobuf行情数据", messageCount);
    }
    else
    {
        zlog_error(cat, "[main] 行情API创建失败.");
        marketPublisher.disconnect();
        zlog_fini();
        return -1;
    }

    mdspi.Destroy();
    marketPublisher.disconnect();
    zlog_info(cat, "[main] 行情订阅程序正常退出");
    zlog_fini();

    return 0;
} 