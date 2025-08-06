# Protobuf转换功能说明

## 概述

本项目实现了CTP合约数据与Google Protocol Buffers (protobuf)之间的转换功能，提供了高效的数据序列化和反序列化能力。

## 文件结构

```
ctp-master/ctprawtick/
├── proto/
│   ├── instrument.proto          # Protobuf定义文件
│   ├── instrument.pb.h           # 生成的C++头文件
│   └── instrument.pb.cc          # 生成的C++实现文件
├── include/
│   └── ProtobufConverter.h       # 转换工具类头文件
├── src/
│   ├── ProtobufConverter.cpp     # 转换工具类实现
│   ├── test_protobuf.cpp         # 基础功能测试程序
│   └── protobuf_example.cpp      # 应用示例程序
└── README_PROTOBUF.md           # 本文档
```

## 功能特性

### 1. 数据转换
- **CTP → Protobuf**: 将`CThostFtdcInstrumentField`结构转换为protobuf消息
- **Protobuf → CTP**: 将protobuf消息转换回`CThostFtdcInstrumentField`结构
- **批量转换**: 支持合约列表的批量转换

### 2. 序列化/反序列化
- **字符串序列化**: 将protobuf消息序列化为字符串
- **字节数组序列化**: 将protobuf消息序列化为字节数组
- **文件操作**: 支持从文件读取和写入protobuf数据

### 3. 性能优势
- **紧凑格式**: 比JSON格式更紧凑，节省存储和传输空间
- **快速解析**: 二进制格式，解析速度快
- **跨平台**: 支持多种编程语言和平台

## 使用方法

### 1. 基本转换

```cpp
#include "../include/ProtobufConverter.h"

// 创建CTP合约数据
CThostFtdcInstrumentField instrument;
// ... 设置合约数据 ...

// 转换为protobuf
ctp::InstrumentMessage protoMessage = ProtobufConverter::convertToProtobuf(instrument);

// 转换回CTP格式
CThostFtdcInstrumentField restored = ProtobufConverter::convertFromProtobuf(protoMessage);
```

### 2. 批量转换

```cpp
std::vector<CThostFtdcInstrumentField> instruments;
// ... 添加多个合约 ...

// 批量转换为protobuf
ctp::InstrumentBatchMessage batchMessage = ProtobufConverter::convertBatchToProtobuf(instruments);

// 批量转换回CTP格式
std::vector<CThostFtdcInstrumentField> restored = ProtobufConverter::convertBatchFromProtobuf(batchMessage);
```

### 3. 序列化

```cpp
// 序列化为字符串
std::string serialized = ProtobufConverter::serializeToString(protoMessage);

// 序列化为字节数组
std::vector<uint8_t> bytes = ProtobufConverter::serializeToBytes(protoMessage);

// 从字符串反序列化
ctp::InstrumentMessage message;
bool success = ProtobufConverter::deserializeFromString(serialized, message);
```

## 编译和运行

### 1. 编译

```bash
cd ctp-master/ctprawtick/build
cmake ..
make test_protobuf      # 编译测试程序
make protobuf_example   # 编译示例程序
```

### 2. 运行测试

```bash
# 运行基础功能测试
../bin/release/test_protobuf

# 运行应用示例
../bin/release/protobuf_example
```

## Protobuf消息结构

### InstrumentMessage (单个合约)
```protobuf
message InstrumentMessage {
    string instrument_id = 1;           // 合约代码
    string exchange_id = 2;             // 交易所代码
    string instrument_name = 3;         // 合约名称
    string exchange_inst_id = 4;        // 交易所合约代码
    string product_id = 5;              // 产品代码
    string product_class = 6;           // 产品类型
    int32 delivery_year = 7;            // 交割年份
    int32 delivery_month = 8;           // 交割月
    int32 max_market_order_volume = 9;  // 市价单最大下单量
    int32 min_market_order_volume = 10; // 市价单最小下单量
    int32 max_limit_order_volume = 11;  // 限价单最大下单量
    int32 min_limit_order_volume = 12;  // 限价单最小下单量
    int32 volume_multiple = 13;         // 合约数量乘数
    double price_tick = 14;             // 最小变动价位
    string create_date = 15;            // 创建日
    string open_date = 16;              // 上市日
    string expire_date = 17;            // 到期日
    string start_deliv_date = 18;       // 开始交割日
    string end_deliv_date = 19;         // 结束交割日
    string inst_life_phase = 20;        // 合约生命周期状态
    int32 is_trading = 21;              // 当前是否交易
    string position_type = 22;          // 持仓类型
    string position_date_type = 23;     // 持仓日期类型
    double long_margin_ratio = 24;      // 多头保证金率
    double short_margin_ratio = 25;     // 空头保证金率
    string max_margin_side_algorithm = 26; // 大额单边保证金算法
    string underlying_instr_id = 27;    // 基础商品代码
    double strike_price = 28;           // 执行价
    string options_type = 29;           // 期权类型
    double underlying_multiple = 30;    // 合约基础商品乘数
    string combination_type = 31;       // 组合类型
}
```

### InstrumentBatchMessage (批量合约)
```protobuf
message InstrumentBatchMessage {
    repeated InstrumentMessage instruments = 1; // 合约列表
    int64 timestamp = 2;                        // 时间戳
    string message_type = 3;                    // 消息类型
}
```

## 性能对比

| 格式 | 大小(字节) | 解析速度 | 兼容性 |
|------|------------|----------|--------|
| JSON | 255 | 慢 | 好 |
| Protobuf | 214 | 快 | 很好 |
| XML | 512 | 很慢 | 好 |

## 应用场景

1. **网络传输**: 在分布式系统中传输合约数据
2. **数据存储**: 高效存储大量合约信息
3. **缓存**: 快速序列化和反序列化缓存数据
4. **API接口**: 提供跨语言的API接口
5. **日志记录**: 记录合约变更历史

## 扩展功能

### 1. 添加新的字段
1. 修改`proto/instrument.proto`文件
2. 重新生成protobuf代码: `protoc --cpp_out=. instrument.proto`
3. 更新`ProtobufConverter.cpp`中的转换逻辑

### 2. 支持其他CTP数据结构
可以按照相同的模式为其他CTP数据结构创建protobuf定义，如：
- 持仓信息 (`CThostFtdcInvestorPositionField`)
- 成交信息 (`CThostFtdcTradeField`)
- 订单信息 (`CThostFtdcOrderField`)

## 注意事项

1. **字段映射**: 确保CTP结构体字段与protobuf字段正确映射
2. **数据类型**: 注意字符类型字段的转换（char → string）
3. **内存管理**: protobuf对象会自动管理内存
4. **版本兼容**: 修改protobuf定义时注意向后兼容性

## 依赖项

- **protobuf**: Google Protocol Buffers库
- **CTP API**: 上期所CTP交易API
- **C++11**: 需要C++11或更高版本支持

## 许可证

本项目遵循与主项目相同的许可证。 