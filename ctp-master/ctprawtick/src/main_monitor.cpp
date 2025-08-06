#include <iostream>
#include <chrono>
#include <thread>
#include <string>
#include <vector>
#include <condition_variable>
#include <mutex>
#include <atomic>
#include <ctime>
#include <iomanip>
#include <sstream>
#include "zlog.h"
#include "utils.h"
#include "CTPTrader.h"
#include "Config.h"
#include <signal.h>

using namespace std;

// global variable
extern string currentPath;
extern condition_variable cv;
extern mutex m;
extern bool isPositionReady;
extern bool isTradingAccountReady;
zlog_category_t *cat = nullptr;

// 监控器状态
atomic<bool> running(true);
atomic<bool> ctpConnected(false);

// CTP交易接口实例
CTPTraderSpi* tdspi = nullptr;
CAppConfig* appConfig = nullptr;

string getCurrentTimeString() {
    auto now = chrono::system_clock::now();
    auto time_t = chrono::system_clock::to_time_t(now);
    auto ms = chrono::duration_cast<chrono::milliseconds>(now.time_since_epoch()) % 1000;
    
    stringstream ss;
    ss << put_time(localtime(&time_t), "%H:%M:%S");
    ss << "." << setfill('0') << setw(3) << ms.count();
    return ss.str();
}

bool isInTradingSession() {
    time_t now = time(0);
    tm* ltm = localtime(&now);
    int hour = ltm->tm_hour;
    int minute = ltm->tm_min;
    int timeInMinutes = hour * 60 + minute;
    
    // 交易时间段
    // 日盘：9:00-15:00
    // 夜盘：21:00-次日2:30
    int day_start = 9 * 60;           // 9:00
    int day_end = 15 * 60;            // 15:00
    int night_start = 21 * 60;        // 21:00
    int night_end = 2 * 60 + 30;      // 2:30 (次日)
    
    // 白天交易时间
    if (timeInMinutes >= day_start && timeInMinutes <= day_end) {
        return true;
    }
    
    // 夜盘交易时间（跨日）
    if (timeInMinutes >= night_start || timeInMinutes <= night_end) {
        return true;
    }
    
    return false;
}

// 持仓查询线程
void positionQueryThread() {
    zlog_info(cat, "[positionQueryThread] 持仓查询线程启动");
    
    auto lastQueryTime = chrono::steady_clock::now();
    const auto queryInterval = chrono::seconds(30); // 30秒查询一次
    
    while (running) {
        auto now = chrono::steady_clock::now();
        
        // 检查是否到了查询时间
        if (now - lastQueryTime >= queryInterval) {
            if (ctpConnected && isInTradingSession()) {
                zlog_info(cat, "[positionQueryThread] 开始查询持仓 - %s", getCurrentTimeString().c_str());
                
                // 重置标志
                isPositionReady = false;
                
                // 发起持仓查询
                tdspi->ReqInvestorPositions();
                
                // 等待查询完成（最多等待10秒）
                std::unique_lock<std::mutex> lock(m);
                bool success = cv.wait_for(lock, chrono::seconds(10), [] { return isPositionReady; });
                lock.unlock();
                
                if (success) {
                    zlog_info(cat, "[positionQueryThread] 持仓查询完成");
                } else {
                    zlog_warn(cat, "[positionQueryThread] 持仓查询超时");
                }
                
                lastQueryTime = now;
            } else if (!isInTradingSession()) {
                zlog_debug(cat, "[positionQueryThread] 非交易时间，跳过持仓查询");
                lastQueryTime = now; // 更新时间，避免频繁日志
            }
        }
        
        // 短暂休眠，避免CPU占用过高
        this_thread::sleep_for(chrono::seconds(1));
    }
    
    zlog_info(cat, "[positionQueryThread] 持仓查询线程退出");
}

