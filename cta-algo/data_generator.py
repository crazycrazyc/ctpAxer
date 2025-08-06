#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
数据生成器模块
用于生成模拟的期货合约基础信息数据
"""

import random
import datetime
from typing import List, Dict, Any
from faker import Faker
from db_config import EXCHANGES, COMMODITIES

# 初始化Faker实例
fake = Faker('zh_CN')

class InstrumentDataGenerator:
    """期货合约基础信息数据生成器"""
    
    def __init__(self):
        """初始化数据生成器"""
        self.current_date = datetime.datetime.now()
        self.base_datenum = int(self.current_date.strftime('%Y%m%d'))
        self.base_timenum = int(self.current_date.strftime('%H%M%S'))
        
    def generate_datenum(self) -> int:
        """生成日期数字 (YYYYMMDD格式)"""
        # 生成最近30天内的随机日期
        days_ago = random.randint(0, 30)
        date = self.current_date - datetime.timedelta(days=days_ago)
        return int(date.strftime('%Y%m%d'))
    
    def generate_timenum(self) -> int:
        """生成时间数字 (HHMMSS格式)"""
        # 生成交易时间段内的随机时间
        trading_hours = [
            (9, 0, 0),    # 上午开盘
            (10, 30, 0),  # 上午收盘前
            (13, 30, 0),  # 下午开盘
            (15, 0, 0),   # 下午收盘
            (21, 0, 0),   # 夜盘开盘
            (23, 30, 0)   # 夜盘收盘
        ]
        
        hour, minute, second = random.choice(trading_hours)
        minute += random.randint(-30, 30)
        second = random.randint(0, 59)
        
        # 确保时间合法
        minute = max(0, min(59, minute))
        
        return hour * 10000 + minute * 100 + second
    
    def generate_instrumentid(self, pz: str) -> str:
        """生成合约ID"""
        # 生成月份代码 (01-12)
        month = random.randint(1, 12)
        
        # 生成年份后两位 (当年或明年)
        year_suffix = random.choice([
            self.current_date.year % 100,
            (self.current_date.year + 1) % 100
        ])
        
        return f"{pz}{year_suffix:02d}{month:02d}"
    
    def generate_end_month(self) -> int:
        """生成到期月份"""
        # 生成未来1-12个月的到期月份
        future_months = []
        for i in range(1, 13):
            future_date = self.current_date + datetime.timedelta(days=30*i)
            future_months.append(int(future_date.strftime('%Y%m')))
        
        return random.choice(future_months)
    
    def generate_tick_size(self, pz: str) -> float:
        """根据品种生成最小变动价位"""
        tick_size_map = {
            # 金属类
            'CU': 10.0, 'AL': 5.0, 'ZN': 5.0, 'PB': 5.0, 'NI': 10.0, 'SN': 10.0,
            'AU': 0.02, 'AG': 1.0,
            # 钢铁类  
            'RB': 1.0, 'HC': 1.0, 'I': 0.5, 'JM': 0.5, 'J': 0.5,
            # 能源化工类
            'FU': 1.0, 'BU': 2.0, 'RU': 5.0, 'TA': 2.0, 'MA': 1.0,
            # 农产品类
            'A': 1.0, 'B': 1.0, 'M': 1.0, 'Y': 2.0, 'C': 1.0, 'CS': 1.0,
            'CF': 5.0, 'SR': 1.0, 'RM': 1.0, 'OI': 2.0,
            'RI': 1.0, 'WH': 1.0, 'PM': 1.0, 'JR': 1.0, 'LR': 1.0,
            # 金融期货类
            'IF': 0.2, 'IH': 0.2, 'IC': 0.2,
            'T': 0.005, 'TF': 0.005, 'TS': 0.005
        }
        
        return tick_size_map.get(pz, 1.0)
    
    def generate_factor(self, pz: str) -> int:
        """根据品种生成合约乘数"""
        factor_map = {
            # 金属类 (吨)
            'CU': 5, 'AL': 5, 'ZN': 5, 'PB': 5, 'NI': 1, 'SN': 1,
            'AU': 1000, 'AG': 15,  # 克
            # 钢铁类 (吨)
            'RB': 10, 'HC': 10, 'I': 100, 'JM': 60, 'J': 100,
            # 能源化工类 (吨)
            'FU': 10, 'BU': 10, 'RU': 10, 'TA': 5, 'MA': 10,
            # 农产品类 (吨)
            'A': 10, 'B': 10, 'M': 10, 'Y': 10, 'C': 10, 'CS': 10,
            'CF': 5, 'SR': 10, 'RM': 10, 'OI': 10,
            'RI': 20, 'WH': 20, 'PM': 50, 'JR': 20, 'LR': 20,
            # 金融期货类
            'IF': 300, 'IH': 300, 'IC': 200,
            'T': 10000, 'TF': 10000, 'TS': 20000
        }
        
        return factor_map.get(pz, 10)
    
    def generate_single_record(self) -> Dict[str, Any]:
        """生成单条记录"""
        # 随机选择交易所和品种
        exname = random.choice(EXCHANGES)
        pz = random.choice(COMMODITIES)
        
        # 生成合约ID
        instrumentid = self.generate_instrumentid(pz)
        
        # 生成基础数据
        record = {
            'datenum': self.generate_datenum(),
            'timenum': self.generate_timenum(),
            'exname': exname,
            'pz': pz,
            'instrumentid': instrumentid,
            'end_month': self.generate_end_month(),
            'maxposition': random.randint(50000, 9999999),
            'maxopen': random.randint(10000, 9999999),
            'vol': random.randint(0, 1000000),
            'opi': random.randint(0, 500000),
            'far_ratio': random.randint(50, 150),
            'cur_far_ratio': random.randint(0, 100),
            'near_ratio': random.randint(50, 150),
            'cur_near_ratio': random.randint(0, 100),
            'tick_size': self.generate_tick_size(pz),
            'factor': self.generate_factor(pz)
        }
        
        return record
    
    def generate_batch_records(self, count: int) -> List[Dict[str, Any]]:
        """生成批量记录"""
        records = []
        used_instrumentids = set()
        
        while len(records) < count:
            record = self.generate_single_record()
            
            # 确保instrumentid和datenum的组合唯一（根据UNIQUE KEY约束）
            unique_key = f"{record['instrumentid']}_{record['datenum']}"
            
            if unique_key not in used_instrumentids:
                used_instrumentids.add(unique_key)
                records.append(record)
        
        return records
    
    def print_record_sample(self, record: Dict[str, Any]) -> None:
        """打印记录样本用于调试"""
        print("=" * 60)
        print("数据样本:")
        print("-" * 60)
        for key, value in record.items():
            print(f"{key:15}: {value}")
        print("=" * 60)


def main():
    """测试数据生成器"""
    generator = InstrumentDataGenerator()
    
    print("🧪 数据生成器测试")
    print("=" * 50)
    
    # 生成单条记录测试
    single_record = generator.generate_single_record()
    generator.print_record_sample(single_record)
    
    # 生成批量记录测试
    print(f"\n📊 生成5条测试记录:")
    batch_records = generator.generate_batch_records(5)
    
    for i, record in enumerate(batch_records, 1):
        print(f"\n记录 {i}:")
        print(f"  合约: {record['instrumentid']} ({record['pz']}-{record['exname']})")
        print(f"  日期: {record['datenum']} 时间: {record['timenum']}")
        print(f"  成交量: {record['vol']:,} 持仓: {record['opi']:,}")
        print(f"  最小变动: {record['tick_size']} 合约乘数: {record['factor']}")
    
    print(f"\n✅ 数据生成器测试完成！")


if __name__ == "__main__":
    main() 