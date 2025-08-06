#ifndef ZMQ_PUBLISHER_H
#define ZMQ_PUBLISHER_H

#include <string>
#include <memory>
#include <zmq.hpp>

class ZMQPublisher {
private:
    std::unique_ptr<zmq::context_t> context;
    std::unique_ptr<zmq::socket_t> socket;
    std::string address;
    bool initialized;

public:
    // 构造函数
    ZMQPublisher(const std::string& addr = "tcp://127.0.0.1:8888");
    
    // 析构函数
    ~ZMQPublisher();
    
    // 初始化连接
    bool connect();
    
    // 发布消息（两帧：type + content）
    bool publishMessage(const std::string& messageType, const std::string& messageContent);
    
    // 发布消息（字节数组版本）
    bool publishMessage(const std::string& messageType, const std::vector<uint8_t>& messageContent);
    
    // 检查连接状态
    bool isConnected() const { return initialized; }
    
    // 获取地址
    std::string getAddress() const { return address; }
    
    // 设置地址
    void setAddress(const std::string& addr) { address = addr; }
    
    // 断开连接
    void disconnect();
};

#endif // ZMQ_PUBLISHER_H 