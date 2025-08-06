#!/usr/bin/env python3
"""
支持WebSocket的Flask应用
集成实时行情分发功能
"""

from flask import Flask, render_template, request, jsonify, send_file, make_response
from flask_socketio import SocketIO, emit, join_room, leave_room
import mysql.connector
from mysql.connector import Error
import pandas as pd
import io
import csv
from datetime import datetime
import os
import redis
import json
import threading
import traceback
import time
from contextlib import contextmanager
import queue  # 添加队列支持

# 复制原有的数据库配置和管理器
DB_CONFIG = {
    'host': '172.16.30.97',
    'port': 13306,
    'user': 'elwriter',
    'password': 'elwriter123',
    'database': 'cta_trade',
    'charset': 'utf8mb4'
}

import threading
from contextlib import contextmanager

class DatabaseManager:
    def __init__(self):
        self.max_retries = 3
        self.retry_delay = 0.5
        self._local = threading.local()
        self._lock = threading.Lock()
    
    @contextmanager
    def get_db_connection(self):
        connection = None
        try:
            connection = mysql.connector.connect(
                **DB_CONFIG,
                autocommit=True,
                use_pure=True,
                connection_timeout=5,
                sql_mode='TRADITIONAL'
            )
            yield connection
        except Error as e:
            print(f"数据库连接创建失败: {e}")
            raise
        finally:
            if connection and connection.is_connected():
                try:
                    connection.close()
                except:
                    pass
    
    def execute_query(self, query, params=None):
        last_error = None
        for attempt in range(self.max_retries):
            try:
                with self.get_db_connection() as connection:
                    cursor = connection.cursor(dictionary=True)
                    cursor.execute(query, params)
                    result = cursor.fetchall()
                    cursor.close()
                    return result
            except Error as e:
                last_error = e
                if attempt < self.max_retries - 1:
                    time.sleep(self.retry_delay * (attempt + 1))
                    continue
            except Exception as e:
                last_error = e
                if attempt < self.max_retries - 1:
                    time.sleep(self.retry_delay)
                    continue
        print(f"数据库查询最终失败: {query}, 最后错误: {last_error}")
        return None
    
    def connect(self):
        return True
    
    def disconnect(self):
        pass

# 初始化
app = Flask(__name__)
app.config['SECRET_KEY'] = 'your-secret-key-here'

# 初始化SocketIO
socketio = SocketIO(app, cors_allowed_origins="*", async_mode='eventlet')

# 数据库管理器
db_manager = DatabaseManager()

# Redis连接
redis_client = redis.Redis(host='localhost', port=6379, db=0, decode_responses=True)

# 全局变量
market_subscribers = set()  # 存储订阅行情的客户端
market_thread = None
market_running = False

# 添加调试变量
client_debug_info = {}  # 存储客户端调试信息
connection_count = 0    # 连接计数器

# 最新行情数据存储
latest_market_data = {}  # 存储最新的行情数据，按合约ID索引
send_batch_offset = 0    # 发送批次偏移量，用于轮换发送

# ========== 原有的路由（原app.py功能已整合） ==========

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/accounts')
def accounts():
    return render_template('accounts.html')

@app.route('/positions')  
def positions():
    return render_template('positions.html')

@app.route('/realtime')
def realtime():
    """实时行情页面"""
    return render_template('realtime.html')

@app.route('/test_simple')
def test_simple():
    return render_template('test_simple.html')

