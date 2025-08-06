# ZMQ数据更新订阅者

这是一个基于ZMQ（ZeroMQ）的C++订阅者程序，用于接收和处理来自其他进程的发布消息。

## 功能特性

1. **ZMQ订阅功能**: 在本地端口8888订阅SUB消息
2. **多帧消息处理**: 支持2帧消息格式（frame1为消息类型，frame2为消息内容）
3. **数据库存储**: 连接MySQL数据库，使用mysqlcppconn库
4. **配置管理**: 支持从配置文件读取参数
5. **日志系统**: 完整的日志记录功能
6. **面向对象设计**: 采用OOP设计模式
7. **CMake构建**: 使用CMake进行项目构建
8. **Linux支持**: 专为Linux环境设计

## 项目结构

```
zmq-dataupdate/
├── include/                 # 头文件目录
│   ├── ConfigManager.h     # 配置管理器
│   ├── Logger.h            # 日志管理器
│   ├── DatabaseManager.h   # 数据库管理器
│   └── ZMQSubscriber.h     # ZMQ订阅者
├── src/                    # 源文件目录
│   ├── main.cpp           # 主程序
│   ├── ConfigManager.cpp  # 配置管理器实现
│   ├── Logger.cpp         # 日志管理器实现
│   ├── DatabaseManager.cpp # 数据库管理器实现
│   └── ZMQSubscriber.cpp  # ZMQ订阅者实现
├── config.ini             # 配置文件
├── init_database.sql      # 数据库初始化脚本
├── test_publisher.cpp     # 测试发布者程序
├── build.sh               # 构建脚本
├── CMakeLists.txt         # CMake构建文件
└── README_CN.md           # 说明文档
```

## 依赖要求

- C++17 或更高版本
- CMake 3.10 或更高版本
- ZeroMQ 库 (libzmq3)
- MySQL Connector/C++ (mysqlcppconn)
- MySQL 服务器

## 安装依赖

### Ubuntu/Debian:
```bash
sudo apt-get update
sudo apt-get install build-essential cmake libzmq3-dev libmysqlcppconn-dev
```

### CentOS/RHEL:
```bash
sudo yum install gcc-c++ cmake zeromq-devel mysql-connector-c++-devel
```

## 构建和运行

### 1. 构建项目
```bash
chmod +x build.sh
./build.sh
```

### 2. 初始化数据库
```bash
mysql -u root -p < init_database.sql
```

### 3. 配置程序
编辑 `config.ini` 文件，设置数据库连接参数：
```ini
# 数据库配置
database.host=localhost
database.port=3306
database.user=your_username
database.password=your_password
database.name=zmq_data
```

### 4. 运行程序
```bash
cd build
./zmq_dataupdate
```

### 5. 测试发布者
```bash
cd build
g++ -o test_publisher ../test_publisher.cpp -lzmq
./test_publisher
```

## 配置说明

### 配置文件 (config.ini)
- `zmq.address`: ZMQ连接地址
- `database.host`: 数据库主机地址
- `database.port`: 数据库端口
- `database.user`: 数据库用户名
- `database.password`: 数据库密码
- `database.name`: 数据库名称
- `log.file`: 日志文件路径
- `log.level`: 日志级别 (0=DEBUG, 1=INFO, 2=WARNING, 3=ERROR)

## 消息格式

程序接收2帧ZMQ消息：
- **Frame 1**: 消息类型 (字符串)
- **Frame 2**: 消息内容 (字符串)

### 支持的消息类型
- `TEXT`: 普通文本消息
- `JSON`: JSON格式消息
- `TICK`: Tick数据消息
- `CSV`: CSV格式消息
- `BINARY`: 二进制消息

### Tick数据格式
```
TICK AAPL 150.25 1000 1640995200000
```
格式：`TICK 品种 价格 成交量 时间戳`

## 自定义消息处理

开发者可以通过设置自定义消息处理函数来处理特定类型的消息：

```cpp
void customMessageHandler(const std::string& messageType, const std::string& messageContent) {
    // 自定义处理逻辑
    if (messageType == "CUSTOM") {
        // 处理自定义消息
    }
}

// 在main.cpp中设置
subscriber->setMessageHandler(customMessageHandler);
```

## 数据库表结构

### messages 表
- `id`: 主键
- `message_type`: 消息类型
- `message_content`: 消息内容
- `created_at`: 创建时间

### tick_data 表
- `id`: 主键
- `symbol`: 交易品种
- `price`: 价格
- `volume`: 成交量
- `timestamp`: 时间戳
- `created_at`: 创建时间

### system_status 表
- `id`: 主键
- `component`: 组件名称
- `status`: 状态
- `message`: 状态信息
- `updated_at`: 更新时间

## 日志系统

程序提供完整的日志记录功能：
- 支持不同日志级别
- 同时输出到文件和控制台
- 线程安全的日志记录
- 时间戳和日志级别标识

## 错误处理

程序包含完善的错误处理机制：
- 数据库连接失败处理
- ZMQ连接异常处理
- 配置文件加载失败处理
- 信号处理（Ctrl+C）

## 性能优化

- 使用智能指针管理内存
- 线程安全的日志记录
- 数据库连接池（可扩展）
- 异步消息处理

## 扩展功能

程序设计为可扩展的架构，可以轻松添加：
- 新的消息类型处理
- 数据库连接池
- 消息过滤功能
- 性能监控
- 集群支持

## 故障排除

### 常见问题

1. **编译错误**: 确保安装了所有依赖库
2. **数据库连接失败**: 检查数据库配置和网络连接
3. **ZMQ连接失败**: 确保发布者程序正在运行
4. **权限问题**: 确保有足够的文件系统权限

### 调试模式

设置日志级别为DEBUG以获取详细日志：
```ini
log.level=0
```

## 许可证

本项目采用MIT许可证。 