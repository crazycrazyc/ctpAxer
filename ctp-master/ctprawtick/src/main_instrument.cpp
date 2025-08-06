#include <iostream>
#include <chrono>
#include <thread>
#include <string>
#include <vector>
#include <condition_variable>
#include <mutex>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <fstream>
#include "zlog.h"
#include "utils.h"
#include "CTPTrader.h"
#include "Config.h"
#include <signal.h>

using namespace std;

// global variable
extern string currentPath;
extern vector<string> contracts;
extern condition_variable cv;
extern mutex m;
extern bool isReady;
zlog_category_t *cat = nullptr;

// 运行计数器文件路径
string getCounterFilePath() {
    time_t now = time(0);
    tm* ltm = localtime(&now);
    
    ostringstream oss;
    oss << currentPath << "/instrument_query_" 
        << (1900 + ltm->tm_year) << "-"
        << setfill('0') << setw(2) << (1 + ltm->tm_mon) << "-"
        << setfill('0') << setw(2) << ltm->tm_mday << ".count";
    
    return oss.str();
}

int getRunCount() {
    string counterFile = getCounterFilePath();
    ifstream file(counterFile);
    if (!file.is_open()) {
        return 0;
    }
    
    int count = 0;
    file >> count;
    file.close();
    return count;
}

void updateRunCount(int count) {
    string counterFile = getCounterFilePath();
    ofstream file(counterFile);
    if (file.is_open()) {
        file << count;
        file.close();
    }
}

string getCurrentTimeString() {
    time_t now = time(0);
    tm* ltm = localtime(&now);
    
    ostringstream oss;
    oss << setfill('0') << setw(2) << ltm->tm_hour << ":"
        << setfill('0') << setw(2) << ltm->tm_min << ":"
        << setfill('0') << setw(2) << ltm->tm_sec;
    
    return oss.str();
}

bool isInTradingTime() {
    time_t now = time(0);
    tm* ltm = localtime(&now);
    int hour = ltm->tm_hour;
    int minute = ltm->tm_min;
    int timeInMinutes = hour * 60 + minute;
    
    // 早盘时间：8:30-9:00 或 晚盘收盘后：15:30-16:00
    int morning_start = 8 * 60 + 30;   // 8:30
    int morning_end = 9 * 60;          // 9:00
    int evening_start = 15 * 60 + 30;  // 15:30
    int evening_end = 16 * 60;         // 16:00
    
    return (timeInMinutes >= morning_start && timeInMinutes <= morning_end) ||
           (timeInMinutes >= evening_start && timeInMinutes <= evening_end);
}

void signal_handler(int signal) {
    zlog_info(cat, "[signal_handler] 收到信号 %d，程序退出", signal);
    exit(0);
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

    zlog_info(cat, "[main] ========== 合约查询器启动 ==========");
    zlog_info(cat, "[main] 当前时间: %s", getCurrentTimeString().c_str());
    
    // 检查今日运行次数
    int runCount = getRunCount();
    zlog_info(cat, "[main] 今日已运行次数: %d", runCount);
    
    if (runCount >= 2) {
        zlog_info(cat, "[main] 今日已运行2次，程序退出");
        zlog_fini();
        return 0;
    }
    
    // 检查是否在合适的时间运行
    if (!isInTradingTime()) {
        zlog_info(cat, "[main] 当前时间不适合查询合约（建议在8:30-9:00或15:30-16:00运行）");
        // 但不强制退出，允许手动运行
    }
    
    // 更新运行计数
    updateRunCount(runCount + 1);
    zlog_info(cat, "[main] 开始第 %d 次合约查询", runCount + 1);

    CAppConfig appConfig;
    CTPTraderSpi tdspi;
    
    if (tdspi.Create(appConfig)) {
        zlog_info(cat, "[main] 交易API创建成功，开始查询合约...");
        
        std::unique_lock<std::mutex> lock(m);
        
        // 等待获取全部合约名称
        bool success = cv.wait_for(lock, std::chrono::minutes(5), [] { return isReady; });
        lock.unlock();
        
        if (success) {
            zlog_info(cat, "[main] 合约查询完成，共获取 %zu 个合约", contracts.size());
            zlog_info(cat, "[main] 合约数据已通过ZMQ发布");
        } else {
            zlog_error(cat, "[main] 合约查询超时（5分钟），可能网络有问题");
        }
        
        tdspi.Destroy();
    } else {
        zlog_error(cat, "[main] 交易API创建失败");
        zlog_fini();
        return -1;
    }
    
    zlog_info(cat, "[main] ========== 合约查询器完成，程序退出 ==========");
    zlog_fini();
    return 0;
} 