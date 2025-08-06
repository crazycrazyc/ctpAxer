# 合约信息收集功能使用说明

## 功能完成情况

✅ 已根据ProtobufConverter中的合约结构创建了`test_update_instrument`表
✅ 已实现合约信息的数据库插入功能
✅ 已配置数据库连接到172.16.30.97:13306
✅ 已实现ZMQ消息处理，支持合约数据接收
✅ 已创建测试程序验证功能

## 快速开始

### 1. 初始化数据库

```bash
# 连接到数据库并创建表
mysql -h 172.16.30.97 -P 13306 -u elwriter -p < init_database.sql
```

### 2. 运行订阅者程序

```bash
# 在后台运行订阅者
./build/zmq_dataupdate &
```

### 3. 发送测试数据

```bash
# 运行测试程序发送合约数据
./test_instrument_publisher
```

### 4. 查看结果

```bash
# 连接到数据库查看插入的数据
mysql -h 172.16.30.97 -P 13306 -u elwriter -p zmq_data

# 查询合约数据
SELECT instrument_id, instrument_name, exchange_id, is_trading FROM test_update_instrument;
```

## 配置说明

- **数据库**: 172.16.30.97:13306 (zmq_data)
- **ZMQ订阅**: tcp://localhost:8888
- **日志文件**: zmq_subscriber.log

## 消息格式

合约消息使用CSV格式，包含31个字段，对应ProtobufConverter中的合约结构。

## 注意事项

1. 确保数据库服务正常运行
2. 确保ZMQ库已安装
3. 程序会自动处理重复数据（使用ON DUPLICATE KEY UPDATE）
4. 支持批量插入提高性能 