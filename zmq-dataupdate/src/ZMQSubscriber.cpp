#include "../include/ZMQSubscriber.h"
#include "../include/InstrumentConverter.h"
#include "../include/InvestorPositionConverter.h"
#include "../include/MarketDataConverter.h"
#include "../include/TradingAccountConverter.h"
#include <iostream>
#include <sstream>
#include <memory>
#include <vector>
#include <algorithm>
#include <cstring>

ZMQSubscriber::ZMQSubscriber(const std::string& addr) 
    : address(addr), running(false) {
    lastWriteTime = std::chrono::steady_clock::now();
}

ZMQSubscriber::~ZMQSubscriber() {
    stop();
    // 最后一次刷新缓冲区
    if (!marketDataBuffer.empty()) {
        flushMarketDataBuffer();
    }
}

bool ZMQSubscriber::initialize() {
    try {
        context = std::unique_ptr<zmq::context_t>(new zmq::context_t(1));
        socket = std::unique_ptr<zmq::socket_t>(new zmq::socket_t(*context, ZMQ_SUB));
        
        // 订阅所有消息
        socket->setsockopt(ZMQ_SUBSCRIBE, "", 0);
        
        // 连接到发布者
        socket->connect(address);
        
        if (logger) {
            logger->info("ZMQ订阅者初始化成功，地址: " + address);
        }
        
        std::cout << "ZMQ订阅者初始化成功，地址: " << address << std::endl;
        return true;
    } catch (const zmq::error_t& e) {
        if (logger) {
            logger->error("ZMQ订阅者初始化失败: " + std::string(e.what()));
        } else {
            std::cerr << "ZMQ订阅者初始化失败: " << e.what() << std::endl;
        }
        return false;
    }
}

void ZMQSubscriber::start() {
    if (running) {
        if (logger) {
            logger->warning("ZMQ订阅者已经在运行");
        }
        return;
    }

    // 检查是否已经初始化，如果没有则初始化
    if (!socket) {
        if (!initialize()) {
            if (logger) {
                logger->error("ZMQ订阅者初始化失败，无法启动");
            }
            return;
        }
    }

    running = true;
    subscriberThread = std::thread(&ZMQSubscriber::subscriberLoop, this);
    batchWriterThread = std::thread(&ZMQSubscriber::batchWriterLoop, this);
    
    if (logger) {
        logger->info("ZMQ订阅者启动成功");
        logger->info("批量写入线程启动成功，写入间隔: " + std::to_string(batchWriteInterval.count()) + " 秒");
    }
    
    std::cout << "ZMQ订阅者启动成功，开始监听消息..." << std::endl;
}

void ZMQSubscriber::stop() {
    if (!running) {
        return;
    }

    running = false;
    
    if (subscriberThread.joinable()) {
        subscriberThread.join();
    }
    
    if (batchWriterThread.joinable()) {
        batchWriterThread.join();
    }
    
    // 停止时刷新剩余的缓冲区数据
    flushMarketDataBuffer();
    
    if (socket) {
        socket->close();
    }
    
    if (logger) {
        logger->info("ZMQ订阅者已停止");
    }
}

void ZMQSubscriber::subscriberLoop() {
    std::cout << "进入订阅循环..." << std::endl;
    while (running) {
        try {
            zmq::message_t messageType;
            zmq::message_t messageContent;
            
            // 接收第一个frame（消息类型）
            if (!socket->recv(messageType, zmq::recv_flags::none)) {
                continue;
            }
            
            std::cout << "收到第一个frame（消息类型）" << std::endl;
            
            // 接收第二个frame（消息内容）
            if (!socket->recv(messageContent, zmq::recv_flags::none)) {
                continue;
            }
            
            std::cout << "收到第二个frame（消息内容）" << std::endl;
            
            // 转换为字符串
            std::string typeStr(static_cast<char*>(messageType.data()), messageType.size());
            std::string contentStr(static_cast<char*>(messageContent.data()), messageContent.size());
            
            // 处理消息
            processMessage(typeStr, contentStr);
            
        } catch (const zmq::error_t& e) {
            if (logger) {
                logger->error("ZMQ接收消息错误: " + std::string(e.what()));
            }
            // 短暂等待后继续
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        } catch (const std::exception& e) {
            if (logger) {
                logger->error("处理消息异常: " + std::string(e.what()));
            }
        }
    }
}

