# CTP合约信息查询系统

这是一个基于Flask的Web应用程序，用于查询和管理CTP合约信息，支持搜索、分页和CSV导出功能。

## 功能特性

- 🔍 **合约搜索**: 支持按合约代码、名称、交易所搜索
- 📊 **数据分页**: 高效的分页显示，支持大量数据
- 📋 **详细信息**: 点击查看合约完整详情
- 📈 **统计信息**: 实时显示合约统计数据
- 📥 **CSV导出**: 支持多种导出方式
  - 导出当前搜索结果
  - 导出全部合约数据  
  - 导出指定合约代码
- 📱 **响应式设计**: 支持桌面和移动设备
- 🎨 **现代化界面**: Bootstrap 5 + Font Awesome图标

## 系统要求

- Python 3.7+
- MySQL 数据库
- 现代浏览器

## 安装和运行

### 1. 安装依赖

```bash
pip install -r requirements.txt
```

### 2. 配置数据库

确保MySQL数据库服务正在运行，并且有一个名为`cta_trade`的数据库，包含`instruments`表。

数据库配置在`websocket_app.py`中：
```python
DB_CONFIG = {
    'host': '172.16.30.97',
    'port': 13306,
    'user': 'elwriter',
    'password': 'elwriter123',
    'database': 'cta_trade',
    'charset': 'utf8mb4'
}
```

### 3. 运行应用

```bash
python websocket_app.py
```

应用将在 `http://localhost:5000` 启动。

## 数据库表结构

应用期望`instruments`表包含以下字段：

```sql
CREATE TABLE instruments (
    InstrumentID VARCHAR(31) PRIMARY KEY,
    InstrumentName VARCHAR(50),
    ExchangeID VARCHAR(9),
    ExchangeInstID VARCHAR(31),
    ProductID VARCHAR(31),
    ProductClass CHAR(1),
    DeliveryYear INT,
    DeliveryMonth INT,
    VolumeMultiple INT,
    PriceTick DOUBLE,
    CreateDate VARCHAR(9),
    OpenDate VARCHAR(9),
    ExpireDate VARCHAR(9),
    StartDelivDate VARCHAR(9),
    EndDelivDate VARCHAR(9),
    InstLifePhase CHAR(1),
    IsTrading INT,
    PositionType CHAR(1),
    PositionDateType CHAR(1),
    LongMarginRatio DOUBLE,
    ShortMarginRatio DOUBLE,
    -- 其他字段...
);
```

## API接口

### 获取合约列表
```
GET /api/instruments?page=1&per_page=20&search=关键词
```

### 获取合约详情
```
GET /api/instrument/<instrument_id>
```

### 导出CSV
```
GET /api/export/csv?search=关键词&instruments=CU2502,AL2502
```

### 获取统计信息
```
GET /api/stats
```

## 文件结构

```
web-interface/
├── websocket_app.py                 # Flask主应用
├── requirements.txt       # Python依赖
├── README.md             # 项目说明
├── templates/
│   └── index.html        # 主页面模板
├── static/
│   ├── css/
│   │   └── style.css     # 样式文件
│   └── js/
│       └── app.js        # JavaScript逻辑
```

## 使用说明

### 基本查询
1. 打开浏览器访问 `http://localhost:5000`
2. 在搜索框中输入合约代码、名称或交易所
3. 点击搜索或按回车键

### 查看详情
- 点击表格中的"详情"按钮查看合约完整信息

### 导出数据
- **导出当前结果**: 导出当前搜索条件下的所有数据
- **导出全部**: 导出数据库中的所有合约
- **导出指定合约**: 在侧边栏输入具体合约代码（用逗号分隔）

### 分页浏览
- 使用底部分页控件浏览大量数据
- 支持跳转到指定页面

## 技术栈

- **后端**: Flask (Python)
- **前端**: Bootstrap 5, JavaScript
- **数据库**: MySQL
- **图标**: Font Awesome
- **样式**: 自定义CSS + Bootstrap

## 注意事项

1. 确保数据库连接配置正确
2. 合约数据需要通过CTP系统预先导入到数据库
3. 大量数据导出可能需要一些时间
4. 建议在生产环境中使用WSGI服务器（如Gunicorn）

## 故障排除

### 数据库连接失败
- 检查数据库服务是否运行
- 验证连接参数是否正确
- 确认防火墙设置

### 页面加载缓慢
- 检查数据库查询性能
- 考虑添加适当的索引
- 调整分页大小

### CSV导出失败
- 检查服务器磁盘空间
- 验证数据库查询权限
- 确认浏览器下载设置 