# 原有的API路由（简化版本）
# ========== 合约信息查询API ==========
@app.route('/api/instruments')
def get_instruments():
    page = request.args.get('page', 1, type=int)
    per_page = request.args.get('per_page', 20, type=int)
    search = request.args.get('search', '', type=str)
    
    where_conditions = []
    params = []
    
    if search:
        where_conditions.append("(InstrumentID LIKE %s OR InstrumentName LIKE %s OR ExchangeID LIKE %s)")
        search_param = f"%{search}%"
        params.extend([search_param, search_param, search_param])
    
    where_clause = ""
    if where_conditions:
        where_clause = "WHERE " + " AND ".join(where_conditions)
    
    count_query = f"SELECT COUNT(*) as total FROM test_update_instrument {where_clause}"
    count_result = db_manager.execute_query(count_query, params)
    total = count_result[0]['total'] if count_result else 0
    
    offset = (page - 1) * per_page
    data_query = f"""
        SELECT InstrumentID, InstrumentName, ExchangeID, ProductID, VolumeMultiple, 
               PriceTick, CreateDate, ExpireDate, IsTrading
        FROM test_update_instrument 
        {where_clause}
        ORDER BY InstrumentID 
        LIMIT %s OFFSET %s
    """
    
    params.extend([per_page, offset])
    instruments = db_manager.execute_query(data_query, params)
    
    return jsonify({
        'data': instruments or [],
        'total': total,
        'page': page,
        'per_page': per_page,
        'pages': (total + per_page - 1) // per_page
    })

@app.route('/api/stats')
def get_stats():
    try:
        stats_queries = {
            'total_instruments': "SELECT COUNT(*) as count FROM test_update_instrument",
            'active_instruments': "SELECT COUNT(*) as count FROM test_update_instrument WHERE IsTrading = 1",
            'exchanges': "SELECT ExchangeID, COUNT(*) as count FROM test_update_instrument GROUP BY ExchangeID ORDER BY count DESC",
            'product_classes': "SELECT ProductClass, COUNT(*) as count FROM test_update_instrument GROUP BY ProductClass ORDER BY count DESC"
        }
        
        stats = {}
        for key, query in stats_queries.items():
            result = db_manager.execute_query(query)
            if result is not None:
                stats[key] = result
            else:
                stats[key] = []
        
        return jsonify({
            'status': 'success',
            'data': stats
        })
    except Exception as e:
        return jsonify({
            'status': 'error',
            'message': '统计信息获取失败',
            'error': str(e)
        }), 500

# ========== 工具和调试API ==========

@app.route('/api/instrument/<instrument_id>')
def get_instrument_detail(instrument_id):
    """获取特定合约详细信息"""
    query = "SELECT * FROM test_update_instrument WHERE InstrumentID = %s"
    result = db_manager.execute_query(query, [instrument_id])
    
    if result:
        return jsonify({'success': True, 'data': result[0]})
    else:
        return jsonify({'success': False, 'message': '合约不存在'})

@app.route('/api/export/csv')
def export_csv():
    """导出CSV文件"""
    search = request.args.get('search', '', type=str)
    instrument_ids = request.args.get('instruments', '', type=str)
    
    # 构建查询条件
    where_clause = ""
    params = []
    
    if instrument_ids:
        # 指定合约代码
        ids = [id.strip() for id in instrument_ids.split(',') if id.strip()]
        if ids:
            placeholders = ','.join(['%s'] * len(ids))
            where_clause = f"WHERE InstrumentID IN ({placeholders})"
            params = ids
    elif search:
        # 搜索条件
        where_clause = "WHERE InstrumentID LIKE %s OR InstrumentName LIKE %s OR ExchangeID LIKE %s"
        search_param = f"%{search}%"
        params = [search_param, search_param, search_param]
    
    # 查询数据
    query = f"SELECT * FROM test_update_instrument {where_clause} ORDER BY InstrumentID"
    instruments = db_manager.execute_query(query, params)
    
    if not instruments:
        return jsonify({'success': False, 'message': '没有找到匹配的数据'})
    
    # 创建CSV文件
    output = io.StringIO()
    writer = csv.writer(output, quoting=csv.QUOTE_ALL)
    
    # 写入表头
    if instruments:
        headers = list(instruments[0].keys())
        writer.writerow(headers)
        
        # 写入数据
        for instrument in instruments:
            writer.writerow([str(instrument[key]) if instrument[key] is not None else '' for key in headers])
    
    # 创建响应
    output.seek(0)
    
    # 生成文件名
    timestamp = datetime.now().strftime('%Y%m%d_%H%M%S')
    filename = f'instruments_{timestamp}.csv'
    
    # 创建响应对象
    response = make_response(output.getvalue())
    response.headers['Content-Type'] = 'text/csv; charset=utf-8-sig'  # 添加BOM以支持中文
    response.headers['Content-Disposition'] = f'attachment; filename={filename}'
    
    return response