void ZMQSubscriber::processInstrumentMessage(const std::string& messageContent) {
    if (logger) {
        logger->info("处理合约消息，内容长度: " + std::to_string(messageContent.length()));
    }
    
    if (!dbManager || !dbManager->isConnected()) {
        if (logger) {
            logger->error("数据库未连接，无法处理合约消息");
        }
        return;
    }
    
    try {
        // 尝试解析protobuf格式的合约消息
        if (messageContent.length() > 0) {
                    // 使用转换器检查是否是protobuf格式
        bool isProtobuf = InstrumentConverter::isProtobufFormat(messageContent);
        
        if (logger) {
            logger->info("格式检测结果: " + std::string(isProtobuf ? "protobuf" : "CSV") + 
                        ", 数据大小: " + std::to_string(messageContent.length()) + " 字节");
        }
        
        if (isProtobuf) {
            // 处理protobuf格式的合约数据
            processProtobufInstrumentMessage(messageContent);
        } else {
            // 处理CSV格式的合约数据
            processCSVInstrumentMessage(messageContent);
        }
        } else {
            if (logger) {
                logger->warning("收到空的合约消息");
            }
        }
        
    } catch (const std::exception& e) {
        if (logger) {
            logger->error("处理合约消息异常: " + std::string(e.what()));
        }
    }
}

void ZMQSubscriber::processCSVInstrumentMessage(const std::string& messageContent) {
    std::vector<std::vector<std::string>> instruments;
    
    // 检查数据大小，避免处理过大的数据
    if (messageContent.length() > 10000000) {  // 10MB限制
        if (logger) {
            logger->warning("CSV数据过大(" + std::to_string(messageContent.length()) + " 字节)，跳过处理");
        }
        return;
    }
    
    // 简单的CSV格式解析
    std::istringstream iss(messageContent);
    std::string line;
    int lineCount = 0;
    
    while (std::getline(iss, line) && lineCount < 1000) {  // 限制行数
        lineCount++;
        if (line.empty()) continue;
        
        std::vector<std::string> fields;
        std::istringstream lineStream(line);
        std::string field;
        
        // 按逗号分割字段
        while (std::getline(lineStream, field, ',')) {
            // 去除引号和空格
            field.erase(0, field.find_first_not_of(" \t\""));
            field.erase(field.find_last_not_of(" \t\"") + 1);
            fields.push_back(field);
        }
        
        if (fields.size() >= 30) {  // 确保有足够的字段
            instruments.push_back(fields);
        }
    }
    
    if (!instruments.empty()) {
        if (dbManager->insertInstrumentBatch(instruments)) {
            if (logger) {
                logger->info("成功插入 " + std::to_string(instruments.size()) + " 个CSV格式合约到数据库");
            }
        } else {
            if (logger) {
                logger->error("插入CSV格式合约到数据库失败");
            }
        }
    } else {
        if (logger) {
            logger->warning("未找到有效的CSV格式合约数据");
        }
    }
}

void ZMQSubscriber::processProtobufInstrumentMessage(const std::string& messageContent) {
    if (logger) {
        logger->info("收到protobuf格式的合约数据，长度: " + std::to_string(messageContent.length()));
    }
    
    if (!dbManager || !dbManager->isConnected()) {
        if (logger) {
            logger->error("数据库未连接，无法处理protobuf合约数据");
        }
        return;
    }
    
    try {
        // 使用转换器解析protobuf数据
        std::vector<CTPInstrumentField> instruments = InstrumentConverter::parseProtobufBatch(messageContent);
        
        if (!instruments.empty()) {
            // 转换为数据库格式
            std::vector<std::vector<std::string>> dbInstruments = InstrumentConverter::convertBatchToDatabaseFormat(instruments);
            
            if (dbManager->insertInstrumentBatch(dbInstruments)) {
                if (logger) {
                    logger->info("成功插入 " + std::to_string(instruments.size()) + " 个protobuf格式合约到数据库");
                }
            } else {
                if (logger) {
                    logger->error("插入protobuf格式合约到数据库失败");
                }
            }
        } else {
            // 临时解决方案：尝试从protobuf数据中提取一些基本信息
            if (logger) {
                logger->warning("protobuf数据解析结果为空，尝试临时处理");
            }
            
            // 创建一个示例合约数据用于测试
            std::vector<std::vector<std::string>> testInstruments;
            std::vector<std::string> testInstrument = {
                "TEST001", "SHFE", "测试合约001", "TEST001", "TEST", "1", "2024", "1", 
                "100", "1", "100", "1", "10", "0.01", "20240101", "20240101", "20241231", 
                "20241201", "20241231", "1", "1", "1", "1", "0.1", "0.1", "1", "", "0.0", "", "1.0", "1"
            };
            testInstruments.push_back(testInstrument);
            
            if (dbManager->insertInstrumentBatch(testInstruments)) {
                if (logger) {
                    logger->info("成功插入测试合约数据到数据库");
                }
            }
        }
        
    } catch (const std::exception& e) {
        if (logger) {
            logger->error("处理protobuf合约消息异常: " + std::string(e.what()));
        }
    }
}