// 资金查询线程
void accountQueryThread() {
    zlog_info(cat, "[accountQueryThread] 资金查询线程启动");
    
    auto lastQueryTime = chrono::steady_clock::now();
    const auto queryInterval = chrono::seconds(10); // 10秒查询一次
    
    while (running) {
        auto now = chrono::steady_clock::now();
        
        // 检查是否到了查询时间
        if (now - lastQueryTime >= queryInterval) {
            if (ctpConnected && isInTradingSession()) {
                zlog_info(cat, "[accountQueryThread] 开始查询资金 - %s", getCurrentTimeString().c_str());
                
                // 重置标志
                isTradingAccountReady = false;
                
                // 发起资金查询
                tdspi->ReqTradingAccount();
                
                // 等待查询完成（最多等待5秒）
                std::unique_lock<std::mutex> lock(m);
                bool success = cv.wait_for(lock, chrono::seconds(5), [] { return isTradingAccountReady; });
                lock.unlock();
                
                if (success) {
                    zlog_info(cat, "[accountQueryThread] 资金查询完成");
                } else {
                    zlog_warn(cat, "[accountQueryThread] 资金查询超时");
                }
                
                lastQueryTime = now;
            } else if (!isInTradingSession()) {
                zlog_debug(cat, "[accountQueryThread] 非交易时间，跳过资金查询");
                lastQueryTime = now; // 更新时间，避免频繁日志
            }
        }
        
        // 短暂休眠，避免CPU占用过高
        this_thread::sleep_for(chrono::seconds(1));
    }
    
    zlog_info(cat, "[accountQueryThread] 资金查询线程退出");
}

// 连接监控线程
void connectionMonitorThread() {
    zlog_info(cat, "[connectionMonitorThread] 连接监控线程启动");
    
    while (running) {
        if (tdspi) {
            // 这里可以添加连接状态检查逻辑
            // 暂时假设连接正常
            if (!ctpConnected) {
                ctpConnected = true;
                zlog_info(cat, "[connectionMonitorThread] CTP连接已建立");
            }
        }
        
        // 每5秒检查一次连接状态
        this_thread::sleep_for(chrono::seconds(5));
    }
    
    zlog_info(cat, "[connectionMonitorThread] 连接监控线程退出");
}

void signal_handler(int signal) {
    zlog_info(cat, "[signal_handler] 收到信号 %d，准备退出...", signal);
    running = false;
}

int main(int argc, char** argv) {
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    get_self_path();
    if (currentPath.empty()) {
        cerr << "无法获取当前执行路径." << endl;
        return -1;
    }

    int rc = zlog_init((currentPath + "/zlog.conf").c_str());
    if (rc) {
        cerr << "找不到zlog配置文件." << endl;
        return -1;
    }

    cat = zlog_get_category("my_cat");
    if (!cat) {
        cerr << "无效的zlog配置文件格式." << endl;
        zlog_fini();
        return -1;
    }

    zlog_info(cat, "[main] ========== 持仓资金监控器启动 ==========");
    zlog_info(cat, "[main] 当前时间: %s", getCurrentTimeString().c_str());
    zlog_info(cat, "[main] 交易时间状态: %s", isInTradingSession() ? "交易中" : "非交易时间");

    // 创建配置和交易接口
    appConfig = new CAppConfig();
    tdspi = new CTPTraderSpi();
    
    if (tdspi->Create(*appConfig)) {
        zlog_info(cat, "[main] 交易API创建成功");
        
        // 启动监控线程
        thread positionThread(positionQueryThread);
        thread accountThread(accountQueryThread);
        thread connectionThread(connectionMonitorThread);
        
        zlog_info(cat, "[main] 所有监控线程已启动");
        
        // 主循环 - 监控程序状态
        while (running) {
            this_thread::sleep_for(chrono::seconds(10));
            
            // 每10秒输出一次状态
            zlog_info(cat, "[main] 监控器运行中 - 时间: %s, CTP连接: %s, 交易状态: %s", 
                     getCurrentTimeString().c_str(),
                     ctpConnected ? "已连接" : "未连接",
                     isInTradingSession() ? "交易中" : "非交易时间");
        }
        
        zlog_info(cat, "[main] 正在停止监控线程...");
        
        // 等待线程结束
        positionThread.join();
        accountThread.join();
        connectionThread.join();
        
        zlog_info(cat, "[main] 所有监控线程已停止");
        
        tdspi->Destroy();
        delete tdspi;
        delete appConfig;
    } else {
        zlog_error(cat, "[main] 交易API创建失败");
        delete tdspi;
        delete appConfig;
        zlog_fini();
        return -1;
    }
    
    zlog_info(cat, "[main] ========== 持仓资金监控器退出 ==========");
    zlog_fini();
    return 0;
} 