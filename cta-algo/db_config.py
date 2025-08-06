#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
数据库配置模块
包含数据库连接参数和配置信息
"""

# 数据库连接配置
DB_CONFIG = {
    'host': '172.16.30.97',
    'port': 13307,
    'user': 'elwriter', 
    'password': 'elwriter123',
    'database': 'cta_trade',
    'charset': 'utf8mb4',
    'autocommit': True
}

# 表名配置
TABLE_NAME = 'instrument_basic_info'

# 字段映射
FIELD_MAPPING = {
    'datenum': 'int',           # 日期数字
    'timenum': 'int',           # 时间数字
    'exname': 'str',            # 交易所名称
    'pz': 'str',                # 品种
    'instrumentid': 'str',      # 合约ID
    'end_month': 'int',         # 到期月份
    'maxposition': 'int',       # 最大持仓
    'maxopen': 'int',           # 最大开仓
    'vol': 'int',               # 成交量
    'opi': 'int',               # 持仓量
    'far_ratio': 'int',         # 远月比例
    'cur_far_ratio': 'int',     # 当前远月比例
    'near_ratio': 'int',        # 近月比例
    'cur_near_ratio': 'int',    # 当前近月比例
    'tick_size': 'float',       # 最小变动价位
    'factor': 'int'             # 因子
}

# 常用的交易所列表
EXCHANGES = [
    'SHFE',     # 上海期货交易所
    'DCE',      # 大连商品交易所  
    'CZCE',     # 郑州商品交易所
    'CFFEX',    # 中国金融期货交易所
    'INE'       # 上海国际能源交易中心
]

# 常用的期货品种
COMMODITIES = [
    'CU',       # 铜
    'AL',       # 铝
    'ZN',       # 锌
    'PB',       # 铅
    'NI',       # 镍
    'SN',       # 锡
    'AU',       # 黄金
    'AG',       # 白银
    'RB',       # 螺纹钢
    'HC',       # 热轧卷板
    'FU',       # 燃料油
    'BU',       # 沥青
    'RU',       # 橡胶
    'A',        # 豆一
    'B',        # 豆二
    'M',        # 豆粕
    'Y',        # 豆油
    'C',        # 玉米
    'CS',       # 玉米淀粉
    'JM',       # 焦煤
    'J',        # 焦炭
    'I',        # 铁矿石
    'TA',       # PTA
    'MA',       # 甲醇
    'CF',       # 棉花
    'SR',       # 白糖
    'RM',       # 菜粕
    'OI',       # 菜油
    'RI',       # 早籼稻
    'WH',       # 强麦
    'PM',       # 普麦
    'JR',       # 粳稻
    'LR',       # 晚籼稻
    'IF',       # 沪深300指数期货
    'IH',       # 上证50指数期货
    'IC',       # 中证500指数期货
    'T',        # 10年期国债期货
    'TF',       # 5年期国债期货
    'TS'        # 2年期国债期货
] 