# 投资者信息Protobuf转换和持仓查询功能

## 功能概述

本项目新增了两个重要功能：

1. **投资者信息Protobuf转换**：在`ProtobufConverter`中添加了`CThostFtdcInvestorField`的protobuf数据类型转换
2. **投资者持仓查询和存储**：新增`InvestorPositionManager`用于查询投资者持仓并插入数据库

## 文件结构

```
ctp-master/ctprawtick/
├── proto/
│   ├── instrument.proto          # 合约信息protobuf定义
│   └── investor.proto            # 投资者信息protobuf定义（新增）
├── include/
│   ├── ProtobufConverter.h       # Protobuf转换器头文件（已更新）
│   └── InvestorPositionManager.h # 投资者持仓管理器头文件（新增）
├── src/
│   ├── ProtobufConverter.cpp     # Protobuf转换器实现（已更新）
│   └── InvestorPositionManager.cpp # 投资者持仓管理器实现（新增）
└── sql/
    └── investor_positions.sql    # 投资者持仓表结构（新增）
```

## 功能1：投资者信息Protobuf转换

### 新增的转换方法

在`ProtobufConverter`类中新增了以下方法：

#### 投资者信息转换
```cpp
// 将CTP投资者结构转换为protobuf消息
static ctp::InvestorMessage convertToProtobuf(const CThostFtdcInvestorField& investor);

// 将protobuf消息转换为CTP投资者结构
static CThostFtdcInvestorField convertFromProtobuf(const ctp::InvestorMessage& message);

// 将投资者列表转换为批量protobuf消息
static ctp::InvestorBatchMessage convertBatchToProtobuf(const std::vector<CThostFtdcInvestorField>& investors);

// 将批量protobuf消息转换为投资者列表
static std::vector<CThostFtdcInvestorField> convertBatchFromProtobuf(const ctp::InvestorBatchMessage& batchMessage);
```

#### 序列化方法
```cpp
// 投资者信息序列化
static std::string serializeToString(const ctp::InvestorMessage& message);
static std::string serializeToString(const ctp::InvestorBatchMessage& batchMessage);

// 投资者信息反序列化
static bool deserializeFromString(const std::string& data, ctp::InvestorMessage& message);
static bool deserializeFromString(const std::string& data, ctp::InvestorBatchMessage& batchMessage);

// 投资者信息字节数组序列化
static std::vector<uint8_t> serializeToBytes(const ctp::InvestorMessage& message);
static std::vector<uint8_t> serializeToBytes(const ctp::InvestorBatchMessage& batchMessage);

// 投资者信息字节数组反序列化
static bool deserializeFromBytes(const std::vector<uint8_t>& data, ctp::InvestorMessage& message);
static bool deserializeFromBytes(const std::vector<uint8_t>& data, ctp::InvestorBatchMessage& batchMessage);
```

### 使用示例

```cpp
#include "../include/ProtobufConverter.h"

// 创建投资者数据
CThostFtdcInvestorField investor;
strcpy(investor.InvestorID, "123456");
strcpy(investor.BrokerID, "9999");
strcpy(investor.InvestorName, "测试投资者");
investor.IsActive = 1;

// 转换为protobuf
ctp::InvestorMessage investorMessage = ProtobufConverter::convertToProtobuf(investor);

// 序列化
std::string serialized = ProtobufConverter::serializeToString(investorMessage);

// 批量处理
std::vector<CThostFtdcInvestorField> investors;
investors.push_back(investor);

ctp::InvestorBatchMessage batchMessage = ProtobufConverter::convertBatchToProtobuf(investors);
std::string batchSerialized = ProtobufConverter::serializeToString(batchMessage);
```

## 功能2：投资者持仓查询和存储

### 数据库表结构

创建`investor_positions`表，包含以下主要字段：

- `BrokerID` - 经纪公司代码
- `InvestorID` - 投资者代码
- `InstrumentID` - 合约代码
- `HedgeFlag` - 投机套保标志
- `PosiDirection` - 持仓多空方向
- `Position` - 今日持仓
- `PositionCost` - 持仓成本
- `PositionProfit` - 持仓盈亏
- `SettlementPrice` - 结算价
- `TradingDay` - 交易日

