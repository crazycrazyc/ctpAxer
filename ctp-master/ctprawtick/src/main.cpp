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
#include "CTPTrader.h"
#include "CTPQuote.h"
#include "Config.h"
#include "readerwriterqueue.h"
#include <signal.h>

using namespace std;
using namespace moodycamel;

// external global variable
extern string currentPath;
extern vector<string>      contracts;
extern condition_variable  cv;
extern mutex               m;
extern bool                isReady;
extern bool                isPositionReady;
extern bool                isTradingAccountReady;
extern string              pushServer;

ReaderWriterQueue<string>  q(10000);
// 缓存发送不成功的kline
ReaderWriterQueue<string>  bufq(500000);

// global variable
zlog_category_t *cat = nullptr;

void signal_handler(int signal)
{
    std::cout << "signal_handler" << std::endl;
    //zlog_info(cat, "[signal_handler] signal %d received.", signal);
   // exit(0);
}

int main(int argc, char** argv)
{
    //signal(SIGUSR1, signal_handler);
    get_self_path();
    if (currentPath.empty())
    {
        cerr << "cannot get current exec path." << endl;
        //return -1;
    }

    int rc = zlog_init((currentPath + "/zlog.conf").c_str());
    if (rc)
    {
        cerr << "can not find zlog configure file." << endl;
        //return -1;
    }

    cat = zlog_get_category("my_cat");
    if (!cat)
    {
        cerr << "invalid zlog configure file format." << endl;
        zlog_fini();
        //return -1;
    }

    zlog_info(cat, "[main] load zlog configure file successfully.");
    CAppConfig appConfig;

    CTPTraderSpi tdspi;
    if (tdspi.Create(appConfig))
    {
        std::unique_lock<std::mutex> lock(m);

        // 等待获取全部合约名称
        cv.wait(lock, [] { return isReady; });
        lock.unlock();
        zlog_info(cat, "[main] 合约查询完成，等待持仓查询...");
        
        // 等待持仓查询完成（最多等待30秒）
        std::unique_lock<std::mutex> posLock(m);
        bool positionReceived = cv.wait_for(posLock, std::chrono::seconds(30), [] { return isPositionReady; });
        posLock.unlock();
        
        if (positionReceived) {
            zlog_info(cat, "[main] 持仓查询完成，开始资金账户查询...");
        } else {
            zlog_info(cat, "[main] 持仓查询超时（30秒），可能没有持仓数据，继续资金账户查询...");
        }
        
        // 查询资金账户
        tdspi.ReqTradingAccount();
        
        // 等待资金账户查询完成（最多等待5秒）
        std::unique_lock<std::mutex> accountLock(m);
        bool accountReceived = cv.wait_for(accountLock, std::chrono::seconds(5), [] { return isTradingAccountReady; });
        accountLock.unlock();
        
        if (accountReceived) {
            zlog_info(cat, "[main] 资金账户查询完成，交易部分完成");
        } else {
            zlog_info(cat, "[main] 资金账户查询超时（5秒），交易部分完成");
        }
        
        tdspi.Destroy();
    }
    else
    {
        zlog_info(cat, "[main] trader API failed.");
        zlog_fini();
        return -1;
    }
    
    zlog_info(cat, "[main] 交易功能完成，程序退出");
    zlog_fini();
    return 0;  // 只运行交易部分
  
    /*
    // 行情部分已分离到 main_market.cpp，避免功能重复
    // 如需行情功能，请单独运行 ctpmarket 程序
    CTPMarketSpi mdspi;
    if (mdspi.Create(appConfig))
    {
        pushSkt.setsockopt(ZMQ_SNDHWM, 10000);
        pushSkt.setsockopt(ZMQ_SNDTIMEO, 200);
		// pushSkt.connect("ipc:///home/hurui/zmqservice/quotegather.ipc");
        if (!pushServer.empty())
        {
            pushSkt.connect(pushServer);
        }
        else
        {
            cerr << "ZMQ Server not specified." << endl;
            mdspi.Destroy();
            zlog_fini();
            return -1;
        }

        chrono::seconds dura(1);
        string msg;
        for (;;)
        {
            time_t now = time(nullptr);
            if (mdspi.exitTs < now)
            {
                break;
            }

            while (bufq.try_dequeue(msg))
            {
                zmq::message_t message(msg.c_str(), msg.length());

                if (!pushSkt.send(message))
                {
                    if (!bufq.try_enqueue(msg))
                    {
                        cerr << msg << endl;
                    }
                }
                msg.clear();
            }
            msg.clear();

            if (!q.try_dequeue(msg))
            {
                this_thread::sleep_for(dura);
                continue;
            }
            // zlog_info(cat, "[msg] %s", msg.c_str());

            zmq::message_t message(msg.c_str(), msg.length());
            if (!pushSkt.send(message))
            {
                if (!bufq.try_enqueue(msg))
                {
                    cerr << msg << endl;
                }
            }
            msg.clear();
        }
    }

    mdspi.Destroy();
    
    zlog_fini();

    return 0;
    */
}