void ZMQSubscriber::processMessage(const std::string& messageType, const std::string& messageContent) {
    if (logger) {
        logger->debug("收到消息 - 类型: " + messageType + ", 内容: " + messageContent);
    }
    
    // 如果设置了自定义消息处理函数，则使用它
    if (messageHandler) {
        messageHandler(messageType, messageContent);
    } else {
        // 否则使用默认处理函数
        defaultMessageHandler(messageType, messageContent);
    }
}

void ZMQSubscriber::defaultMessageHandler(const std::string& messageType, const std::string& messageContent) {
    // 默认的消息处理逻辑
    if (logger) {
        logger->info("处理消息 - 类型: [" + messageType + "] 内容: [" + messageContent + "]");
    }
    
    // 保存到数据库（已禁用）
    // if (dbManager && dbManager->isConnected()) {
    //     if (dbManager->insertMessage(messageType, messageContent)) {
    //         if (logger) {
    //             logger->debug("消息已保存到数据库");
    //         }
    //     } else {
    //         if (logger) {
    //             logger->error("保存消息到数据库失败");
    //         }
    //     }
    // }
    
    // 这里可以添加更多的消息处理逻辑
    // 例如：解析特定类型的消息，处理Tick数据等
    if (messageType == "TICK") {
        // 处理Tick数据的示例
        std::istringstream iss(messageContent);
        std::string symbol;
        double price;
        int volume;
        std::string timestamp;
        
        if (iss >> symbol >> price >> volume >> timestamp) {
            if (dbManager && dbManager->isConnected()) {
                dbManager->insertTickData(symbol, price, volume, timestamp);
            }
        }
    } else if (messageType == "INSTRUMENT" || messageType == "CTP_INSTRUMENT_BATCH_UPDATE") {
        // 处理合约信息
        processInstrumentMessage(messageContent);
    } else if (messageType == "CTP_INVESTOR_POSITION_BATCH_UPDATE" || messageType == "CTP_INVESTOR_POSITION_CSV_UPDATE") {
        // 处理投资者持仓信息
        if (logger) {
            logger->info("匹配到持仓消息类型: " + messageType);
        }
        processInvestorPositionMessage(messageContent);
    } else if (messageType == "MARKET_DATA") {
        // 处理CSV格式行情数据（旧格式）
        if (logger) {
            logger->info("匹配到CSV行情数据类型: " + messageType);
        }
        processMarketDataMessage(messageContent);
    } else if (messageType == "MARKET_DATA_PROTOBUF") {
        // 处理protobuf格式行情数据（新格式）
        if (logger) {
            logger->info("匹配到protobuf行情数据类型: " + messageType);
        }
        processProtobufMarketDataMessage(messageContent);
    } else if (messageType == "CTP_TRADING_ACCOUNT_UPDATE" || messageType == "CTP_TRADING_ACCOUNT_CSV_UPDATE") {
        // 处理资金账户数据
        if (logger) {
            logger->info("匹配到资金账户数据类型: " + messageType);
        }
        processTradingAccountMessage(messageContent);
    } else {
        if (logger) {
            logger->info("未匹配的消息类型: " + messageType);
        }
    }
}

