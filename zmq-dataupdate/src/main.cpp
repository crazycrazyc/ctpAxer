#include <iostream>
#include <signal.h>
#include <memory>
#include <chrono>
#include <thread>

#include "../include/ConfigManager.h"
#include "../include/Logger.h"
#include "../include/DatabaseManager.h"
#include "../include/ZMQSubscriber.h"

// 全局变量，用于信号处理
std::atomic<bool> g_running(true);

// 信号处理函数
void signalHandler(int signum) {
    std::cout << "\n收到信号 " << signum << "，正在关闭程序..." << std::endl;
    g_running = false;
}

// 自定义消息处理函数示例
void customMessageHandler(const std::string& messageType, const std::string& messageContent) {
    std::cout << "自定义处理 - 消息类型: " << messageType << ", 内容: " << messageContent << std::endl;
    
    // 这里可以添加开发者自定义的消息处理逻辑
    // 例如：解析JSON、处理特定格式的数据等
    
    if (messageType == "JSON") {
        // 处理JSON格式的消息
        std::cout << "处理JSON消息: " << messageContent << std::endl;
    } else if (messageType == "CSV") {
        // 处理CSV格式的消息
        std::cout << "处理CSV消息: " << messageContent << std::endl;
    } else if (messageType == "BINARY") {
        // 处理二进制消息
        std::cout << "处理二进制消息，长度: " << messageContent.length() << std::endl;
    }
}

// CTP合约处理函数已移除

int main(int argc, char* argv[]) {
    std::cout << "ZMQ数据更新订阅者启动..." << std::endl;
    
    // 设置信号处理
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    try {
        // 1. 加载配置
        std::string configFile = "config.ini";
        if (argc > 1) {
            configFile = argv[1];
        }
        
        ConfigManager config(configFile);
        if (!config.loadConfig()) {
            std::cerr << "加载配置文件失败，使用默认配置" << std::endl;
        }
        
        // 2. 初始化日志
        auto logger = std::make_shared<Logger>(
            config.getLogFile(), 
            static_cast<LogLevel>(config.getLogLevel())
        );
        logger->info("ZMQ数据更新订阅者启动");
        
        // 3. 初始化数据库连接
        auto dbManager = std::make_shared<DatabaseManager>(
            config.getDBHost(),
            config.getDBPort(),
            config.getDBUser(),
            config.getDBPassword(),
            config.getDBName()
        );
        
        if (!dbManager->connect()) {
            logger->warning("数据库连接失败，程序将继续运行但不保存数据");
        } else {
            logger->info("数据库连接成功");
        }
        
        // 4. 创建ZMQ订阅者
        auto subscriber = std::unique_ptr<ZMQSubscriber>(new ZMQSubscriber(config.getZMQAddress()));
        
        // 设置依赖
        subscriber->setLogger(logger);
        subscriber->setDatabaseManager(dbManager);
        
        // 设置自定义消息处理函数（可选）
        // subscriber->setMessageHandler(customMessageHandler);
        
        // 5. 启动订阅者
        subscriber->start();
        
        logger->info("程序启动完成，等待消息...");
        std::cout << "程序正在运行，按 Ctrl+C 退出" << std::endl;
        
        // 6. 主循环
        while (g_running) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            
            // 可以在这里添加定期任务
            // 例如：心跳检查、状态报告等
        }
        
        // 7. 清理资源
        logger->info("正在关闭程序...");
        subscriber->stop();
        
        logger->info("程序已安全关闭");
        
    } catch (const std::exception& e) {
        std::cerr << "程序异常: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "程序发生未知异常" << std::endl;
        return 1;
    }
    
    return 0;
} 