#include "../include/ZMQPublisher.h"
#include <iostream>
#include <chrono>
#include <thread>

ZMQPublisher::ZMQPublisher(const std::string& addr) 
    : address(addr), initialized(false) {
}

ZMQPublisher::~ZMQPublisher() {
    disconnect();
}

bool ZMQPublisher::connect() {
    try {
        // 创建ZMQ上下文
        context = std::unique_ptr<zmq::context_t>(new zmq::context_t(1));
        
        // 创建发布者socket
        socket = std::unique_ptr<zmq::socket_t>(new zmq::socket_t(*context, ZMQ_PUB));
        
        // 设置socket选项
        int linger = 0;
        socket->setsockopt(ZMQ_LINGER, &linger, sizeof(linger));
        
        // 绑定地址
        socket->bind(address);
        
        // 等待一小段时间确保绑定完成
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        initialized = true;
        
        std::cout << "[ZMQPublisher::connect] 成功连接到: " << address << std::endl;
        return true;
        
    } catch (const zmq::error_t& e) {
        std::cerr << "[ZMQPublisher::connect] ZMQ连接失败: " << e.what() << std::endl;
        initialized = false;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "[ZMQPublisher::connect] 连接异常: " << e.what() << std::endl;
        initialized = false;
        return false;
    }
}

bool ZMQPublisher::publishMessage(const std::string& messageType, const std::string& messageContent) {
    if (!initialized || !socket) {
        std::cerr << "[ZMQPublisher::publishMessage] 发布者未初始化" << std::endl;
        return false;
    }
    
    try {
        // 创建第一帧：消息类型
        zmq::message_t typeMsg(messageType.c_str(), messageType.length());
        
        // 创建第二帧：消息内容
        zmq::message_t contentMsg(messageContent.c_str(), messageContent.length());
        
        // 发送第一帧（带more标志）
        socket->send(typeMsg, zmq::send_flags::sndmore);
        
        // 发送第二帧（最后一帧）
        socket->send(contentMsg);
        
        std::cout << "[ZMQPublisher::publishMessage] 成功发布消息 - 类型: " << messageType 
                  << ", 大小: " << messageContent.length() << " 字节" << std::endl;
        return true;
        
    } catch (const zmq::error_t& e) {
        std::cerr << "[ZMQPublisher::publishMessage] ZMQ发送失败: " << e.what() << std::endl;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "[ZMQPublisher::publishMessage] 发送异常: " << e.what() << std::endl;
        return false;
    }
}

bool ZMQPublisher::publishMessage(const std::string& messageType, const std::vector<uint8_t>& messageContent) {
    if (!initialized || !socket) {
        std::cerr << "[ZMQPublisher::publishMessage] 发布者未初始化" << std::endl;
        return false;
    }
    
    try {
        // 创建第一帧：消息类型
        zmq::message_t typeMsg(messageType.c_str(), messageType.length());
        
        // 创建第二帧：消息内容（字节数组）
        zmq::message_t contentMsg(messageContent.data(), messageContent.size());
        
        // 发送第一帧（带more标志）
        socket->send(typeMsg, zmq::send_flags::sndmore);
        
        // 发送第二帧（最后一帧）
        socket->send(contentMsg);
        
        std::cout << "[ZMQPublisher::publishMessage] 成功发布二进制消息 - 类型: " << messageType 
                  << ", 大小: " << messageContent.size() << " 字节" << std::endl;
        return true;
        
    } catch (const zmq::error_t& e) {
        std::cerr << "[ZMQPublisher::publishMessage] ZMQ发送失败: " << e.what() << std::endl;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "[ZMQPublisher::publishMessage] 发送异常: " << e.what() << std::endl;
        return false;
    }
}

void ZMQPublisher::disconnect() {
    try {
        if (socket) {
            socket->close();
            socket.reset();
        }
        
        if (context) {
            context->close();
            context.reset();
        }
        
        initialized = false;
        std::cout << "[ZMQPublisher::disconnect] 已断开连接" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "[ZMQPublisher::disconnect] 断开连接异常: " << e.what() << std::endl;
    }
} 