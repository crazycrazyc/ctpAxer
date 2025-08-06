# 配置文件格式变更说明

## 概述

本项目已将配置文件从 `.cfg` 格式改为 `.json` 格式，以提供更好的可读性和结构化配置管理。

## 变更内容

### 1. 配置文件格式

**原格式 (config.cfg):**
```
md_server=114.94.16.137:33417
td_server=114.94.16.137:33415
brokerid=0043
userid=12008000
password=1656.shg
appid=client_AAAhAA_1.0.0
authcode=U4FEJB7TU7VPNRBT
dayend=09:00:00
nightend=15:00:00
ZMQServer=tcp://127.0.0.1:5555
DEC=a2507
```

**新格式 (config.json):**
```json
{
    "md_server": "114.94.16.137:33417",
    "td_server": "114.94.16.137:33415",
    "brokerid": "0043",
    "userid": "12008000",
    "password": "1656.shg",
    "appid": "client_AAAhAA_1.0.0",
    "authcode": "U4FEJB7TU7VPNRBT",
    "dayend": "09:00:00",
    "nightend": "15:00:00",
    "ZMQServer": "tcp://127.0.0.1:5555",
    "DEC": "a2507"
}
```

### 2. 代码变更

#### 新增文件
- `include/JsonConfig.h` - JSON配置读取类头文件
- `src/JsonConfig.cpp` - JSON配置读取类实现
- `src/testJsonConfig.cpp` - 配置读取测试程序

#### 修改文件
- `src/appConfig.cpp` - 修改为使用JsonConfig类
- `CMakeLists.txt` - 添加新的源文件到编译列表

### 3. 配置读取方式

**原来的读取方式:**
```cpp
#include "Config.h"
Config cfg("config.cfg");
string value = cfg.Read<string>("key");
```

**新的读取方式:**
```cpp
#include "JsonConfig.h"
JsonConfig cfg("config.json");
string value = cfg.Read<string>("key");
```

### 4. 新功能特性

#### 支持的数据类型
- 字符串: `cfg.Read<string>("key")`
- 整数: `cfg.Read<int>("key")`
- 浮点数: `cfg.Read<double>("key")`
- 布尔值: `cfg.Read<bool>("key")`

#### 默认值支持
```cpp
string value = cfg.Read<string>("key", "default_value");
int number = cfg.Read<int>("key", 100);
```

#### 数组读取
```cpp
vector<string> symbols = cfg.ReadArray("DEC");
```

#### 键存在性检查
```cpp
if (cfg.KeyExists("key")) {
    // 键存在
}
```

#### 异常处理
```cpp
try {
    JsonConfig cfg("config.json");
    string value = cfg.Read<string>("key");
} catch (const JsonConfig::FileNotFound& e) {
    // 配置文件未找到
} catch (const JsonConfig::KeyNotFound& e) {
    // 配置键未找到
}
```

## 编译和测试

### 编译
```bash
cd ctp-master/ctprawtick
mkdir -p build && cd build
cmake .. && make
```

### 测试配置读取
```bash
cd bin/debug
./testJsonConfig
```

### 运行主程序
```bash
cd bin/debug
./zyctestmysql
```

## 兼容性说明

- 旧的 `config.cfg` 文件仍然保留，但程序现在使用 `config.json`
- 如果需要回退到旧格式，只需修改 `appConfig.cpp` 中的文件路径和配置类
- 新的 JSON 格式提供了更好的可读性和扩展性

## 配置项说明

| 配置项 | 类型 | 说明 |
|--------|------|------|
| md_server | string | 行情服务器地址 |
| td_server | string | 交易服务器地址 |
| brokerid | string | 经纪商ID |
| userid | string | 用户ID |
| password | string | 密码 |
| appid | string | 应用ID |
| authcode | string | 认证码 |
| dayend | string | 日盘结束时间 |
| nightend | string | 夜盘结束时间 |
| ZMQServer | string | ZMQ服务器地址 |
| DEC | string | 合约代码（逗号分隔） | 