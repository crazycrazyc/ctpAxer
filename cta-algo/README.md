# CTA算法 - 期货合约基础信息数据插入工具

## 📋 项目概述

本项目是一个专门用于向 `instrument_basic_info` 表插入期货合约基础信息数据的Python工具。支持生成模拟的期货合约数据并批量插入到MySQL数据库中。

## 🗂️ 文件结构

```
cta-algo/
├── data.sql              # 数据库表结构定义
├── requirements.txt      # Python依赖包
├── db_config.py         # 数据库配置模块
├── data_generator.py    # 数据生成器模块
├── insert_data.py       # 主要的数据插入脚本
└── README.md           # 项目说明文档
```

## 🔧 环境配置

### 1. 安装Python依赖

```bash
cd /home/scott/projects/AIOperations/cta-algo
pip install -r requirements.txt
```

### 2. 数据库配置

数据库连接信息已在 `db_config.py` 中配置：

- **主机**: 172.16.30.97
- **端口**: 13307
- **用户**: elwriter
- **密码**: elwriter123
- **数据库**: cta_trader
- **表名**: instrument_basic_info

## 📊 表结构说明

`instrument_basic_info` 表包含以下字段：

| 字段名 | 类型 | 说明 |
|--------|------|------|
| id | bigint(11) | 主键，自增 |
| datenum | int(11) | 日期数字 (YYYYMMDD) |
| timenum | int(11) | 时间数字 (HHMMSS) |
| exname | varchar(100) | 交易所名称 |
| pz | varchar(50) | 品种代码 |
| instrumentid | varchar(50) | 合约ID |
| end_month | int(11) | 到期月份 |
| maxposition | int(11) | 最大持仓 |
| maxopen | int(11) | 最大开仓 |
| vol | int(11) | 成交量 |
| opi | int(11) | 持仓量 |
| far_ratio | int(11) | 远月比例 |
| cur_far_ratio | int(10) | 当前远月比例 |
| near_ratio | int(11) | 近月比例 |
| cur_near_ratio | int(10) | 当前近月比例 |
| tick_size | double(7,4) | 最小变动价位 |
| factor | int(11) | 合约乘数 |
| update_time | TIMESTAMP | 更新时间 |

## 🚀 使用方法

### 1. 测试数据生成器

```bash
python data_generator.py
```

这将运行数据生成器的测试模式，展示生成的数据样本。

### 2. 插入100条记录

```bash
python insert_data.py
```

程序将：
1. 进行数据库连接测试
2. 检查表结构
3. 生成100条模拟数据
4. 批量插入到数据库
5. 显示插入结果和统计信息

### 3. 自定义插入数量

修改 `insert_data.py` 中的 `main()` 函数：

```python
def main():
    inserter = DataInserter()
    inserter.run(count=500)  # 插入500条记录
```

## 📈 数据生成特点

### 支持的交易所
- **SHFE**: 上海期货交易所
- **DCE**: 大连商品交易所
- **CZCE**: 郑州商品交易所
- **CFFEX**: 中国金融期货交易所
- **INE**: 上海国际能源交易中心

### 支持的期货品种
涵盖主要期货品种：
- **金属类**: CU(铜), AL(铝), ZN(锌), AU(黄金), AG(白银)等
- **钢铁类**: RB(螺纹钢), HC(热轧卷板), I(铁矿石)等
- **能源化工**: FU(燃料油), BU(沥青), RU(橡胶)等
- **农产品**: A(豆一), M(豆粕), C(玉米), CF(棉花)等
- **金融期货**: IF(沪深300), IH(上证50), IC(中证500)等

### 智能数据生成
- **合约ID**: 根据品种和年月自动生成
- **最小变动价位**: 根据品种特点设置真实的tick_size
- **合约乘数**: 根据品种设置符合实际的factor
- **交易时间**: 生成符合期货交易时段的时间
- **唯一性保证**: 确保instrumentid和datenum组合唯一

## 🔍 运行示例

```bash
$ python insert_data.py

🎯 期货合约基础信息数据插入程序
======================================================================
🔍 运行预检查...
--------------------------------------------------
1. 测试数据库连接...
✅ 数据库连接成功
2. 检查表 instrument_basic_info 结构...
✅ 表 instrument_basic_info 结构正常，包含 18 个字段
📊 当前表中记录数: 681,484 条
✅ 预检查通过！

❓ 确认要插入 100 条测试数据吗？
请输入 y/yes 确认，其他键取消: y

🚀 开始插入 100 条测试数据...
--------------------------------------------------
📊 生成测试数据...
✅ 成功生成 100 条数据

📋 数据样本:
  datenum        : 20240722
  timenum        : 213045
  exname         : SHFE
  pz             : CU
  instrumentid   : CU2408
  end_month      : 202408
  maxposition    : 8888888
  maxopen        : 5555555
  vol            : 125000
  opi            : 45000
  far_ratio      : 95
  cur_far_ratio  : 12
  near_ratio     : 105
  cur_near_ratio : 8
  tick_size      : 10.0
  factor         : 5

💾 开始插入数据...
✅ 批次 1: 成功插入 20 条记录 (总进度: 20/100)
✅ 批次 2: 成功插入 20 条记录 (总进度: 40/100)
✅ 批次 3: 成功插入 20 条记录 (总进度: 60/100)
✅ 批次 4: 成功插入 20 条记录 (总进度: 80/100)
✅ 批次 5: 成功插入 20 条记录 (总进度: 100/100)

📈 插入结果:
  成功插入: 100/100 条
  成功率: 100.0%
  耗时: 1.23 秒
  平均速度: 81.3 条/秒
🎉 所有数据插入成功！

📋 显示最新插入的 5 条记录:
--------------------------------------------------------------------------------

记录 1:
  ID: 681585
  合约: RB2409 (RB-SHFE)
  日期: 20240721 时间: 143022
  成交量: 234,567 持仓: 123,456
  最小变动: 1.0 合约乘数: 10
  更新时间: 2024-07-22 11:05:15

📊 最终统计:
  表中总记录数: 681,584 条
✅ 数据插入任务完成！
```

## ⚠️ 注意事项

1. **数据库权限**: 确保用户 `elwriter` 有对表的INSERT权限
2. **唯一约束**: 系统会自动处理 `instrumentid` 和 `datenum` 的唯一性约束
3. **网络连接**: 确保能够访问数据库服务器 172.16.30.97:13307
4. **批量处理**: 默认每批次插入20条记录，可根据需要调整

## 🔧 自定义配置

### 修改插入批次大小

在 `insert_data.py` 中修改：

```python
def insert_test_data(self, count: int = 100, batch_size: int = 50):  # 改为50
```

### 添加新的期货品种

在 `db_config.py` 中的 `COMMODITIES` 列表中添加：

```python
COMMODITIES = [
    # 现有品种...
    'NEW_COMMODITY',  # 添加新品种
]
```

### 自定义tick_size和factor

在 `data_generator.py` 中修改相应的映射表：

```python
tick_size_map = {
    'NEW_COMMODITY': 0.5,  # 自定义最小变动价位
}

factor_map = {
    'NEW_COMMODITY': 20,   # 自定义合约乘数
}
```

## 📞 技术支持

如有问题，请检查：
1. 数据库连接是否正常
2. Python依赖是否正确安装
3. 网络连接是否畅通
4. 数据库用户权限是否充足 