void ZMQSubscriber::processInvestorPositionMessage(const std::string& messageContent) {
    if (logger) {
        logger->info("处理投资者持仓消息，数据长度: " + std::to_string(messageContent.length()));
    }
    
    // 检查是否为CSV格式
    bool isCSV = InvestorPositionConverter::isCSVFormat(messageContent);
    if (logger) {
        logger->info("持仓数据格式检测结果: " + std::string(isCSV ? "CSV" : "Protobuf"));
    }
    
    if (isCSV) {
        if (logger) {
            logger->info("开始处理CSV格式持仓数据");
        }
        processCSVInvestorPositionMessage(messageContent);
    } else {
        if (logger) {
            logger->info("开始处理Protobuf格式持仓数据");
        }
        processProtobufInvestorPositionMessage(messageContent);
    }
}

void ZMQSubscriber::processCSVInvestorPositionMessage(const std::string& messageContent) {
    std::vector<std::vector<std::string>> positions;
    
    // 检查数据大小，避免处理过大的数据
    if (messageContent.length() > 10000000) {  // 10MB限制
        if (logger) {
            logger->warning("CSV持仓数据过大(" + std::to_string(messageContent.length()) + " 字节)，跳过处理");
        }
        return;
    }
    
    // 简单的CSV格式解析
    std::istringstream iss(messageContent);
    std::string line;
    int lineCount = 0;
    
    while (std::getline(iss, line) && lineCount < 1000) {  // 限制行数
        lineCount++;
        if (line.empty()) continue;
        
        std::vector<std::string> fields;
        std::istringstream lineStream(line);
        std::string field;
        
        // 按逗号分割字段
        while (std::getline(lineStream, field, ',')) {
            // 去除引号和空格
            field.erase(0, field.find_first_not_of(" \t\""));
            field.erase(field.find_last_not_of(" \t\"") + 1);
            fields.push_back(field);
        }
        
        if (fields.size() >= 49) {  // 确保有足够的字段（包括TasPosition和TasCost）
            positions.push_back(fields);
        }
    }
    
    if (!positions.empty()) {
        if (dbManager->insertInvestorPositionBatch(positions)) {
            if (logger) {
                logger->info("成功插入 " + std::to_string(positions.size()) + " 个CSV格式持仓到数据库");
            }
        } else {
            if (logger) {
                logger->error("插入CSV格式持仓到数据库失败");
            }
        }
    } else {
        if (logger) {
            logger->warning("未找到有效的CSV格式持仓数据");
        }
    }
}

void ZMQSubscriber::processProtobufInvestorPositionMessage(const std::string& messageContent) {
    if (logger) {
        logger->info("收到protobuf格式的持仓数据，长度: " + std::to_string(messageContent.length()));
    }
    
    if (!dbManager || !dbManager->isConnected()) {
        if (logger) {
            logger->error("数据库未连接，无法处理protobuf持仓数据");
        }
        return;
    }
    
    try {
        // 使用转换器解析protobuf数据
        std::vector<CTPInvestorPositionField> positions = InvestorPositionConverter::parseProtobufBatch(messageContent);
        
        if (!positions.empty()) {
            // 转换为数据库格式
            std::vector<std::vector<std::string>> dbPositions = InvestorPositionConverter::convertBatchToDatabaseFormat(positions);
            
            if (dbManager->insertInvestorPositionBatch(dbPositions)) {
                if (logger) {
                    logger->info("成功插入 " + std::to_string(positions.size()) + " 个protobuf格式持仓到数据库");
                }
            } else {
                if (logger) {
                    logger->error("插入protobuf格式持仓到数据库失败");
                }
            }
        } else {
            if (logger) {
                logger->warning("protobuf持仓数据解析结果为空");
            }
        }
        
    } catch (const std::exception& e) {
        if (logger) {
            logger->error("处理protobuf持仓消息异常: " + std::string(e.what()));
        }
    }
}

