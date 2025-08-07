# Redis 安装和配置指南

## 📋 概述

Redis是CTPRawTick系统中的重要组件，用于实时数据缓存和消息分发。本文档详细说明Redis的安装、配置和使用方法。

## 🎯 Redis在系统中的作用

```
ctpmarket → ZMQ → market_bridge.py → Redis Pub/Sub → websocket_app.py → Web前端
```

- **实时缓存**: 存储最新的行情数据
- **消息分发**: 通过Pub/Sub机制向Web前端推送实时数据
- **性能优化**: 减少数据库查询压力

## 🚀 安装Redis

### 方法一：使用包管理器安装（推荐）

#### Ubuntu/Debian系统
```bash
# 更新包列表
sudo apt update

# 安装Redis服务器
sudo apt install redis-server

# 查看版本
redis-server --version
```

#### CentOS/RHEL系统
```bash
# 安装EPEL仓库
sudo yum install epel-release

# 安装Redis
sudo yum install redis

# 或者在CentOS 8+/RHEL 8+上使用dnf
sudo dnf install redis
```

#### 使用Snap安装（跨平台）
```bash
sudo snap install redis
```

### 方法二：从源码编译安装

```bash
# 安装编译依赖
sudo apt install build-essential tcl

# 下载Redis源码
wget http://download.redis.io/redis-stable.tar.gz
tar xzf redis-stable.tar.gz
cd redis-stable

# 编译和安装
make
sudo make install

# 创建配置目录
sudo mkdir /etc/redis
sudo cp redis.conf /etc/redis/
```

### 方法三：使用Docker安装

```bash
# 拉取Redis镜像
docker pull redis:latest

# 运行Redis容器
docker run -d \
  --name redis-server \
  -p 6379:6379 \
  -v redis-data:/data \
  redis:latest redis-server --appendonly yes

# 查看运行状态
docker ps | grep redis
```

## ⚙️ Redis配置

### 1. 基础配置文件

编辑Redis配置文件：
```bash
sudo nano /etc/redis/redis.conf
```

### 2. 关键配置项

#### 网络配置
```conf
# 绑定地址（允许所有IP访问，生产环境需谨慎）
bind 0.0.0.0

# 端口号
port 6379

# 关闭保护模式（开发环境）
protected-mode no

# 设置密码（可选但推荐）
requirepass your_password_here
```

#### 内存配置
```conf
# 最大内存使用量（根据系统内存调整）
maxmemory 1gb

# 内存淘汰策略
maxmemory-policy allkeys-lru
```

#### 持久化配置
```conf
# RDB持久化
save 900 1
save 300 10
save 60 10000

# AOF持久化
appendonly yes
appendfsync everysec
```

#### 日志配置
```conf
# 日志级别
loglevel notice

# 日志文件
logfile /var/log/redis/redis-server.log
```

### 3. 为CTPRawTick优化的配置

创建专用配置文件：
```bash
sudo nano /etc/redis/ctprawtick.conf
```

```conf
# CTPRawTick专用Redis配置
port 6379
bind 0.0.0.0
protected-mode no

# 数据库数量
databases 16

# 内存配置（根据实际情况调整）
maxmemory 2gb
maxmemory-policy allkeys-lru

# 持久化配置
save 900 1
save 300 10
save 60 10000
appendonly yes
appendfsync everysec

# 网络配置
timeout 0
keepalive 300

# 日志配置
loglevel notice
logfile /var/log/redis/ctprawtick-redis.log

# 客户端配置
maxclients 10000

# Pub/Sub优化
client-output-buffer-limit pubsub 32mb 8mb 60
```

## 🔧 启动和管理Redis

### 1. 系统服务管理

#### 启动Redis服务
```bash
# 使用systemd启动
sudo systemctl start redis-server

# 或使用service命令
sudo service redis-server start
```

#### 设置开机自启
```bash
sudo systemctl enable redis-server
```

#### 查看服务状态
```bash
sudo systemctl status redis-server
```

#### 重启Redis服务
```bash
sudo systemctl restart redis-server
```

#### 停止Redis服务
```bash
sudo systemctl stop redis-server
```

### 2. 手动启动

```bash
# 使用默认配置启动
redis-server

# 使用指定配置文件启动
redis-server /etc/redis/ctprawtick.conf

# 后台启动
redis-server /etc/redis/ctprawtick.conf --daemonize yes
```

## 🔍 Redis测试和验证

### 1. 连接测试

```bash
# 连接到Redis
redis-cli

# 连接到指定主机和端口
redis-cli -h localhost -p 6379

# 如果设置了密码
redis-cli -h localhost -p 6379 -a your_password
```

### 2. 基本命令测试

```bash
# 在redis-cli中执行以下命令
127.0.0.1:6379> ping
PONG

# 设置和获取键值
127.0.0.1:6379> set test_key "Hello Redis"
OK
127.0.0.1:6379> get test_key
"Hello Redis"

# 查看服务器信息
127.0.0.1:6379> info server

# 查看内存使用
127.0.0.1:6379> info memory
```

