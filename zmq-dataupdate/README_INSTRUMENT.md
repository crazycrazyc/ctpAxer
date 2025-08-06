# 合约信息收集和存储功能说明

## 功能概述

本功能用于收集CTP合约信息并将其存储到数据库的`test_update_instrument`表中，同时支持将合约信息发布到指定的ZMQ地址。

## 数据库表结构

### test_update_instrument表

基于ProtobufConverter中的合约结构创建，包含以下字段：

- `instrument_id` - 合约代码
- `exchange_id` - 交易所代码  
- `instrument_name` - 合约名称
- `exchange_inst_id` - 合约在交易所的代码
- `product_id` - 产品代码
- `product_class` - 产品类型
- `delivery_year` - 交割年份
- `delivery_month` - 交割月份
- `max_market_order_volume` - 市价单最大下单量
- `min_market_order_volume` - 市价单最小下单量
- `max_limit_order_volume` - 限价单最大下单量
- `min_limit_order_volume` - 限价单最小下单量
- `volume_multiple` - 合约数量乘数
- `price_tick` - 最小变动价位
- `create_date` - 创建日
- `open_date` - 上市日
- `expire_date` - 到期日
- `start_deliv_date` - 开始交割日
- `end_deliv_date` - 结束交割日
- `inst_life_phase` - 合约生命周期状态
- `is_trading` - 当前是否交易
- `position_type` - 持仓类型
- `position_date_type` - 持仓日期类型
- `long_margin_ratio` - 多头保证金率
- `short_margin_ratio` - 空头保证金率
- `max_margin_side_algorithm` - 最大保证金算法
- `underlying_instr_id` - 基础商品代码
- `strike_price` - 执行价
- `options_type` - 期权类型
- `underlying_multiple` - 基础商品乘数
- `combination_type` - 组合类型

## 配置说明

### 数据库配置 (config.ini)

```ini
# 数据库配置
database.host=172.16.30.97
database.port=13306
database.user=elwriter
database.password=elwriter123
database.name=zmq_data
```

### ZMQ配置

- 订阅地址: `tcp://localhost:8888`
- 发布地址: `tcp://172.16.30.97:13306`
- 主题: `test_update_instrument`

## 使用方法

### 1. 初始化数据库

```bash
mysql -h 172.16.30.97 -P 13306 -u elwriter -p < init_database.sql
```

### 2. 编译程序

```bash
./build.sh
```

### 3. 运行订阅者

```bash
./zmq_dataupdate
```

### 4. 发送合约数据

使用测试程序发送合约数据：

```bash
g++ -o test_instrument_publisher test_instrument_publisher.cpp -lzmq
./test_instrument_publisher
```

## 消息格式

### 合约消息格式

合约消息使用CSV格式，字段顺序如下：

```
instrument_id,exchange_id,instrument_name,exchange_inst_id,product_id,product_class,delivery_year,delivery_month,max_market_order_volume,min_market_order_volume,max_limit_order_volume,min_limit_order_volume,volume_multiple,price_tick,create_date,open_date,expire_date,start_deliv_date,end_deliv_date,inst_life_phase,is_trading,position_type,position_date_type,long_margin_ratio,short_margin_ratio,max_margin_side_algorithm,underlying_instr_id,strike_price,options_type,underlying_multiple,combination_type
```

### 示例数据

```
IF2401,SHFE,沪深300指数期货2401,IF2401,IF,1,2024,1,100,1,100,1,300,0.2,20231201,20231201,20240119,20240115,20240119,1,1,1,1,0.12,0.12,1,,0.0,,1.0,1
```

## 功能特性

1. **自动去重**: 使用`ON DUPLICATE KEY UPDATE`确保合约信息不重复
2. **批量插入**: 支持批量插入多个合约，提高性能
3. **事务支持**: 批量插入使用事务确保数据一致性
4. **错误处理**: 完善的错误处理和日志记录
5. **实时处理**: 支持实时接收和处理合约信息

## 日志记录

程序会记录以下信息：
- 合约消息接收
- 数据库插入操作
- 错误和异常信息
- 处理统计信息

日志文件: `zmq_subscriber.log`

## 注意事项

1. 确保数据库连接正常
2. 确保ZMQ服务正常运行
3. 合约数据格式必须符合要求
4. 建议定期备份数据库
5. 监控日志文件以检查运行状态 