### 主要类

#### InvestorPositionDBManager
负责数据库连接和持仓数据存储：

```cpp
class InvestorPositionDBManager {
public:
    // 连接数据库
    bool connect();
    
    // 保存投资者持仓数据到数据库
    bool saveInvestorPositions(const vector<CThostFtdcInvestorPositionField>& positions);
};
```

#### InvestorPositionQueryManager
负责CTP连接和持仓查询：

```cpp
class InvestorPositionQueryManager {
public:
    // 连接CTP
    bool connect();
    
    // 查询投资者持仓
    bool queryInvestorPosition(const string& instrumentID = "");
    
    // 查询持仓并保存到数据库
    bool queryAndSavePositions(const string& db_host, const string& db_user, 
                              const string& db_password, const string& db_database, int db_port);
    
    // 获取查询结果
    vector<CThostFtdcInvestorPositionField> getPositionResults();
};
```

### 使用示例

```cpp
#include "../include/InvestorPositionManager.h"

// 数据库配置
const string DB_HOST = "172.16.30.97";
const string DB_USER = "elwriter";
const string DB_PASSWORD = "elwriter123";
const string DB_DATABASE = "cta_trader";
const int DB_PORT = 13307;

// CTP配置
const string BROKER_ID = "9999";
const string USER_ID = "123456";
const string PASSWORD = "123456";
const string TD_SERVER = "tcp://180.168.146.187:10201";

int main() {
    // 创建持仓查询管理器
    InvestorPositionQueryManager queryManager(TD_SERVER, BROKER_ID, USER_ID, PASSWORD);
    
    // 连接CTP
    if (!queryManager.connect()) {
        cout << "CTP连接失败" << endl;
        return -1;
    }
    
    // 查询并保存持仓数据
    if (!queryManager.queryAndSavePositions(DB_HOST, DB_USER, DB_PASSWORD, DB_DATABASE, DB_PORT)) {
        cout << "持仓查询和保存失败" << endl;
        return -1;
    }
    
    cout << "程序执行完成" << endl;
    return 0;
}
```

## 编译和运行

### 1. 编译protobuf文件

```bash
cd ctp-master/ctprawtick/proto
protoc --cpp_out=../src/ investor.proto
```

### 2. 编译C++代码

```bash
cd ctp-master/ctprawtick
mkdir -p build
cd build
cmake ..
make
```

### 3. 创建数据库表

```bash
mysql -h 172.16.30.97 -P 13307 -u elwriter -p cta_trader < sql/investor_positions.sql
```

### 4. 运行程序

```bash
cd ctp-master/ctprawtick/build
./InvestorPositionManager
```

## 配置说明

### 数据库配置
- **主机**: 172.16.30.97
- **端口**: 13307
- **用户**: elwriter
- **密码**: elwriter123
- **数据库**: cta_trader
- **表名**: investor_positions

### CTP配置
- **经纪公司代码**: 9999
- **用户代码**: 123456
- **密码**: 123456
- **交易服务器**: tcp://180.168.146.187:10201

## 注意事项

1. **protobuf依赖**: 确保已安装protobuf库
2. **MySQL依赖**: 确保已安装MySQL C++ Connector
3. **CTP连接**: 确保CTP交易服务器可访问
4. **数据库权限**: 确保数据库用户有创建表和插入数据的权限
5. **网络连接**: 确保网络连接正常

## 错误处理

程序包含完整的错误处理机制：

- 数据库连接失败处理
- CTP连接失败处理
- 查询超时处理
- 数据插入失败处理
- 事务回滚机制

## 扩展功能

可以根据需要扩展以下功能：

1. **定时查询**: 添加定时器定期查询持仓
2. **数据导出**: 支持导出持仓数据到CSV或Excel
3. **监控告警**: 添加持仓变化监控和告警
4. **历史数据**: 支持查询历史持仓数据
5. **多账户支持**: 支持查询多个投资者账户的持仓 