#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
数据插入模块
连接数据库并插入期货合约基础信息数据
"""

import pymysql
import sys
import time
from typing import List, Dict, Any, Optional
from contextlib import contextmanager

from db_config import DB_CONFIG, TABLE_NAME
from data_generator import InstrumentDataGenerator


class DatabaseManager:
    """数据库管理器"""
    
    def __init__(self, config: Dict[str, Any]):
        """初始化数据库管理器"""
        self.config = config
        self.connection = None
        
    @contextmanager
    def get_connection(self):
        """获取数据库连接的上下文管理器"""
        try:
            self.connection = pymysql.connect(**self.config)
            yield self.connection
        except Exception as e:
            print(f"❌ 数据库连接失败: {e}")
            raise
        finally:
            if self.connection:
                self.connection.close()
                self.connection = None
    
    def test_connection(self) -> bool:
        """测试数据库连接"""
        try:
            with self.get_connection() as conn:
                with conn.cursor() as cursor:
                    cursor.execute("SELECT 1")
                    result = cursor.fetchone()
                    return result[0] == 1
        except Exception as e:
            print(f"❌ 连接测试失败: {e}")
            return False
    
    def get_table_info(self, table_name: str) -> Optional[List[Dict]]:
        """获取表结构信息"""
        try:
            with self.get_connection() as conn:
                with conn.cursor(pymysql.cursors.DictCursor) as cursor:
                    cursor.execute(f"DESCRIBE {table_name}")
                    return cursor.fetchall()
        except Exception as e:
            print(f"❌ 获取表结构失败: {e}")
            return None
    
    def insert_single_record(self, table_name: str, record: Dict[str, Any]) -> bool:
        """插入单条记录"""
        try:
            # 构建SQL语句
            columns = ', '.join(record.keys())
            placeholders = ', '.join(['%s'] * len(record))
            sql = f"INSERT INTO {table_name} ({columns}) VALUES ({placeholders})"
            
            with self.get_connection() as conn:
                with conn.cursor() as cursor:
                    cursor.execute(sql, list(record.values()))
                    conn.commit()
                    return True
                    
        except Exception as e:
            print(f"❌ 插入单条记录失败: {e}")
            return False
    
    def insert_batch_records(self, table_name: str, records: List[Dict[str, Any]], 
                           batch_size: int = 50) -> int:
        """批量插入记录"""
        if not records:
            return 0
        
        success_count = 0
        total_count = len(records)
        
        try:
            # 构建SQL语句
            columns = ', '.join(records[0].keys())
            placeholders = ', '.join(['%s'] * len(records[0]))
            sql = f"INSERT INTO {table_name} ({columns}) VALUES ({placeholders})"
            
            with self.get_connection() as conn:
                with conn.cursor() as cursor:
                    # 分批处理
                    for i in range(0, total_count, batch_size):
                        batch = records[i:i + batch_size]
                        batch_values = [list(record.values()) for record in batch]
                        
                        try:
                            cursor.executemany(sql, batch_values)
                            conn.commit()
                            success_count += len(batch)
                            
                            print(f"✅ 批次 {i//batch_size + 1}: 成功插入 {len(batch)} 条记录 "
                                  f"(总进度: {success_count}/{total_count})")
                                  
                        except Exception as batch_error:
                            print(f"❌ 批次 {i//batch_size + 1} 插入失败: {batch_error}")
                            # 尝试逐条插入失败的批次
                            for record in batch:
                                if self.insert_single_record(table_name, record):
                                    success_count += 1
                            
        except Exception as e:
            print(f"❌ 批量插入失败: {e}")
        
        return success_count
    
    def get_record_count(self, table_name: str) -> int:
        """获取表中记录数量"""
        try:
            with self.get_connection() as conn:
                with conn.cursor() as cursor:
                    cursor.execute(f"SELECT COUNT(*) FROM {table_name}")
                    result = cursor.fetchone()
                    return result[0]
        except Exception as e:
            print(f"❌ 获取记录数量失败: {e}")
            return -1
    
    def get_latest_records(self, table_name: str, limit: int = 5) -> List[Dict]:
        """获取最新的几条记录"""
        try:
            with self.get_connection() as conn:
                with conn.cursor(pymysql.cursors.DictCursor) as cursor:
                    cursor.execute(f"""
                        SELECT * FROM {table_name} 
                        ORDER BY update_time DESC 
                        LIMIT %s
                    """, (limit,))
                    return cursor.fetchall()
        except Exception as e:
            print(f"❌ 获取最新记录失败: {e}")
            return []


class DataInserter:
    """数据插入器"""
    
    def __init__(self):
        """初始化数据插入器"""
        self.db_manager = DatabaseManager(DB_CONFIG)
        self.data_generator = InstrumentDataGenerator()
    
    def run_pre_check(self) -> bool:
        """运行预检查"""
        print("🔍 运行预检查...")
        print("-" * 50)
        
        # 测试数据库连接
        print("1. 测试数据库连接...")
        if not self.db_manager.test_connection():
            print("❌ 数据库连接失败！")
            return False
        print("✅ 数据库连接成功")
        
        # 检查表结构
        print(f"2. 检查表 {TABLE_NAME} 结构...")
        table_info = self.db_manager.get_table_info(TABLE_NAME)
        if not table_info:
            print(f"❌ 表 {TABLE_NAME} 不存在或无法访问！")
            return False
        
        print(f"✅ 表 {TABLE_NAME} 结构正常，包含 {len(table_info)} 个字段")
        
        # 显示当前记录数
        current_count = self.db_manager.get_record_count(TABLE_NAME)
        if current_count >= 0:
            print(f"📊 当前表中记录数: {current_count:,} 条")
        
        print("✅ 预检查通过！")
        return True
    
    def insert_test_data(self, count: int = 100, batch_size: int = 20) -> bool:
        """插入测试数据"""
        print(f"\n🚀 开始插入 {count} 条测试数据...")
        print("-" * 50)
        
        start_time = time.time()
        
        try:
            # 生成数据
            print("📊 生成测试数据...")
            records = self.data_generator.generate_batch_records(count)
            print(f"✅ 成功生成 {len(records)} 条数据")
            
            # 显示数据样本
            print(f"\n📋 数据样本:")
            sample_record = records[0]
            for key, value in sample_record.items():
                print(f"  {key:15}: {value}")
            
            # 插入数据
            print(f"\n💾 开始插入数据...")
            success_count = self.db_manager.insert_batch_records(
                TABLE_NAME, records, batch_size
            )
            
            end_time = time.time()
            elapsed_time = end_time - start_time
            
            # 显示结果
            print(f"\n📈 插入结果:")
            print(f"  成功插入: {success_count}/{count} 条")
            print(f"  成功率: {success_count/count*100:.1f}%")
            print(f"  耗时: {elapsed_time:.2f} 秒")
            print(f"  平均速度: {success_count/elapsed_time:.1f} 条/秒")
            
            if success_count == count:
                print("🎉 所有数据插入成功！")
                return True
            else:
                print(f"⚠️ 部分数据插入失败，失败 {count - success_count} 条")
                return False
                
        except Exception as e:
            print(f"❌ 插入数据过程中发生错误: {e}")
            return False
    
    def show_inserted_data(self, limit: int = 5) -> None:
        """显示已插入的数据"""
        print(f"\n📋 显示最新插入的 {limit} 条记录:")
        print("-" * 80)
        
        records = self.db_manager.get_latest_records(TABLE_NAME, limit)
        
        if not records:
            print("❌ 未找到记录")
            return
        
        for i, record in enumerate(records, 1):
            print(f"\n记录 {i}:")
            print(f"  ID: {record.get('id', 'N/A')}")
            print(f"  合约: {record.get('instrumentid', 'N/A')} "
                  f"({record.get('pz', 'N/A')}-{record.get('exname', 'N/A')})")
            print(f"  日期: {record.get('datenum', 'N/A')} "
                  f"时间: {record.get('timenum', 'N/A')}")
            print(f"  成交量: {record.get('vol', 'N/A'):,} "
                  f"持仓: {record.get('opi', 'N/A'):,}")
            print(f"  最小变动: {record.get('tick_size', 'N/A')} "
                  f"合约乘数: {record.get('factor', 'N/A')}")
            print(f"  更新时间: {record.get('update_time', 'N/A')}")
    
    def run(self, count: int = 100) -> None:
        """运行数据插入流程"""
        print("🎯 期货合约基础信息数据插入程序")
        print("=" * 70)
        
        # 预检查
        if not self.run_pre_check():
            print("❌ 预检查失败，程序退出")
            sys.exit(1)
        
        # 用户确认
        print(f"\n❓ 确认要插入 {count} 条测试数据吗？")
        choice = input("请输入 y/yes 确认，其他键取消: ").lower().strip()
        
        if choice not in ['y', 'yes']:
            print("🚫 操作已取消")
            return
        
        # 插入数据
        success = self.insert_test_data(count)
        
        # 显示结果
        if success:
            self.show_inserted_data()
            
            # 显示最终统计
            final_count = self.db_manager.get_record_count(TABLE_NAME)
            print(f"\n📊 最终统计:")
            print(f"  表中总记录数: {final_count:,} 条")
            print("✅ 数据插入任务完成！")
        else:
            print("❌ 数据插入任务失败！")


def main():
    """主程序入口"""
    try:
        inserter = DataInserter()
        inserter.run(count=100)
        
    except KeyboardInterrupt:
        print("\n\n🛑 程序被用户中断")
    except Exception as e:
        print(f"\n❌ 程序运行错误: {e}")
        sys.exit(1)


if __name__ == "__main__":
    main() 