# CTPTrader与zmq-dataupdate合约数据对应关系

## 概述

本文档详细说明了CTPTrader中的`allInstruments`（`vector<CThostFtdcInstrumentField>`）与zmq-dataupdate中的`instruments`（数据库表`test_update_instrument`）之间的对应关系。

## 数据流向

```
CTPTrader (allInstruments) 
    ↓ (protobuf序列化)
ZMQ消息 (CTP_INSTRUMENT_BATCH_UPDATE)
    ↓ (ZMQ传输)
zmq-dataupdate (接收)
    ↓ (protobuf反序列化)
InstrumentConverter (转换)
    ↓ (格式转换)
DatabaseManager (插入)
    ↓ (SQL执行)
test_update_instrument表
```

## 字段对应关系

### CThostFtdcInstrumentField → test_update_instrument表

| CTP字段 | 数据库字段 | 类型 | 说明 |
|---------|------------|------|------|
| `InstrumentID` | `instrument_id` | VARCHAR(31) | 合约代码 |
| `ExchangeID` | `exchange_id` | VARCHAR(9) | 交易所代码 |
| `InstrumentName` | `instrument_name` | VARCHAR(21) | 合约名称 |
| `ExchangeInstID` | `exchange_inst_id` | VARCHAR(31) | 合约在交易所的代码 |
| `ProductID` | `product_id` | VARCHAR(21) | 产品代码 |
| `ProductClass` | `product_class` | CHAR(1) | 产品类型 |
| `DeliveryYear` | `delivery_year` | INT | 交割年份 |
| `DeliveryMonth` | `delivery_month` | INT | 交割月份 |
| `MaxMarketOrderVolume` | `max_market_order_volume` | INT | 市价单最大下单量 |
| `MinMarketOrderVolume` | `min_market_order_volume` | INT | 市价单最小下单量 |
| `MaxLimitOrderVolume` | `max_limit_order_volume` | INT | 限价单最大下单量 |
| `MinLimitOrderVolume` | `min_limit_order_volume` | INT | 限价单最小下单量 |
| `VolumeMultiple` | `volume_multiple` | INT | 合约数量乘数 |
| `PriceTick` | `price_tick` | DECIMAL(10,8) | 最小变动价位 |
| `CreateDate` | `create_date` | VARCHAR(9) | 创建日 |
| `OpenDate` | `open_date` | VARCHAR(9) | 上市日 |
| `ExpireDate` | `expire_date` | VARCHAR(9) | 到期日 |
| `StartDelivDate` | `start_deliv_date` | VARCHAR(9) | 开始交割日 |
| `EndDelivDate` | `end_deliv_date` | VARCHAR(9) | 结束交割日 |
| `InstLifePhase` | `inst_life_phase` | CHAR(1) | 合约生命周期状态 |
| `IsTrading` | `is_trading` | TINYINT(1) | 当前是否交易 |
| `PositionType` | `position_type` | CHAR(1) | 持仓类型 |
| `PositionDateType` | `position_date_type` | CHAR(1) | 持仓日期类型 |
| `LongMarginRatio` | `long_margin_ratio` | DECIMAL(10,4) | 多头保证金率 |
| `ShortMarginRatio` | `short_margin_ratio` | DECIMAL(10,4) | 空头保证金率 |
| `MaxMarginSideAlgorithm` | `max_margin_side_algorithm` | CHAR(1) | 最大保证金算法 |
| `UnderlyingInstrID` | `underlying_instr_id` | VARCHAR(31) | 基础商品代码 |
| `StrikePrice` | `strike_price` | DECIMAL(10,4) | 执行价 |
| `OptionsType` | `options_type` | CHAR(1) | 期权类型 |
| `UnderlyingMultiple` | `underlying_multiple` | DECIMAL(10,4) | 基础商品乘数 |
| `CombinationType` | `combination_type` | CHAR(1) | 组合类型 |

## 数据转换流程

### 1. CTPTrader端 (发送)