@app.route('/api/health/db')
def check_db_health():
    """数据库连接健康检查"""
    try:
        # 测试简单查询
        result = db_manager.execute_query("SELECT 1 as test")
        
        if result is not None:
            return jsonify({
                'status': 'healthy',
                'message': '数据库连接正常',
                'timestamp': datetime.now().isoformat()
            })
        else:
            return jsonify({
                'status': 'unhealthy',
                'message': '数据库查询失败',
                'timestamp': datetime.now().isoformat()
            }), 503
            
    except Exception as e:
        return jsonify({
            'status': 'error',
            'message': f'数据库连接异常: {str(e)}',
            'timestamp': datetime.now().isoformat()
        }), 503

@app.route('/api/debug/tables')  
def check_tables():
    """检查数据库表是否存在"""
    try:
        tables_to_check = [
            'test_update_instrument',
            'instruments', 
            'investor_positions',
            'trading_account'
        ]
        
        table_status = {}
        for table in tables_to_check:
            try:
                result = db_manager.execute_query(f"SELECT COUNT(*) as count FROM {table} LIMIT 1")
                if result is not None:
                    table_status[table] = {
                        'exists': True,
                        'count': result[0]['count'] if result else 0
                    }
                else:
                    table_status[table] = {
                        'exists': False,
                        'error': '查询失败'
                    }
            except Exception as e:
                table_status[table] = {
                    'exists': False,
                    'error': str(e)
                }
        
        return jsonify({
            'status': 'success',
            'tables': table_status,
            'timestamp': datetime.now().isoformat()
        })
        
    except Exception as e:
        return jsonify({
            'status': 'error',
            'message': str(e),
            'timestamp': datetime.now().isoformat()
        }), 500

# ========== 持仓和资金查询API ==========

@app.route('/api/accounts')
def get_trading_accounts():
    """获取资金账户列表API"""
    page = request.args.get('page', 1, type=int)
    per_page = request.args.get('per_page', 20, type=int)
    search = request.args.get('search', '', type=str)
    
    # 构建查询条件
    where_conditions = []
    params = []
    
    if search:
        where_conditions.append("(AccountID LIKE %s OR BrokerID LIKE %s)")
        search_param = f"%{search}%"
        params.extend([search_param, search_param])
    
    where_clause = ""
    if where_conditions:
        where_clause = "WHERE " + " AND ".join(where_conditions)
    
    # 获取总数
    count_query = f"SELECT COUNT(*) as total FROM trading_account {where_clause}"
    count_result = db_manager.execute_query(count_query, params)
    total = count_result[0]['total'] if count_result else 0
    
    # 获取分页数据
    offset = (page - 1) * per_page
    data_query = f"""
        SELECT BrokerID, AccountID, Balance, Available, CurrMargin, PositionProfit,
               CloseProfit, FrozenMargin, FrozenCash, TradingDay, PreBalance,
               Deposit, Withdraw, WithdrawQuota, Commission
        FROM trading_account 
        {where_clause}
        ORDER BY TradingDay DESC, AccountID 
        LIMIT %s OFFSET %s
    """
    
    query_params = params + [per_page, offset]
    accounts = db_manager.execute_query(data_query, query_params)
    
    return jsonify({
        'data': accounts or [],
        'total': total,
        'page': page,
        'per_page': per_page,
        'pages': (total + per_page - 1) // per_page
    })

@app.route('/api/account/<account_id>')
def get_account_detail(account_id):
    """获取特定账户详细信息"""
    query = """
        SELECT * FROM trading_account 
        WHERE AccountID = %s 
        ORDER BY TradingDay DESC 
        LIMIT 1
    """
    result = db_manager.execute_query(query, [account_id])
    
    if result:
        return jsonify({'success': True, 'data': result[0]})
    else:
        return jsonify({'success': False, 'message': '账户记录不存在'})