### 3. Pub/Sub功能测试

#### 订阅者终端
```bash
redis-cli
127.0.0.1:6379> subscribe market_data
Reading messages... (press Ctrl-C to quit)
1) "subscribe"
2) "market_data"
3) (integer) 1
```

#### 发布者终端
```bash
redis-cli
127.0.0.1:6379> publish market_data "test message"
(integer) 1
```

## 🔧 CTPRawTick集成配置

### 1. Python Redis客户端安装

```bash
# 安装redis-py
pip3 install redis

# 或者指定版本
pip3 install redis==4.5.4
```

### 2. 配置market_bridge.py

确保market_bridge.py中的Redis配置正确：
```python
# Redis配置
self.redis_client = redis.Redis(
    host='localhost', 
    port=6379, 
    db=0, 
    decode_responses=True,
    password='your_password'  # 如果设置了密码
)
```

### 3. 配置websocket_app.py

确保websocket_app.py中的Redis配置正确：
```python
# Redis连接配置
redis_client = redis.Redis(
    host='localhost',
    port=6379,
    db=0,
    decode_responses=True
)
```

## 📊 性能监控和优化

### 1. 监控Redis性能

```bash
# 实时监控Redis命令
redis-cli monitor

# 查看慢查询日志
redis-cli slowlog get 10

# 查看客户端连接
redis-cli client list

# 查看内存使用详情
redis-cli --bigkeys
```

### 2. 性能优化建议

#### 内存优化
```conf
# 启用内存压缩
hash-max-ziplist-entries 512
hash-max-ziplist-value 64
list-max-ziplist-entries 512
list-max-ziplist-value 64
set-max-intset-entries 512
zset-max-ziplist-entries 128
zset-max-ziplist-value 64
```

#### 网络优化
```conf
# TCP keepalive
tcp-keepalive 300

# 禁用Nagle算法
tcp-nodelay yes
```

## 🛠️ 故障排查

### 1. 常见问题

#### Redis无法启动
```bash
# 检查端口占用
sudo netstat -tlnp | grep 6379

# 检查日志文件
sudo tail -f /var/log/redis/redis-server.log

# 检查配置文件语法
redis-server /etc/redis/redis.conf --test-config
```

#### 连接被拒绝
```bash
# 检查防火墙设置
sudo ufw status
sudo ufw allow 6379

# 检查Redis绑定地址
grep "^bind" /etc/redis/redis.conf
```

#### 内存不足
```bash
# 查看内存使用
redis-cli info memory

# 清理过期键
redis-cli --eval "return redis.call('flushdb')" 0
```

### 2. 日志分析

```bash
# 查看Redis日志
sudo tail -f /var/log/redis/redis-server.log

# 查看系统日志
sudo journalctl -u redis-server -f
```

## 🔒 安全配置

### 1. 访问控制

```conf
# 设置密码
requirepass strong_password_here

# 重命名危险命令
rename-command FLUSHDB ""
rename-command FLUSHALL ""
rename-command CONFIG "CONFIG_9bf2f1ce016dccd2"
```

### 2. 网络安全

```conf
# 绑定到特定IP
bind 127.0.0.1 192.168.1.100

# 启用保护模式
protected-mode yes
```

### 3. 防火墙配置

```bash
# UFW防火墙配置
sudo ufw allow from 192.168.1.0/24 to any port 6379
sudo ufw deny 6379
```

## 📚 维护和备份

### 1. 数据备份

```bash
# 手动保存RDB快照
redis-cli bgsave

# 复制RDB文件
sudo cp /var/lib/redis/dump.rdb /backup/redis-backup-$(date +%Y%m%d).rdb
```

### 2. 自动备份脚本

```bash
#!/bin/bash
# redis_backup.sh
BACKUP_DIR="/backup/redis"
DATE=$(date +%Y%m%d_%H%M%S)

mkdir -p $BACKUP_DIR
redis-cli bgsave
sleep 5
cp /var/lib/redis/dump.rdb $BACKUP_DIR/redis-backup-$DATE.rdb

# 清理7天前的备份
find $BACKUP_DIR -name "redis-backup-*.rdb" -mtime +7 -delete
```

### 3. 定期维护

```bash
# 添加到crontab
crontab -e

# 每天凌晨2点备份
0 2 * * * /path/to/redis_backup.sh

# 每周清理过期键
0 3 * * 0 redis-cli --eval "redis.call('expire', KEYS[1], 0)" 1
```

## ✅ 验证安装

完成安装后，运行以下检查：

```bash
# 1. 检查Redis服务状态
sudo systemctl status redis-server

# 2. 检查端口监听
sudo netstat -tlnp | grep 6379

# 3. 测试连接
redis-cli ping

# 4. 测试Pub/Sub
redis-cli publish test "installation complete"
```

## 📞 技术支持

如果遇到问题，请检查：
1. Redis服务是否正常运行
2. 防火墙和网络配置
3. 配置文件语法是否正确
4. 系统资源是否充足

---

**注意**: 生产环境中请务必设置密码并配置适当的安全策略。 