```cpp
// 在CTPTrader.cpp中
vector<CThostFtdcInstrumentField> allInstruments;

// 收集合约数据
allInstruments.push_back(instrument);

// 转换为protobuf批量消息
ctp::InstrumentBatchMessage batchMessage = ProtobufConverter::convertBatchToProtobuf(allInstruments);

// 序列化并发送
std::string batchSerialized = ProtobufConverter::serializeToString(batchMessage);
publisher.publishMessage("CTP_INSTRUMENT_BATCH_UPDATE", batchSerialized);
```

### 2. zmq-dataupdate端 (接收)

```cpp
// 在ZMQSubscriber.cpp中
void ZMQSubscriber::processInstrumentMessage(const std::string& messageContent) {
    // 检查消息格式
    bool isProtobuf = InstrumentConverter::isProtobufFormat(messageContent);
    
    if (isProtobuf) {
        processProtobufInstrumentMessage(messageContent);
    } else {
        processCSVInstrumentMessage(messageContent);
    }
}

void ZMQSubscriber::processProtobufInstrumentMessage(const std::string& messageContent) {
    // 解析protobuf数据
    std::vector<CTPInstrumentField> instruments = InstrumentConverter::parseProtobufBatch(messageContent);
    
    // 转换为数据库格式
    std::vector<std::vector<std::string>> dbInstruments = InstrumentConverter::convertBatchToDatabaseFormat(instruments);
    
    // 插入数据库
    dbManager->insertInstrumentBatch(dbInstruments);
}
```

## 消息格式

### Protobuf格式 (CTPTrader发送)

```protobuf
message InstrumentBatchMessage {
    int64 timestamp = 1;
    string message_type = 2;
    repeated InstrumentMessage instruments = 3;
}

message InstrumentMessage {
    string instrument_id = 1;
    string exchange_id = 2;
    string instrument_name = 3;
    // ... 其他字段
}
```

### CSV格式 (备用格式)

```
IF2401,SHFE,沪深300指数期货2401,IF2401,IF,1,2024,1,100,1,100,1,300,0.2,20231201,20231201,20240119,20240115,20240119,1,1,1,1,0.12,0.12,1,,0.0,,1.0,1
```

## 数据库操作

### 插入语句

```sql
INSERT INTO test_update_instrument (
    instrument_id, exchange_id, instrument_name, exchange_inst_id, product_id,
    product_class, delivery_year, delivery_month, max_market_order_volume,
    min_market_order_volume, max_limit_order_volume, min_limit_order_volume,
    volume_multiple, price_tick, create_date, open_date, expire_date,
    start_deliv_date, end_deliv_date, inst_life_phase, is_trading, position_type,
    position_date_type, long_margin_ratio, short_margin_ratio, max_margin_side_algorithm,
    underlying_instr_id, strike_price, options_type, underlying_multiple, combination_type
) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
ON DUPLICATE KEY UPDATE
    instrument_name=VALUES(instrument_name),
    exchange_inst_id=VALUES(exchange_inst_id),
    -- ... 其他字段更新
    updated_at=NOW()
```

## 错误处理

### 1. 数据格式错误
- 检测protobuf格式是否有效
- 验证字段数量和类型
- 记录错误日志

### 2. 数据库连接错误
- 自动重连机制
- 事务回滚
- 错误日志记录

### 3. 数据转换错误
- 字段类型转换异常处理
- 空值处理
- 默认值设置

## 性能优化

### 1. 批量处理
- 使用批量插入减少数据库交互
- 事务处理提高性能
- 内存优化

### 2. 并发处理
- 多线程处理
- 锁机制保护共享资源
- 异步处理

### 3. 缓存机制
- 连接池
- 查询结果缓存
- 减少重复计算

## 监控和日志

### 1. 数据统计
- 接收消息数量
- 处理成功/失败数量
- 处理时间统计

### 2. 错误监控
- 数据库连接状态
- 消息处理异常
- 系统资源使用

### 3. 日志记录
- 详细的操作日志
- 错误堆栈信息
- 性能指标记录

## 扩展性

### 1. 支持新字段
- 在CTPInstrumentField中添加新字段
- 更新数据库表结构
- 修改转换逻辑

### 2. 支持新格式
- 添加新的消息格式解析器
- 扩展InstrumentConverter
- 更新消息处理逻辑

### 3. 支持新数据库
- 抽象数据库接口
- 实现新的数据库适配器
- 配置数据库连接 