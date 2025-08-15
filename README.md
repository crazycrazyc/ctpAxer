# CTPRawTick 使用指南

## 📋 项目概述

CTPRawTick是一个基于CTP API的期货数据采集和处理系统，包含三个核心程序：
- **ctpmarket**: 实时行情数据订阅
- **ctpinstrument**: 合约信息查询
- **ctpmonitor**: 持仓和资金账户查询

## 🏗️ 系统架构

```
CTP期货交易所
    ↓ (CTP API)
三个核心程序 (ctpmarket/ctpinstrument/ctpmonitor)
    ↓ (ZMQ消息队列)
数据桥接和存储 (market_bridge.py)
    ↓ (MySQL + Redis)
Web界面 (websocket_app.py)
    ↓ (WebSocket)
前端实时显示
```

## 📁 目录结构

```
ctprawtick/
├── bin/debug/              # 可执行文件
│   ├── ctpmarket          # 实时行情订阅程序
│   ├── ctpinstrument      # 合约信息查询程序
│   ├── ctpmonitor         # 持仓资金查询程序
│   └── zycConfig.json     # 主配置文件
├── sql/                   # 数据库表结构
│   ├── market_data.sql
│   ├── test_update_instrument.sql
│   ├── trading_account.sql
│   └── investor_positions.sql
├── src/                   # 源代码
├── include/               # 头文件
├── proto/                 # Protobuf定义文件
├── start_system.sh        # 系统启动脚本
├── start_realtime_system.sh  # 实时系统启动脚本
├── install_crontab.sh     # 定时任务安装脚本
└── crontab_optimized.txt  # 定时任务配置
```

## ⚙️ 配置说明

### 1. 主配置文件 (bin/debug/zycConfig.json)

```json
{
    "FRONT_ADDR": "182.254.243.31:30011",
    "BROKER_ID": "9999",
    "INVESTOR_ID": "123456",
    "PASSWORD": "password123",
    "td_server": "182.254.243.31:30001",
    "md_server": "182.254.243.31:30011",
    "DEC": "cu2508,al2508,zn2508,au2508,ag2508,rb2508"
}
```

**关键配置项说明**:
- `FRONT_ADDR`: CTP前置服务器地址
- `BROKER_ID`: 期货公司代码
- `INVESTOR_ID`: 投资者账号
- `PASSWORD`: 登录密码
- `DEC`: 订阅的合约列表（逗号分隔）

### 2. 定时任务配置 (crontab_optimized.txt)

```bash
# 合约信息查询 - 每日8:30和15:30
30 8,15 * * 1-5 cd /path/to/ctprawtick && ./start_system.sh start-instrument

# 持仓资金查询 - 每5分钟
*/5 * * * 1-5 cd /path/to/ctprawtick && ./start_system.sh start-monitor

# 实时行情订阅 - 每10分钟检查
*/10 9-15 * * 1-5 cd /path/to/ctprawtick && ./start_realtime_system.sh
```

## 🚀 使用方法

### 1. 编译程序

```bash
# 进入项目目录
cd /home/zyc/projects/ctprade/ctp-master/ctprawtick

# 创建build目录
mkdir -p build && cd build

# 配置和编译
cmake ..
make

# 编译完成后，可执行文件位于 bin/debug/ 目录
```

### 2. 配置数据库

```bash
# 连接MySQL数据库
mysql -u root -p

# 创建数据库
CREATE DATABASE IF NOT EXISTS cta_trade;
USE cta_trade;

# 导入表结构
source sql/market_data.sql;
source sql/test_update_instrument.sql;
source sql/trading_account.sql;
source sql/investor_positions.sql;
```

### 3. 启动系统

#### 方式一：手动启动单个程序

```bash
cd bin/debug

# 启动实时行情订阅
./ctpmarket

# 启动合约信息查询
./ctpinstrument

# 启动持仓资金查询
./ctpmonitor
```

#### 方式二：使用启动脚本

```bash
# 启动合约信息查询
./start_system.sh start-instrument

# 启动持仓资金查询
./start_system.sh start-monitor

# 启动实时行情系统
./start_realtime_system.sh
```

#### 方式三：安装定时任务