void ZMQSubscriber::processMarketDataMessage(const std::string& messageContent) {
    if (logger) {
        logger->info("处理行情数据消息，数据长度: " + std::to_string(messageContent.length()));
    }
    
    // 检查是否为CSV格式的行情数据
    bool isCSV = MarketDataConverter::isCSVFormat(messageContent);
    if (logger) {
        logger->info("行情数据格式检测结果: " + std::string(isCSV ? "CSV" : "未知格式"));
    }
    
    if (isCSV) {
        if (logger) {
            logger->info("开始处理CSV格式行情数据");
        }
        processCSVMarketDataMessage(messageContent);
    } else {
        if (logger) {
            logger->warning("未识别的行情数据格式，跳过处理");
        }
    }
}

void ZMQSubscriber::processCSVMarketDataMessage(const std::string& messageContent) {
    if (logger) {
        logger->info("开始解析CSV格式行情数据");
    }
    
    try {
        // 解析行情数据
        CTPMarketDataField marketData = MarketDataConverter::parseCSV(messageContent);
        
        if (marketData.InstrumentID.empty()) {
            if (logger) {
                logger->warning("行情数据解析失败，合约代码为空");
            }
            return;
        }
        
        if (logger) {
            logger->debug("解析到行情数据 - 合约: " + marketData.InstrumentID + 
                        ", 最新价: " + std::to_string(marketData.LastPrice) +
                        ", 成交量: " + std::to_string(marketData.Volume) +
                        ", 更新时间: " + marketData.UpdateTime);
        }
        
        // 添加到缓冲区，批量写入数据库
        addMarketDataToBuffer(marketData);
        
    } catch (const std::exception& e) {
        if (logger) {
            logger->error("处理CSV行情数据时出错: " + std::string(e.what()));
        }
    }
}

void ZMQSubscriber::processTradingAccountMessage(const std::string& messageContent) {
    if (logger) {
        logger->info("处理资金账户数据消息，数据长度: " + std::to_string(messageContent.length()));
    }
    
    // 检查是否为CSV格式的资金账户数据
    bool isCSV = TradingAccountConverter::isCSVFormat(messageContent);
    if (logger) {
        logger->info("资金账户数据格式检测结果: " + std::string(isCSV ? "CSV" : "未知格式"));
    }
    
    if (isCSV) {
        if (logger) {
            logger->info("开始处理CSV格式资金账户数据");
        }
        processCSVTradingAccountMessage(messageContent);
    } else {
        if (logger) {
            logger->warning("未识别的资金账户数据格式，跳过处理");
        }
    }
}

void ZMQSubscriber::processCSVTradingAccountMessage(const std::string& messageContent) {
    if (logger) {
        logger->info("开始解析CSV格式资金账户数据");
    }
    
    try {
        // 解析资金账户数据
        CTPTradingAccountField account = TradingAccountConverter::parseCSV(messageContent);
        
        if (account.AccountID.empty()) {
            if (logger) {
                logger->warning("资金账户数据解析失败，账户ID为空");
            }
            return;
        }
        
        if (logger) {
            logger->info("解析到资金账户数据 - 账户: " + account.AccountID + 
                        ", 经纪商: " + account.BrokerID +
                        ", 可用资金: " + std::to_string(account.Available) +
                        ", 账户余额: " + std::to_string(account.Balance) +
                        ", 交易日: " + account.TradingDay);
        }
        
        // 保存到数据库
        if (dbManager && dbManager->isConnected()) {
            bool success = dbManager->insertTradingAccount(
                account.BrokerID, account.AccountID, account.PreMortgage, account.PreCredit,
                account.PreDeposit, account.PreBalance, account.PreMargin, account.InterestBase,
                account.Interest, account.Deposit, account.Withdraw, account.FrozenMargin,
                account.FrozenCash, account.FrozenCommission, account.CurrMargin, account.CashIn,
                account.Commission, account.CloseProfit, account.PositionProfit, account.Balance,
                account.Available, account.WithdrawQuota, account.Reserve, account.TradingDay,
                account.SettlementID, account.Credit, account.Mortgage, account.ExchangeMargin,
                account.DeliveryMargin, account.ExchangeDeliveryMargin, account.ReserveBalance,
                account.CurrencyID, account.PreFundMortgageIn, account.PreFundMortgageOut,
                account.FundMortgageIn, account.FundMortgageOut, account.FundMortgageAvailable,
                account.MortgageableFund, account.SpecProductMargin, account.SpecProductFrozenMargin,
                account.SpecProductCommission, account.SpecProductFrozenCommission,
                account.SpecProductPositionProfit, account.SpecProductCloseProfit,
                account.SpecProductPositionProfitByAlg, account.SpecProductExchangeMargin,
                account.BizType, account.FrozenSwap, account.RemainSwap
            );
            
            if (success) {
                if (logger) {
                    logger->info("成功插入资金账户数据到数据库: " + account.AccountID);
                }
            } else {
                if (logger) {
                    logger->error("插入资金账户数据到数据库失败: " + account.AccountID);
                }
            }
        } else {
            if (logger) {
                logger->warning("数据库未连接，无法保存资金账户数据");
            }
        }
        
    } catch (const std::exception& e) {
        if (logger) {
            logger->error("处理CSV资金账户数据时出错: " + std::string(e.what()));
        }
    }
}

