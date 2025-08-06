# CTP实时行情系统

## 🎯 系统概述

这是一个基于WebSocket的CTP实时行情分发系统，支持实时推送行情数据到Web前端展示。

### 系统架构

```
CTP行情推送 → ZMQ(9999端口) → 行情桥接服务 → Redis消息队列 → WebSocket服务器 → Web前端实时显示
```

## 🔧 核心组件

### 1. 行情桥接服务 (`market_bridge.py`)
- **功能**: 订阅ZMQ行情数据，转发到Redis消息队列
- **端口**: 订阅ZMQ的9999端口
- **Redis频道**: `market_data`
- **特性**: 自动重连、错误处理、统计报告

### 2. WebSocket服务器 (`websocket_app.py`)
- **功能**: 提供WebSocket服务和Web界面
- **端口**: 5502
- **特性**: Flask-SocketIO、实时双向通信、房间管理

### 3. 实时行情页面 (`templates/realtime.html`)
- **功能**: 实时显示行情数据
- **特性**: 响应式设计、实时更新、状态监控

### 4. 系统管理脚本 (`start_realtime_system.sh`)
- **功能**: 统一管理所有服务
- **支持命令**: start, stop, restart, status, logs, test

## 🚀 快速开始

### 1. 启动系统
```bash
# 启动完整系统
./start_realtime_system.sh start

# 查看系统状态
./start_realtime_system.sh status

# 测试系统功能
./start_realtime_system.sh test
```

### 2. 访问界面
- **主页面**: http://localhost:5502
- **实时行情**: http://localhost:5502/realtime
- **合约查询**: http://localhost:5502 (原有功能)
- **持仓查询**: http://localhost:5502/positions
- **资金查询**: http://localhost:5502/accounts

### 3. 使用步骤
1. 打开实时行情页面
2. 点击"开始订阅"按钮
3. 系统自动接收和显示实时行情
4. 可点击"发送测试数据"测试功能

## 📊 功能特性

### 实时数据展示
- ✅ 实时价格更新
- ✅ 涨跌幅显示
- ✅ 买卖价格
- ✅ 成交量信息
- ✅ 价格变化动画效果

### 系统监控
- ✅ 连接状态指示
- ✅ 消息统计
- ✅ 系统日志
- ✅ 性能监控

### 管理功能
- ✅ 订阅/取消订阅
- ✅ 测试数据发送
- ✅ 数据清空
- ✅ 状态刷新

## 🔧 系统管理

### 服务管理命令
```bash
# 启动所有服务
./start_realtime_system.sh start

# 停止所有服务
./start_realtime_system.sh stop

# 重启服务
./start_realtime_system.sh restart

# 查看状态
./start_realtime_system.sh status

# 查看日志
./start_realtime_system.sh logs

# 测试功能
./start_realtime_system.sh test

# 仅启动WebSocket服务器
./start_realtime_system.sh start-websocket

# 仅启动行情桥接服务
./start_realtime_system.sh start-bridge
```

### 日志文件
- `websocket.log` - WebSocket服务器日志
- `bridge.log` - 行情桥接服务日志

### 手动操作
```bash
# 查看进程状态
ps aux | grep -E "(websocket_app|market_bridge)"

# 查看端口占用
netstat -tulpn | grep -E ":5502|:9999|:6379"

# 查看Redis状态
redis-cli ping
redis-cli pubsub channels
redis-cli pubsub numsub market_data

# 手动发送测试数据
curl http://localhost:5502/api/realtime/test
```

## 🛠️ 技术栈

### 后端技术
- **Python 3**: 主要开发语言
- **Flask**: Web框架
- **Flask-SocketIO**: WebSocket支持
- **Redis**: 消息队列和缓存
- **ZMQ (pyzmq)**: 消息传输
- **MySQL**: 数据存储

### 前端技术
- **HTML5**: 页面结构
- **Bootstrap 5**: UI框架
- **Socket.IO**: 客户端WebSocket
- **JavaScript**: 交互逻辑
- **Font Awesome**: 图标库

### 系统依赖
- **Redis服务器**: 消息队列
- **CTP行情服务**: 数据源 (ctpmarket)
- **MySQL数据库**: 基础数据存储

## 📈 性能指标

### 系统容量
- **并发连接**: 支持多个WebSocket客户端
- **消息处理**: 实时处理行情数据
- **数据存储**: Redis内存存储，快速访问

### 监控指标
- **接收消息数**: 累计接收的行情消息
- **活跃订阅者**: 当前WebSocket连接数
- **消息速率**: 每秒处理的消息数量
- **系统运行时间**: 服务启动时长

## 🔍 故障排除

### 常见问题

#### 1. WebSocket连接失败
- 检查5502端口是否被占用
- 确认WebSocket服务器是否启动
- 检查防火墙设置

#### 2. 行情数据不更新
- 检查CTP行情服务(ctpmarket)是否运行
- 确认ZMQ端口9999是否可访问
- 检查行情桥接服务状态

#### 3. Redis连接失败
- 确认Redis服务是否启动: `redis-cli ping`
- 检查Redis端口6379是否可访问
- 查看Redis服务日志

#### 4. 页面加载异常
- 检查`no_proxy`环境变量设置
- 确认Flask服务器启动正常
- 查看WebSocket服务器日志

### 调试命令
```bash
# 检查系统状态
./start_realtime_system.sh status

# 查看详细日志
./start_realtime_system.sh logs

# 测试系统功能
./start_realtime_system.sh test

# 重启系统
./start_realtime_system.sh restart
```

## 📝 开发说明

### 扩展功能
1. **新增行情数据类型**: 修改`market_bridge.py`中的数据解析逻辑
2. **自定义前端显示**: 修改`templates/realtime.html`和相关CSS
3. **增加数据过滤**: 在桥接服务中添加过滤规则
4. **历史数据支持**: 集成数据库存储历史行情

### 配置修改
- **端口配置**: 修改各服务脚本中的端口设置
- **Redis配置**: 调整Redis连接参数
- **ZMQ配置**: 修改ZMQ订阅地址和主题

## 🎉 系统特色

1. **🚀 高性能**: 基于事件驱动的异步架构
2. **🔄 实时性**: 毫秒级数据推送
3. **📱 响应式**: 支持多设备访问
4. **🛡️ 稳定性**: 完善的错误处理和自动重连
5. **📊 可视化**: 直观的行情数据展示
6. **🔧 易管理**: 统一的服务管理脚本

---

**系统版本**: v1.0  
**创建时间**: 2025-08-04  
**技术支持**: CTP实时行情团队 