@app.route('/api/account/history/<account_id>')
def get_account_history(account_id):
    """获取账户历史记录"""
    query = """
        SELECT * FROM trading_account 
        WHERE AccountID = %s 
        ORDER BY TradingDay DESC 
        LIMIT 30
    """
    result = db_manager.execute_query(query, [account_id])
    
    if result:
        return jsonify({'success': True, 'data': result})
    else:
        return jsonify({'success': False, 'message': '没有找到历史记录'})

@app.route('/api/accounts/summary')
def get_accounts_summary():
    """获取账户汇总信息"""
    query = """
        SELECT 
            COUNT(DISTINCT AccountID) as total_accounts,
            SUM(Balance) as total_balance,
            SUM(Available) as total_available,
            SUM(CurrMargin) as total_margin,
            SUM(PositionProfit) as total_position_profit,
            SUM(CloseProfit) as total_close_profit
        FROM trading_account 
        WHERE TradingDay = (SELECT MAX(TradingDay) FROM trading_account)
    """
    result = db_manager.execute_query(query)
    
    if result:
        return jsonify({'success': True, 'data': result[0]})
    else:
        return jsonify({'success': False, 'message': '无法获取汇总信息'})

@app.route('/api/positions')
def get_positions():
    """获取持仓列表API"""
    page = request.args.get('page', 1, type=int)
    per_page = request.args.get('per_page', 20, type=int)
    search = request.args.get('search', '', type=str)
    investor_id = request.args.get('investor_id', '', type=str)
    broker_id = request.args.get('broker_id', '', type=str)
    
    # 构建查询条件
    where_conditions = []
    params = []
    
    if investor_id:
        where_conditions.append("InvestorID LIKE %s")
        params.append(f"%{investor_id}%")
    
    if broker_id:
        where_conditions.append("BrokerID LIKE %s")
        params.append(f"%{broker_id}%")
    
    if search:
        where_conditions.append("(InstrumentID LIKE %s OR InvestorID LIKE %s OR BrokerID LIKE %s)")
        search_param = f"%{search}%"
        params.extend([search_param, search_param, search_param])
    
    where_clause = ""
    if where_conditions:
        where_clause = "WHERE " + " AND ".join(where_conditions)
    
    # 获取总数
    count_query = f"SELECT COUNT(*) as total FROM investor_positions {where_clause}"
    count_result = db_manager.execute_query(count_query, params)
    total = count_result[0]['total'] if count_result else 0
    
    # 获取分页数据
    offset = (page - 1) * per_page
    data_query = f"""
        SELECT InstrumentID, BrokerID, InvestorID, PosiDirection, HedgeFlag,
               YdPosition, Position, PositionProfit, UseMargin, PositionCost,
               OpenCost, Commission, CloseProfit, TradingDay
        FROM investor_positions 
        {where_clause}
        ORDER BY InvestorID, InstrumentID 
        LIMIT %s OFFSET %s
    """
    
    query_params = params + [per_page, offset]
    positions = db_manager.execute_query(data_query, query_params)
    
    return jsonify({
        'data': positions or [],
        'total': total,
        'page': page,
        'per_page': per_page,
        'pages': (total + per_page - 1) // per_page
    })

@app.route('/api/position/<position_id>')
def get_position_detail(position_id):
    """获取特定持仓详细信息"""
    query = "SELECT * FROM investor_positions WHERE id = %s"
    result = db_manager.execute_query(query, [position_id])
    
    if result:
        return jsonify({'success': True, 'data': result[0]})
    else:
        return jsonify({'success': False, 'message': '持仓记录不存在'})

@app.route('/api/investors')
def get_investors():
    """获取投资者账户列表"""
    query = "SELECT DISTINCT InvestorID, BrokerID FROM investor_positions ORDER BY InvestorID"
    result = db_manager.execute_query(query)
    
    if result:
        return jsonify({'success': True, 'data': result})
    else:
        return jsonify({'success': False, 'message': '没有找到投资者信息'})

