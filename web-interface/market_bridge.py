#!/usr/bin/env python3
"""
行情数据桥接服务
从ZMQ订阅行情数据，转发到Redis供WebSocket使用，并写入数据库
"""

import zmq
import redis
import json
import time
import threading
import signal
import sys
from datetime import datetime
import traceback
import mysql.connector
from mysql.connector import Error
import struct
import os

# 设置protobuf环境变量
os.environ['PROTOCOL_BUFFERS_PYTHON_IMPLEMENTATION'] = 'python'

from protobuf_parser import ProtobufParser

class MarketDataBridge:
    def __init__(self):
        # ZMQ配置
        self.zmq_context = zmq.Context()
        self.zmq_subscriber = None
        self.zmq_url = "tcp://localhost:9999"  # 行情数据端口
        
        # Redis配置
        self.redis_client = redis.Redis(host='localhost', port=6379, db=0, decode_responses=True)
        self.redis_channel = "market_data"  # Redis频道名称
        
        # 数据库配置
        self.db_config = {
            'host': '172.16.30.97',
            'port': 13306,
            'user': 'elwriter',
            'password': 'elwriter123',
            'database': 'cta_trade',
            'autocommit': True,
            'charset': 'utf8mb4'
        }
        self.db_connection = None
        
        # 运行控制
        self.running = False
        self.stats = {
            'received': 0,
            'published': 0,
            'db_inserted': 0,
            'errors': 0,
            'start_time': None
        }
        
        # 信号处理
        signal.signal(signal.SIGINT, self.signal_handler)
        signal.signal(signal.SIGTERM, self.signal_handler)
    
    def signal_handler(self, signum, frame):
        """信号处理器"""
        print(f"\n收到信号 {signum}，正在停止服务...")
        self.running = False
    
    def connect_database(self):
        """连接数据库"""
        try:
            self.db_connection = mysql.connector.connect(**self.db_config)
            print("✅ 数据库连接成功")
            return True
        except Error as e:
            print(f"❌ 数据库连接失败: {e}")
            return False
    
    def insert_market_data_to_db(self, market_data_proto):
        """将protobuf行情数据插入数据库"""
        if not self.db_connection or not self.db_connection.is_connected():
            if not self.connect_database():
                return False
        
        try:
            cursor = self.db_connection.cursor()
            
            # 插入market_data表
            insert_sql = """
            INSERT INTO market_data (
                TradingDay, InstrumentID, ExchangeID, ExchangeInstID,
                LastPrice, PreSettlementPrice, PreClosePrice, PreOpenInterest,
                OpenPrice, HighestPrice, LowestPrice, Volume, Turnover,
                OpenInterest, ClosePrice, SettlementPrice, UpperLimitPrice,
                LowerLimitPrice, PreDelta, CurrDelta, UpdateTime, UpdateMillisec,
                BidPrice1, BidVolume1, AskPrice1, AskVolume1,
                BidPrice2, BidVolume2, AskPrice2, AskVolume2,
                BidPrice3, BidVolume3, AskPrice3, AskVolume3,
                BidPrice4, BidVolume4, AskPrice4, AskVolume4,
                BidPrice5, BidVolume5, AskPrice5, AskVolume5,
                AveragePrice, ActionDay
            ) VALUES (
                %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s,
                %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s,
                %s, %s, %s, %s, %s, %s, %s, %s, %s, %s
            )
            """
            
            # 从protobuf数据中提取字段值
            values = (
                market_data_proto.get('trading_day', ''),
                market_data_proto.get('instrument_id', ''),
                market_data_proto.get('exchange_id', ''),
                market_data_proto.get('exchange_inst_id', ''),
                market_data_proto.get('last_price', 0.0),
                market_data_proto.get('pre_settlement_price', 0.0),
                market_data_proto.get('pre_close_price', 0.0),
                market_data_proto.get('pre_open_interest', 0.0),
                market_data_proto.get('open_price', 0.0),
                market_data_proto.get('highest_price', 0.0),
                market_data_proto.get('lowest_price', 0.0),
                market_data_proto.get('volume', 0),
                market_data_proto.get('turnover', 0.0),
                market_data_proto.get('open_interest', 0.0),
                market_data_proto.get('close_price', 0.0),
                market_data_proto.get('settlement_price', 0.0),
                market_data_proto.get('upper_limit_price', 0.0),
                market_data_proto.get('lower_limit_price', 0.0),
                market_data_proto.get('pre_delta', 0.0),
                market_data_proto.get('curr_delta', 0.0),
                market_data_proto.get('update_time', ''),
                market_data_proto.get('update_millisec', 0),
                market_data_proto.get('bid_price1', 0.0),
                market_data_proto.get('bid_volume1', 0),
                market_data_proto.get('ask_price1', 0.0),
                market_data_proto.get('ask_volume1', 0),
                market_data_proto.get('bid_price2', 0.0),
                market_data_proto.get('bid_volume2', 0),
                market_data_proto.get('ask_price2', 0.0),
                market_data_proto.get('ask_volume2', 0),
                market_data_proto.get('bid_price3', 0.0),
                market_data_proto.get('bid_volume3', 0),
                market_data_proto.get('ask_price3', 0.0),
                market_data_proto.get('ask_volume3', 0),
                market_data_proto.get('bid_price4', 0.0),
                market_data_proto.get('bid_volume4', 0),
                market_data_proto.get('ask_price4', 0.0),
                market_data_proto.get('ask_volume4', 0),
                market_data_proto.get('bid_price5', 0.0),
                market_data_proto.get('bid_volume5', 0),
                market_data_proto.get('ask_price5', 0.0),
                market_data_proto.get('ask_volume5', 0),
                market_data_proto.get('average_price', 0.0),
                market_data_proto.get('action_day', '')
            )
            
            cursor.execute(insert_sql, values)
            cursor.close()
            self.stats['db_inserted'] += 1
            return True
            
        except Error as e:
            print(f"❌ 数据库插入失败: {e}")
            self.stats['errors'] += 1
            return False
    
    def connect_zmq(self):
        """连接ZMQ订阅者"""
        try:
            self.zmq_subscriber = self.zmq_context.socket(zmq.SUB)
            self.zmq_subscriber.connect(self.zmq_url)
            
            # 订阅所有行情数据消息
            self.zmq_subscriber.setsockopt(zmq.SUBSCRIBE, b"MARKET_DATA_PROTOBUF")
            
            # 设置接收超时
            self.zmq_subscriber.setsockopt(zmq.RCVTIMEO, 1000)  # 1秒超时
            
            print(f"✅ ZMQ连接成功: {self.zmq_url}")
            return True
            
        except Exception as e:
            print(f"❌ ZMQ连接失败: {e}")
            return False
    
    def test_redis(self):
        """测试Redis连接"""
        try:
            self.redis_client.ping()
            print("✅ Redis连接成功")
            return True
        except Exception as e:
            print(f"❌ Redis连接失败: {e}")
            return False
    
    def parse_market_data(self, data):
        """解析行情数据"""
        try:
            # 使用ProtobufParser解析二进制数据
            parser = ProtobufParser()
            market_data_proto = parser.parse_market_data(data)
            
            if market_data_proto:
                # 创建标准化的行情数据格式
                market_data = {
                    'timestamp': datetime.now().isoformat(),
                    'data_type': 'market_tick',
                    'raw_data': data.hex(),  # 将二进制数据转换为十六进制字符串
                    'size': len(data),
                    'trading_day': market_data_proto.get('trading_day', ''),
                    'instrument_id': market_data_proto.get('instrument_id', ''),
                    'exchange_id': market_data_proto.get('exchange_id', ''),
                    'exchange_inst_id': market_data_proto.get('exchange_inst_id', ''),
                    'last_price': market_data_proto.get('last_price', 0.0),
                    'pre_settlement_price': market_data_proto.get('pre_settlement_price', 0.0),
                    'pre_close_price': market_data_proto.get('pre_close_price', 0.0),
                    'pre_open_interest': market_data_proto.get('pre_open_interest', 0.0),
                    'open_price': market_data_proto.get('open_price', 0.0),
                    'highest_price': market_data_proto.get('highest_price', 0.0),
                    'lowest_price': market_data_proto.get('lowest_price', 0.0),
                    'volume': market_data_proto.get('volume', 0),
                    'turnover': market_data_proto.get('turnover', 0.0),
                    'open_interest': market_data_proto.get('open_interest', 0.0),
                    'close_price': market_data_proto.get('close_price', 0.0),
                    'settlement_price': market_data_proto.get('settlement_price', 0.0),
                    'upper_limit_price': market_data_proto.get('upper_limit_price', 0.0),
                    'lower_limit_price': market_data_proto.get('lower_limit_price', 0.0),
                    'pre_delta': market_data_proto.get('pre_delta', 0.0),
                    'curr_delta': market_data_proto.get('curr_delta', 0.0),
                    'update_time': market_data_proto.get('update_time', ''),
                    'update_millisec': market_data_proto.get('update_millisec', 0),
                    'bid_price1': market_data_proto.get('bid_price1', 0.0),
                    'bid_volume1': market_data_proto.get('bid_volume1', 0),
                    'ask_price1': market_data_proto.get('ask_price1', 0.0),
                    'ask_volume1': market_data_proto.get('ask_volume1', 0),
                    'bid_price2': market_data_proto.get('bid_price2', 0.0),
                    'bid_volume2': market_data_proto.get('bid_volume2', 0),
                    'ask_price2': market_data_proto.get('ask_price2', 0.0),
                    'ask_volume2': market_data_proto.get('ask_volume2', 0),
                    'bid_price3': market_data_proto.get('bid_price3', 0.0),
                    'bid_volume3': market_data_proto.get('bid_volume3', 0),
                    'ask_price3': market_data_proto.get('ask_price3', 0.0),
                    'ask_volume3': market_data_proto.get('ask_volume3', 0),
                    'bid_price4': market_data_proto.get('bid_price4', 0.0),
                    'bid_volume4': market_data_proto.get('bid_volume4', 0),
                    'ask_price4': market_data_proto.get('ask_price4', 0.0),
                    'ask_volume4': market_data_proto.get('ask_volume4', 0),
                    'bid_price5': market_data_proto.get('bid_price5', 0.0),
                    'bid_volume5': market_data_proto.get('bid_volume5', 0),
                    'ask_price5': market_data_proto.get('ask_price5', 0.0),
                    'ask_volume5': market_data_proto.get('ask_volume5', 0),
                    'average_price': market_data_proto.get('average_price', 0.0),
                    'action_day': market_data_proto.get('action_day', '')
                }
                return market_data
            else:
                print(f"❌ 解析Protobuf数据失败: {data.hex()}")
                return None
            
        except Exception as e:
            print(f"解析行情数据失败: {e}")
            return None
    
    def create_sample_data(self, instrument_id="TEST001"):
        """创建示例行情数据（用于测试）"""
        import random
        
        base_price = 3000.0
        change = random.uniform(-10, 10)
        
        return {
            'timestamp': datetime.now().isoformat(),
            'instrument_id': instrument_id,
            'last_price': round(base_price + change, 2),
            'bid_price': round(base_price + change - 0.5, 2),
            'ask_price': round(base_price + change + 0.5, 2),
            'bid_volume': random.randint(1, 100),
            'ask_volume': random.randint(1, 100),
            'volume': random.randint(1000, 10000),
            'turnover': random.randint(100000, 1000000),
            'open_price': round(base_price, 2),
            'high_price': round(base_price + 20, 2),
            'low_price': round(base_price - 20, 2),
            'change': round(change, 2),
            'change_percent': round(change / base_price * 100, 2),
            'data_type': 'market_tick'
        }
    
    def publish_to_redis(self, market_data):
        """发布数据到Redis"""
        try:
            json_data = json.dumps(market_data)
            result = self.redis_client.publish(self.redis_channel, json_data)
            self.stats['published'] += 1
            return result > 0
            
        except Exception as e:
            print(f"发布到Redis失败: {e}")
            self.stats['errors'] += 1
            return False
    
    def start_bridge(self):
        """启动桥接服务"""
        print("🚀 启动行情数据桥接服务...")
        
        # 测试连接
        if not self.test_redis():
            return False
            
        if not self.connect_zmq():
            return False
            
        if not self.connect_database():
            return False
        
        self.running = True
        self.stats['start_time'] = datetime.now()
        
        print(f"📡 开始监听行情数据: {self.zmq_url}")
        print(f"📤 发布到Redis频道: {self.redis_channel}")
        print(f"💾 写入数据库: {self.db_config['database']}")
        print("按 Ctrl+C 停止服务\n")
        
        # 启动统计线程
        stats_thread = threading.Thread(target=self.stats_reporter, daemon=True)
        stats_thread.start()
        
        # 主循环
        try:
            while self.running:
                try:
                    # 尝试接收ZMQ消息
                    try:
                        topic = self.zmq_subscriber.recv_string(zmq.NOBLOCK)
                        data = self.zmq_subscriber.recv(zmq.NOBLOCK)
                        
                        self.stats['received'] += 1
                        
                        # 解析数据
                        market_data = self.parse_market_data(data)
                        if market_data:
                            # 发布到Redis
                            if self.publish_to_redis(market_data):
                                print(f"📊 行情数据已转发: {len(data)} bytes")
                            
                            # 写入数据库
                            if self.insert_market_data_to_db(market_data):
                                print(f"💾 行情数据已写入数据库: {market_data['instrument_id']}")
                            else:
                                print(f"❌ 行情数据写入数据库失败: {market_data['instrument_id']}")
                        
                    except zmq.Again:
                        # 没有消息，继续循环
                        pass
                    except Exception as e:
                        print(f"接收ZMQ消息失败: {e}")
                        self.stats['errors'] += 1
                
                    # 如果长时间没有真实数据，发送测试数据
                    if self.stats['received'] == 0:
                        # 发送示例数据（仅用于演示）
                        sample_data = self.create_sample_data()
                        if self.publish_to_redis(sample_data):
                            print("📊 发送示例行情数据")
                        time.sleep(2)  # 每2秒发送一次示例数据
                    else:
                        time.sleep(0.01)  # 短暂休眠，避免CPU占用过高
                        
                except KeyboardInterrupt:
                    break
                except Exception as e:
                    print(f"桥接服务异常: {e}")
                    traceback.print_exc()
                    time.sleep(1)
                    
        except KeyboardInterrupt:
            pass
        finally:
            self.cleanup()
    
    def stats_reporter(self):
        """统计报告线程"""
        while self.running:
            time.sleep(10)  # 每10秒报告一次
            if self.running:
                uptime = datetime.now() - self.stats['start_time']
                print(f"📈 运行状态 - 已接收: {self.stats['received']}, "
                      f"已发布: {self.stats['published']}, "
                      f"已入库: {self.stats['db_inserted']}, "
                      f"错误: {self.stats['errors']}, "
                      f"运行时间: {uptime}")
    
    def cleanup(self):
        """清理资源"""
        print("\n🧹 清理资源...")
        self.running = False
        
        if self.zmq_subscriber:
            self.zmq_subscriber.close()
        
        if self.zmq_context:
            self.zmq_context.term()
        
        print("✅ 桥接服务已停止")

def main():
    """主函数"""
    bridge = MarketDataBridge()
    
    try:
        bridge.start_bridge()
    except Exception as e:
        print(f"桥接服务启动失败: {e}")
        traceback.print_exc()
    finally:
        bridge.cleanup()

if __name__ == "__main__":
    main() 