```bash
# 安装定时任务
./install_crontab.sh

# 查看定时任务
crontab -l

# 查看定时任务日志
tail -f /var/log/cron
```

### 4. 启动Web界面

```bash
# 进入web-interface目录
cd ../web-interface

# 启动数据桥接服务
nohup python3 market_bridge.py > market_bridge.log 2>&1 &

# 启动Web应用
nohup python3 websocket_app.py > websocket_app.log 2>&1 &

# 访问Web界面
# http://localhost:5502
```

## 📊 数据流向

### 1. 行情数据流

```
CTP行情服务器 → ctpmarket → ZMQ(9999) → market_bridge.py → Redis + MySQL → websocket_app.py → Web前端
```

### 2. 合约数据流

```
CTP交易服务器 → ctpinstrument → ZMQ(8888) → MySQL(test_update_instrument) → Web查询API
```

### 3. 资金持仓数据流

```
CTP交易服务器 → ctpmonitor → ZMQ(8888) → MySQL(trading_account/investor_positions) → Web查询API
```

## 🔧 运维管理

### 1. 进程监控

```bash
# 查看运行中的程序
ps aux | grep ctp

# 查看ZMQ端口占用
netstat -tlnp | grep -E "(8888|9999)"

# 查看日志
tail -f bin/debug/*.log
```

### 2. 问题排查

#### 程序无法启动
```bash
# 检查配置文件
cat bin/debug/zycConfig.json

# 检查CTP服务器连接
telnet 182.254.243.31 30011

# 检查依赖库
ldd bin/debug/ctpmarket
```

#### 无数据写入数据库
```bash
# 检查数据库连接
mysql -h 172.16.30.97 -P 13306 -u elwriter -p

# 检查表结构
DESCRIBE market_data;

# 检查数据
SELECT COUNT(*) FROM market_data WHERE TradingDay = '20250806';
```

#### Web界面无法访问
```bash
# 检查端口
netstat -tlnp | grep 5502

# 检查防火墙
sudo ufw status

# 检查日志
tail -f web-interface/websocket_app.log
```

### 3. 性能优化

#### MySQL优化
```sql
-- 添加索引
ALTER TABLE market_data ADD INDEX idx_instrument_time (InstrumentID, UpdateTime);

-- 清理历史数据
DELETE FROM market_data WHERE TradingDay < '20250801';
```

#### Redis优化
```bash
# 清理Redis缓存
redis-cli FLUSHDB

# 监控Redis性能
redis-cli --latency
```

## 📱 Web界面功能

### 1. 合约信息查询
- 路径: `/`
- 功能: 查询、搜索、导出合约信息

### 2. 持仓信息查询
- 路径: `/positions`
- 功能: 查看持仓明细、统计

### 3. 资金账户查询
- 路径: `/accounts`
- 功能: 查看账户余额、盈亏

### 4. 实时行情订阅
- 路径: `/realtime`
- 功能: WebSocket实时行情推送

## 🎯 最佳实践

### 1. 部署建议
- 使用专门的服务器部署
- 配置足够的内存和存储空间
- 定期备份数据库和配置文件

### 2. 监控告警
- 设置进程监控脚本
- 配置数据库连接监控
- 设置磁盘空间告警

### 3. 安全建议
- 定期更换CTP账户密码
- 限制数据库访问权限
- 配置防火墙规则

## ❓ 常见问题

### Q: 程序编译失败怎么办？
A: 检查依赖库是否安装完整，确保protobuf、zmq、mysql-connector等库版本正确。

### Q: CTP连接失败怎么办？
A: 检查网络连接、账户密码、服务器地址是否正确，确认CTP服务器状态。

### Q: 数据不更新怎么办？
A: 检查程序是否正常运行、数据库连接是否正常、ZMQ消息队列是否阻塞。

### Q: Web界面访问慢怎么办？
A: 检查数据库查询性能、添加必要索引、优化SQL语句。

## 📞 技术支持

如有问题，请检查：
1. 日志文件 (`*.log`)
2. 系统资源使用情况
3. 网络连接状态
4. 数据库连接状态

---

**注意**: 本系统仅用于学习和研究目的，实际生产环境使用请确保合规性和安全性。 