@app.route('/api/export/positions/csv')
def export_positions_csv():
    """导出持仓数据为CSV"""
    search = request.args.get('search', '', type=str)
    investor_id = request.args.get('investor_id', '', type=str)
    broker_id = request.args.get('broker_id', '', type=str)
    
    # 构建查询条件
    where_conditions = []
    params = []
    
    if investor_id:
        where_conditions.append("InvestorID LIKE %s")
        params.append(f"%{investor_id}%")
    
    if broker_id:
        where_conditions.append("BrokerID LIKE %s")
        params.append(f"%{broker_id}%")
    
    if search:
        where_conditions.append("(InstrumentID LIKE %s OR InvestorID LIKE %s OR BrokerID LIKE %s)")
        search_param = f"%{search}%"
        params.extend([search_param, search_param, search_param])
    
    where_clause = ""
    if where_conditions:
        where_clause = "WHERE " + " AND ".join(where_conditions)
    
    # 查询数据
    query = f"SELECT * FROM investor_positions {where_clause} ORDER BY InvestorID, InstrumentID"
    positions = db_manager.execute_query(query, params)
    
    if not positions:
        return jsonify({'success': False, 'message': '没有找到匹配的数据'})
    
    # 创建CSV文件
    output = io.StringIO()
    writer = csv.writer(output)
    
    # 写入表头
    if positions:
        headers = list(positions[0].keys())
        writer.writerow(headers)
        
        # 写入数据
        for position in positions:
            writer.writerow([position[key] for key in headers])
    
    # 创建响应
    output.seek(0)
    
    # 生成文件名
    timestamp = datetime.now().strftime('%Y%m%d_%H%M%S')
    filename = f'positions_{timestamp}.csv'
    
    # 创建响应对象
    response = make_response(output.getvalue())
    response.headers['Content-Type'] = 'text/csv; charset=utf-8-sig'
    response.headers['Content-Disposition'] = f'attachment; filename={filename}'
    
    return response

@app.route('/api/position_stats')
def get_position_stats():
    """获取持仓统计信息"""
    query = """
        SELECT 
            COUNT(*) as total_positions,
            COUNT(DISTINCT InvestorID) as total_investors,
            COUNT(DISTINCT InstrumentID) as total_instruments,
            SUM(Position) as total_position,
            SUM(PositionProfit) as total_position_profit,
            SUM(UseMargin) as total_margin
        FROM investor_positions
        WHERE Position > 0
    """
    result = db_manager.execute_query(query)
    
    if result:
        return jsonify({'success': True, 'data': result[0]})
    else:
        return jsonify({'success': False, 'message': '无法获取统计信息'})

@app.route('/api/export/accounts')
def export_accounts_csv():
    """导出资金账户数据为CSV"""
    search = request.args.get('search', '', type=str)
    
    # 构建查询条件
    where_conditions = []
    params = []
    
    if search:
        where_conditions.append("(AccountID LIKE %s OR BrokerID LIKE %s)")
        search_param = f"%{search}%"
        params.extend([search_param, search_param])
    
    where_clause = ""
    if where_conditions:
        where_clause = "WHERE " + " AND ".join(where_conditions)
    
    # 查询数据
    query = f"SELECT * FROM trading_account {where_clause} ORDER BY TradingDay DESC, AccountID"
    accounts = db_manager.execute_query(query, params)
    
    if not accounts:
        return jsonify({'success': False, 'message': '没有找到匹配的数据'})
    
    # 创建CSV文件
    output = io.StringIO()
    writer = csv.writer(output)
    
    # 写入表头
    if accounts:
        headers = list(accounts[0].keys())
        writer.writerow(headers)
        
        # 写入数据
        for account in accounts:
            writer.writerow([account[key] for key in headers])
    
    # 创建响应
    output.seek(0)
    
    # 生成文件名
    timestamp = datetime.now().strftime('%Y%m%d_%H%M%S')
    filename = f'accounts_{timestamp}.csv'
    
    # 创建响应对象
    response = make_response(output.getvalue())
    response.headers['Content-Type'] = 'text/csv; charset=utf-8-sig'
    response.headers['Content-Disposition'] = f'attachment; filename={filename}'
    
    return response

# ========== WebSocket相关功能 ==========