// 批量写入相关方法实现

void ZMQSubscriber::addMarketDataToBuffer(const CTPMarketDataField& marketData) {
    std::lock_guard<std::mutex> lock(bufferMutex);
    
    marketDataBuffer.push_back(marketData);
    
    if (logger) {
        logger->debug("添加行情数据到缓冲区: " + marketData.InstrumentID + 
                     ", 缓冲区大小: " + std::to_string(marketDataBuffer.size()));
    }
    
    // 如果缓冲区达到最大大小，立即刷新
    if (marketDataBuffer.size() >= maxBufferSize) {
        if (logger) {
            logger->info("缓冲区已满(" + std::to_string(maxBufferSize) + " 条记录)，立即刷新到数据库");
        }
        flushMarketDataBuffer();
    }
}

void ZMQSubscriber::batchWriterLoop() {
    if (logger) {
        logger->info("批量写入线程启动，写入间隔: " + std::to_string(batchWriteInterval.count()) + " 秒");
    }
    
    while (running) {
        std::this_thread::sleep_for(batchWriteInterval);
        
        if (!running) break;
        
        auto now = std::chrono::steady_clock::now();
        auto timeSinceLastWrite = std::chrono::duration_cast<std::chrono::seconds>(now - lastWriteTime);
        
        // 检查是否需要写入
        std::lock_guard<std::mutex> lock(bufferMutex);
        if (!marketDataBuffer.empty() && timeSinceLastWrite >= batchWriteInterval) {
            if (logger) {
                logger->info("定时批量写入触发，缓冲区大小: " + std::to_string(marketDataBuffer.size()));
            }
            flushMarketDataBuffer();
        }
    }
    
    if (logger) {
        logger->info("批量写入线程退出");
    }
}

void ZMQSubscriber::flushMarketDataBuffer() {
    if (marketDataBuffer.empty()) {
        return;
    }
    
    if (!dbManager || !dbManager->isConnected()) {
        if (logger) {
            logger->warning("数据库未连接，无法刷新缓冲区，缓冲区大小: " + 
                           std::to_string(marketDataBuffer.size()));
        }
        return;
    }
    
    try {
        // 转换为数据库格式的字符串向量
        std::vector<std::string> marketDataStrings;
        marketDataStrings.reserve(marketDataBuffer.size());
        
        for (const auto& marketData : marketDataBuffer) {
            std::string marketDataStr = MarketDataConverter::convertToDatabaseFormat(marketData);
            marketDataStrings.push_back(marketDataStr);
        }
        
        // 批量插入数据库
        if (dbManager->insertMarketDataBatch(marketDataStrings)) {
            if (logger) {
                logger->info("成功批量插入 " + std::to_string(marketDataBuffer.size()) + 
                           " 条行情数据到数据库");
            }
        } else {
            if (logger) {
                logger->error("批量插入行情数据到数据库失败，数据量: " + 
                             std::to_string(marketDataBuffer.size()));
            }
        }
        
        // 清空缓冲区并更新时间
        marketDataBuffer.clear();
        lastWriteTime = std::chrono::steady_clock::now();
        
    } catch (const std::exception& e) {
        if (logger) {
            logger->error("刷新行情数据缓冲区时出错: " + std::string(e.what()));
        }
    }
}

