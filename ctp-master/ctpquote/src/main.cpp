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

using namespace std;
using namespace moodycamel;

// external global variable
extern string currentPath;
extern vector<string>      contracts;
extern condition_variable  cv;
extern mutex               m;
extern bool                isReady;
extern string              pushServer;

ReaderWriterQueue<string>  q(10000);
// 缓存发送不成功的kline
ReaderWriterQueue<string>  bufq(500000);

// global variable
zlog_category_t *cat = nullptr;

// zmq instance
zmq::context_t zmqCtx(1);
zmq::socket_t pushSkt(zmqCtx, ZMQ_PUSH);

int main(int argc, char** argv)
{
    get_self_path();
    if (currentPath.empty())
    {
        cerr << "cannot get current exec path." << endl;
        return -1;
    }

    int rc = zlog_init((currentPath + "/zlog.conf").c_str());
    if (rc)
    {
        cerr << "can not find zlog configure file." << endl;
        return -1;
    }

    cat = zlog_get_category("my_cat");
    if (!cat)
    {
        cerr << "invalid zlog configure file format." << endl;
        zlog_fini();
        return -1;
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
        tdspi.Destroy();
    }
    else
    {
        zlog_info(cat, "[main] trader API failed.");
        zlog_fini();
        return -1;
    }

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

        // ctp is ready to send quote
        string ctp_msg = "EL/CTP_READY/CTP/H/1 || ctp_ready";
        zmq::message_t ctp_ready_msg(ctp_msg.c_str(), ctp_msg.length());
        pushSkt.send(ctp_ready_msg);

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
}