def redis_listener():
    """Redis订阅监听器"""
    global market_running
    
    print("🎧 启动Redis行情监听器...")
    
    try:
        pubsub = redis_client.pubsub()
        pubsub.subscribe('market_data')
        print("📡 已订阅Redis频道'market_data'")
        
        message_count = 0
        last_status_time = time.time()
        
        for message in pubsub.listen():
            if not market_running:
                print("🛑 收到停止信号，退出Redis监听器")
                break
                
            if message['type'] == 'subscribe':
                print(f"✅ 成功订阅Redis频道: {message['channel']}")
                
            elif message['type'] == 'message':
                try:
                    message_count += 1
                    current_time = time.time()
                    
                    # 解析行情数据
                    market_data = json.loads(message['data'])
                    
                    # 每30秒或每100条消息显示一次详细状态
                    if (current_time - last_status_time > 30) or (message_count % 100 == 1):
                        print(f"📊 状态报告 [消息{message_count}]:")
                        print(f"   当前订阅客户端: {list(market_subscribers)} (共{len(market_subscribers)}个)")
                        print(f"   market_running: {market_running}")
                        print(f"   连接总数: {connection_count}")
                        print(f"   客户端调试信息: {client_debug_info}")
                        last_status_time = current_time
                    
                    # 更新最新行情数据
                    latest_market_data[market_data['instrument_id']] = market_data
                    
                    if message_count % 50 == 1:
                        print(f"📨 [{message_count}] 收到Redis消息: {market_data.get('instrument_id', 'UNKNOWN')} - {market_data.get('last_price', 'N/A')}")
                        print(f"📦 当前存储合约数: {len(latest_market_data)}")
                    
                except Exception as e:
                    print(f"❌ 处理Redis消息失败: {e}")
                    print(f"   消息内容: {message}")
                    
    except Exception as e:
        print(f"❌ Redis监听器异常: {e}")
        traceback.print_exc()
    finally:
        print("🎧 Redis监听器已停止")

@socketio.on('connect')
def handle_connect():
    """客户端连接"""
    global connection_count
    connection_count += 1
    print(f"✅ 客户端连接: {request.sid} (连接计数: {connection_count})")
    emit('connected', {'status': 'success', 'message': '连接成功'})

@socketio.on('disconnect')
def handle_disconnect():
    """客户端断开连接"""
    global connection_count
    connection_count -= 1
    
    # 检查客户端是否在订阅列表中
    was_subscribed = request.sid in market_subscribers
    
    print(f"❌ 客户端断开: {request.sid} (连接计数: {connection_count})")
    print(f"   断开前是否订阅: {was_subscribed}")
    print(f"   断开前订阅列表: {list(market_subscribers)} (共{len(market_subscribers)}个)")
    
    market_subscribers.discard(request.sid)
    
    print(f"   断开后订阅列表: {list(market_subscribers)} (共{len(market_subscribers)}个)")
    
    # 清理调试信息
    if request.sid in client_debug_info:
        del client_debug_info[request.sid]

@socketio.on('subscribe_market')
def handle_subscribe_market():
    """订阅行情数据"""
    global market_thread, market_running
    
    print(f"📡 客户端 {request.sid} 请求订阅行情数据")
    print(f"   订阅前market_subscribers: {list(market_subscribers)} (共{len(market_subscribers)}个)")
    
    # 记录客户端调试信息
    client_debug_info[request.sid] = {
        'subscribe_time': time.time(),
        'connection_count': connection_count
    }
    
    # 直接添加到订阅列表（不使用room机制）
    market_subscribers.add(request.sid)
    print(f"   已添加到market_subscribers")
    print(f"   订阅后market_subscribers: {list(market_subscribers)} (共{len(market_subscribers)}个)")
    
    # 启动Redis监听线程（如果还未启动）
    if not market_running:
        print("🚀 启动Redis监听线程...")
        market_running = True
        market_thread = threading.Thread(target=redis_listener, daemon=True)
        market_thread.start()
        print(f"✅ Redis监听线程已启动，线程ID: {market_thread.ident}")
        print(f"   线程是否活跃: {market_thread.is_alive()}")
    else:
        print(f"ℹ️  Redis监听线程已在运行，线程ID: {market_thread.ident if market_thread else 'None'}")
        if market_thread:
            print(f"   线程是否活跃: {market_thread.is_alive()}")
    
    emit('subscription_success', {
        'status': 'success', 
        'message': '行情订阅成功',
        'subscribers': len(market_subscribers)
    })
    print(f"✅ 已发送订阅成功响应给客户端 {request.sid}")
    print(f"   当前全局状态: market_running={market_running}, 订阅客户端数={len(market_subscribers)}")
    
    # 发送测试消息验证连接
    emit('test_message', {
        'message': '这是一条测试消息',
        'timestamp': time.time(),
        'client_id': request.sid
    })
    print(f"🧪 已发送测试消息给客户端 {request.sid}")
    
    # 启动定时发送行情数据
    if len(market_subscribers) == 1:  # 第一个客户端订阅时启动
        print("🕒 启动定时发送任务...")
        start_market_data_sender()