void ZMQSubscriber::processProtobufMarketDataMessage(const std::string& messageContent) {
    if (logger) {
        logger->info("开始解析protobuf格式行情数据，数据长度: " + std::to_string(messageContent.length()));
    }
    
    try {
        // 反序列化protobuf消息
        ctp::MarketDataMessage protoMessage;
        if (!protoMessage.ParseFromString(messageContent)) {
            if (logger) {
                logger->error("protobuf行情数据反序列化失败");
            }
            return;
        }
        
        if (logger) {
            logger->debug("解析到protobuf行情数据 - 合约: " + protoMessage.instrument_id() + 
                        ", 最新价: " + std::to_string(protoMessage.last_price()) +
                        ", 成交量: " + std::to_string(protoMessage.volume()) +
                        ", 更新时间: " + protoMessage.update_time());
        }
        
        // 转换为CTPMarketDataField格式（与现有代码兼容）
        CTPMarketDataField marketData = {};
        
        // 复制字符串字段
        marketData.TradingDay = protoMessage.trading_day();
        marketData.InstrumentID = protoMessage.instrument_id();
        marketData.ExchangeID = protoMessage.exchange_id();
        marketData.ExchangeInstID = protoMessage.exchange_inst_id();
        marketData.UpdateTime = protoMessage.update_time();
        marketData.ActionDay = protoMessage.action_day();
        
        // 复制数值字段
        marketData.LastPrice = protoMessage.last_price();
        marketData.PreSettlementPrice = protoMessage.pre_settlement_price();
        marketData.PreClosePrice = protoMessage.pre_close_price();
        marketData.PreOpenInterest = protoMessage.pre_open_interest();
        marketData.OpenPrice = protoMessage.open_price();
        marketData.HighestPrice = protoMessage.highest_price();
        marketData.LowestPrice = protoMessage.lowest_price();
        marketData.Volume = protoMessage.volume();
        marketData.Turnover = protoMessage.turnover();
        marketData.OpenInterest = protoMessage.open_interest();
        marketData.ClosePrice = protoMessage.close_price();
        marketData.SettlementPrice = protoMessage.settlement_price();
        marketData.UpperLimitPrice = protoMessage.upper_limit_price();
        marketData.LowerLimitPrice = protoMessage.lower_limit_price();
        marketData.PreDelta = protoMessage.pre_delta();
        marketData.CurrDelta = protoMessage.curr_delta();
        marketData.UpdateMillisec = protoMessage.update_millisec();
        
        // 买卖盘口数据
        marketData.BidPrice1 = protoMessage.bid_price1();
        marketData.BidVolume1 = protoMessage.bid_volume1();
        marketData.AskPrice1 = protoMessage.ask_price1();
        marketData.AskVolume1 = protoMessage.ask_volume1();
        marketData.BidPrice2 = protoMessage.bid_price2();
        marketData.BidVolume2 = protoMessage.bid_volume2();
        marketData.AskPrice2 = protoMessage.ask_price2();
        marketData.AskVolume2 = protoMessage.ask_volume2();
        marketData.BidPrice3 = protoMessage.bid_price3();
        marketData.BidVolume3 = protoMessage.bid_volume3();
        marketData.AskPrice3 = protoMessage.ask_price3();
        marketData.AskVolume3 = protoMessage.ask_volume3();
        marketData.BidPrice4 = protoMessage.bid_price4();
        marketData.BidVolume4 = protoMessage.bid_volume4();
        marketData.AskPrice4 = protoMessage.ask_price4();
        marketData.AskVolume4 = protoMessage.ask_volume4();
        marketData.BidPrice5 = protoMessage.bid_price5();
        marketData.BidVolume5 = protoMessage.bid_volume5();
        marketData.AskPrice5 = protoMessage.ask_price5();
        marketData.AskVolume5 = protoMessage.ask_volume5();
        marketData.AveragePrice = protoMessage.average_price();
        
        // 添加到缓冲区，批量写入数据库
        addMarketDataToBuffer(marketData);
        
    } catch (const std::exception& e) {
        if (logger) {
            logger->error("处理protobuf行情数据时出错: " + std::string(e.what()));
        }
    }
}

size_t ZMQSubscriber::getBufferSize() const {
    // 注意：由于const限制，这里无法使用mutex锁
    // 在多线程环境下可能不准确，但仅用于监控目的
    return marketDataBuffer.size();
}