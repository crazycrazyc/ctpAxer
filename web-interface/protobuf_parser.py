#!/usr/bin/env python3
"""
Protobuf数据解析器
用于解析CTP行情数据的protobuf格式
"""

import struct
from market_data_pb2 import MarketDataMessage

class ProtobufParser:
    """protobuf数据解析器"""
    
    @staticmethod
    def parse_market_data(raw_data):
        """
        解析protobuf行情数据
        Args:
            raw_data: bytes，原始protobuf数据
        Returns:
            dict: 解析后的行情数据字典
        """
        try:
            # 创建protobuf消息对象
            market_msg = MarketDataMessage()
            
            # 解析protobuf数据
            market_msg.ParseFromString(raw_data)
            
            # 转换为字典格式
            market_data = {
                'trading_day': market_msg.trading_day,
                'instrument_id': market_msg.instrument_id,
                'exchange_id': market_msg.exchange_id,
                'exchange_inst_id': market_msg.exchange_inst_id,
                'last_price': market_msg.last_price,
                'pre_settlement_price': market_msg.pre_settlement_price,
                'pre_close_price': market_msg.pre_close_price,
                'pre_open_interest': market_msg.pre_open_interest,
                'open_price': market_msg.open_price,
                'highest_price': market_msg.highest_price,
                'lowest_price': market_msg.lowest_price,
                'volume': market_msg.volume,
                'turnover': market_msg.turnover,
                'open_interest': market_msg.open_interest,
                'close_price': market_msg.close_price,
                'settlement_price': market_msg.settlement_price,
                'upper_limit_price': market_msg.upper_limit_price,
                'lower_limit_price': market_msg.lower_limit_price,
                'pre_delta': market_msg.pre_delta,
                'curr_delta': market_msg.curr_delta,
                'update_time': market_msg.update_time,
                'update_millisec': market_msg.update_millisec,
                'bid_price1': market_msg.bid_price1,
                'bid_volume1': market_msg.bid_volume1,
                'ask_price1': market_msg.ask_price1,
                'ask_volume1': market_msg.ask_volume1,
                'bid_price2': market_msg.bid_price2,
                'bid_volume2': market_msg.bid_volume2,
                'ask_price2': market_msg.ask_price2,
                'ask_volume2': market_msg.ask_volume2,
                'bid_price3': market_msg.bid_price3,
                'bid_volume3': market_msg.bid_volume3,
                'ask_price3': market_msg.ask_price3,
                'ask_volume3': market_msg.ask_volume3,
                'bid_price4': market_msg.bid_price4,
                'bid_volume4': market_msg.bid_volume4,
                'ask_price4': market_msg.ask_price4,
                'ask_volume4': market_msg.ask_volume4,
                'bid_price5': market_msg.bid_price5,
                'bid_volume5': market_msg.bid_volume5,
                'ask_price5': market_msg.ask_price5,
                'ask_volume5': market_msg.ask_volume5,
                'average_price': market_msg.average_price,
                'action_day': market_msg.action_day,
                'local_timestamp': market_msg.local_timestamp,
                'timestamp': market_msg.timestamp
            }
            
            return market_data
            
        except Exception as e:
            print(f"protobuf解析失败: {e}")
            return None
    
    @staticmethod
    def test_parser():
        """测试解析器功能"""
        print("🧪 测试protobuf解析器...")
        
        # 创建测试数据
        test_msg = MarketDataMessage()
        test_msg.trading_day = "20250806"
        test_msg.instrument_id = "cu2508"
        test_msg.exchange_id = "SHFE"
        test_msg.last_price = 78260.0
        test_msg.volume = 12345
        test_msg.update_time = "14:05:30"
        
        # 序列化
        raw_data = test_msg.SerializeToString()
        print(f"   序列化数据长度: {len(raw_data)} bytes")
        
        # 解析
        parsed_data = ProtobufParser.parse_market_data(raw_data)
        if parsed_data:
            print(f"   ✅ 解析成功: {parsed_data['instrument_id']} - {parsed_data['last_price']}")
            return True
        else:
            print("   ❌ 解析失败")
            return False

if __name__ == "__main__":
    ProtobufParser.test_parser()
