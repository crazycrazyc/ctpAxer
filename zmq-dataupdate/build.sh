#!/bin/bash

echo "开始构建ZMQ数据更新订阅者..."

# 创建构建目录
mkdir -p build
cd build

# 运行CMake
echo "运行CMake..."
cmake ..

# 编译
echo "编译项目..."
make -j$(nproc)

if [ $? -eq 0 ]; then
    echo "构建成功！"
    echo "可执行文件位置: build/zmq_dataupdate"
    echo ""
    echo "使用方法:"
    echo "1. 首先运行数据库初始化脚本: mysql -u root -p < ../init_database.sql"
    echo "2. 修改配置文件: ../config.ini"
    echo "3. 运行程序: ./zmq_dataupdate"
    echo "4. 测试发布者: g++ -o test_publisher ../test_publisher.cpp -lzmq"
    echo "5. 运行测试发布者: ./test_publisher"
else
    echo "构建失败！"
    exit 1
fi 