@socketio.on('unsubscribe_market')
def handle_unsubscribe_market():
    """取消订阅行情数据"""
    print(f"🛑 客户端 {request.sid} 请求取消订阅行情数据")
    print(f"   取消前market_subscribers: {list(market_subscribers)} (共{len(market_subscribers)}个)")
    
    # 从订阅列表中移除
    market_subscribers.discard(request.sid)
    print(f"   已从market_subscribers中移除")
    print(f"   取消后market_subscribers: {list(market_subscribers)} (共{len(market_subscribers)}个)")
    
    # 清理客户端调试信息
    if request.sid in client_debug_info:
        del client_debug_info[request.sid]
        print(f"   已清理客户端调试信息")
    
    # 发送取消订阅成功响应
    emit('unsubscription_success', {
        'status': 'success', 
        'message': '已取消行情订阅',
        'subscribers': len(market_subscribers)
    })
    print(f"✅ 已发送取消订阅成功响应给客户端 {request.sid}")
    print(f"   当前订阅客户端数: {len(market_subscribers)}")


def start_market_data_sender():
    """启动定时发送行情数据的任务"""
    @socketio.on('start_background_task')
    def handle_start_background_task():
        """启动后台任务"""
        print("🔄 启动后台发送任务")
        
    def background_task():
        """后台发送任务"""
        print("🚀 后台任务启动...")
        
        global send_batch_offset
        
        while market_running and market_subscribers:
            try:
                # 发送最新的行情数据
                sent_count = 0
                
                if latest_market_data and market_subscribers:
                    # 获取所有合约数据
                    all_items = list(latest_market_data.items())
                    total_contracts = len(all_items)
                    
                    if total_contracts > 0:
                        # 每次发送30个合约，采用轮换策略
                        batch_size = 30
                        
                        # 计算当前批次的起始和结束位置
                        start_idx = send_batch_offset % total_contracts
                        end_idx = min(start_idx + batch_size, total_contracts)
                        
                        # 如果不够30个，从头开始补充
                        if end_idx - start_idx < batch_size and total_contracts > batch_size:
                            # 取当前批次 + 从头开始的部分
                            current_batch = all_items[start_idx:end_idx]
                            remaining_needed = batch_size - len(current_batch)
                            current_batch.extend(all_items[:remaining_needed])
                            send_items = current_batch
                        else:
                            send_items = all_items[start_idx:end_idx]
                        
                        # 更新偏移量，下次从不同位置开始
                        send_batch_offset = (send_batch_offset + batch_size) % total_contracts
                        
                        for instrument_id, market_data in send_items:
                            for client_id in list(market_subscribers):
                                try:
                                    socketio.emit('market_data', market_data, room=client_id)
                                    sent_count += 1
                                except Exception as e:
                                    print(f"❌ 后台发送失败: {e}")
                                    market_subscribers.discard(client_id)
                        
                        if sent_count > 0:
                            print(f"🔄 后台发送: {sent_count} 条数据给 {len(market_subscribers)} 个客户端")
                            print(f"   本批次合约: {len(send_items)}个 (第{start_idx+1}-{start_idx+len(send_items)}个)")
                            print(f"   总合约数: {total_contracts}个, 下次从第{(send_batch_offset % total_contracts)+1}个开始")
                
                # 等待2秒再发送下一批
                socketio.sleep(2)
                
            except Exception as e:
                print(f"❌ 后台任务异常: {e}")
                break
                
        print("🔄 后台任务结束")
    
    # 启动后台任务
    socketio.start_background_task(background_task)
    print("✅ SocketIO后台发送任务已启动")


@socketio.on('get_market_status')
def handle_get_market_status():
    """获取行情服务状态"""
    try:
        # 获取Redis连接状态
        redis_status = redis_client.ping()
        
        emit('market_status', {
            'redis_connected': redis_status,
            'subscribers': len(market_subscribers),
            'thread_running': market_running,
            'thread_alive': market_thread.is_alive() if market_thread else False
        })
    except Exception as e:
        emit('market_status', {
            'error': str(e),
            'subscribers': len(market_subscribers)
        })

@socketio.on('trigger_manual_send')
def handle_trigger_manual_send():
    """手动触发发送测试"""
    print(f"🔧 客户端 {request.sid} 请求手动发送测试")
    
    # 发送手动触发响应
    emit('manual_trigger', {
        'message': '手动触发测试',
        'timestamp': time.time(),
        'client_id': request.sid,
        'subscribers_count': len(market_subscribers)
    })
    
    # 如果有存储的行情数据，发送一条
    if latest_market_data and market_subscribers:
        # 取第一条行情数据
        sample_data = next(iter(latest_market_data.values()))
        emit('market_data', sample_data, room=request.sid)
        print(f"🔧 手动发送了一条行情数据: {sample_data.get('instrument_id', 'UNKNOWN')}")
    else:
        print(f"🔧 没有可用的行情数据或订阅者")
    
    print(f"🔧 手动触发完成")

# ========== 健康检查API ==========

@app.route('/api/realtime/status')
def realtime_status():
    """实时行情服务状态"""
    try:
        redis_status = redis_client.ping()
        
        return jsonify({
            'status': 'success',
            'redis_connected': redis_status,
            'active_subscribers': len(market_subscribers),
            'market_listener_running': market_running,
            'timestamp': datetime.now().isoformat()
        })
    except Exception as e:
        return jsonify({
            'status': 'error',
            'error': str(e),
            'timestamp': datetime.now().isoformat()
        }), 500

@app.route('/api/realtime/test')
def test_market_data():
    """发送测试行情数据"""
    try:
        import random
        
        test_data = {
            'timestamp': datetime.now().isoformat(),
            'instrument_id': 'TEST001',
            'last_price': round(3000 + random.uniform(-10, 10), 2),
            'volume': random.randint(1000, 10000),
            'data_type': 'test_tick'
        }
        
        # 发布到Redis
        result = redis_client.publish('market_data', json.dumps(test_data))
        
        return jsonify({
            'status': 'success',
            'message': '测试数据已发送',
            'data': test_data,
            'subscribers_notified': result
        })
        
    except Exception as e:
        return jsonify({
            'status': 'error',
            'error': str(e)
        }), 500

def cleanup():
    """清理资源"""
    global market_running
    market_running = False
    print("🧹 WebSocket应用清理完成")

if __name__ == '__main__':
    print("🚀 启动支持WebSocket的CTP查询系统...")
    print("请确保数据库和Redis连接正常...")
    
    # 测试数据库连接
    if db_manager.connect():
        print("✓ 数据库连接成功")
    else:
        print("✗ 数据库连接失败，请检查配置")
    
    # 测试Redis连接
    try:
        redis_client.ping()
        print("✓ Redis连接成功")
    except:
        print("✗ Redis连接失败，请检查Redis服务")
    
    print("启动WebSocket服务器...")
    print("访问地址: http://localhost:5502")
    print("实时行情: http://localhost:5502/realtime")
    print("按 Ctrl+C 停止服务")
    
    try:
        socketio.run(app, debug=False, host='0.0.0.0', port=5502)
    except KeyboardInterrupt:
        cleanup()
    except Exception as e:
        print(f"应用启动失败: {e}")
        cleanup() 