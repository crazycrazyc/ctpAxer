#!/bin/bash

# CTP实时行情系统启动脚本
# 管理WebSocket服务器和行情桥接服务

cd "$(dirname "$0")"

echo "========== CTP实时行情系统管理脚本 =========="
echo "当前时间: $(date '+%Y-%m-%d %H:%M:%S')"
echo "工作目录: $(pwd)"
echo ""

# 检查服务状态
check_service_status() {
    local service_name=$1
    local process_pattern=$2
    
    if pgrep -f "$process_pattern" > /dev/null; then
        echo "✅ $service_name: 运行中"
        return 0
    else
        echo "❌ $service_name: 未运行"
        return 1
    fi
}

# 显示服务状态
show_status() {
    echo "🔍 系统服务状态:"
    check_service_status "WebSocket服务器" "python3 websocket_app.py"
    check_service_status "行情桥接服务" "python3 market_bridge.py"
    
    echo ""
    echo "🔗 端口占用情况:"
    netstat -tulpn | grep -E ":5502|:9999|:6379" | head -10
    
    echo ""
    echo "📊 Redis状态:"
    if redis-cli ping > /dev/null 2>&1; then
        echo "✅ Redis服务: 运行中"
        echo "   订阅者数量: $(redis-cli pubsub numsub market_data | tail -1)"
    else
        echo "❌ Redis服务: 未运行"
    fi
}

# 启动WebSocket服务器
start_websocket() {
    echo "🌐 启动WebSocket服务器..."
    
    if pgrep -f "python3 websocket_app.py" > /dev/null; then
        echo "⚠️ WebSocket服务器已在运行"
        return 1
    fi
    
    # 设置no_proxy环境变量
    export no_proxy="localhost,127.0.0.1,0.0.0.0"
    
    nohup python3 websocket_app.py > websocket.log 2>&1 &
    local pid=$!
    
    echo "WebSocket服务器已启动 (PID: $pid)"
    echo "访问地址: http://localhost:5502"
    echo "实时行情: http://localhost:5502/realtime"
    echo "日志文件: websocket.log"
    
    # 等待服务启动
    sleep 3
    if check_service_status "WebSocket服务器" "python3 websocket_app.py"; then
        echo "✅ WebSocket服务器启动成功"
    else
        echo "❌ WebSocket服务器启动失败，请检查日志"
        tail -10 websocket.log
    fi
}

# 启动行情桥接服务
start_bridge() {
    echo "🌉 启动行情桥接服务..."
    
    if pgrep -f "python3 market_bridge.py" > /dev/null; then
        echo "⚠️ 行情桥接服务已在运行"
        return 1
    fi
    
    nohup python3 market_bridge.py > bridge.log 2>&1 &
    local pid=$!
    
    echo "行情桥接服务已启动 (PID: $pid)"
    echo "ZMQ订阅: tcp://localhost:9999"
    echo "Redis发布: market_data频道"
    echo "日志文件: bridge.log"
    
    # 等待服务启动
    sleep 3
    if check_service_status "行情桥接服务" "python3 market_bridge.py"; then
        echo "✅ 行情桥接服务启动成功"
    else
        echo "❌ 行情桥接服务启动失败，请检查日志"
        tail -10 bridge.log
    fi
}

# 停止所有服务
stop_all() {
    echo "🛑 停止所有实时行情服务..."
    
    echo "停止WebSocket服务器..."
    pkill -f "python3 websocket_app.py"
    
    echo "停止行情桥接服务..."
    pkill -f "python3 market_bridge.py"
    
    echo "清理Redis缓存..."
    redis-cli del market_data_cache > /dev/null 2>&1
    
    sleep 2
    echo "✅ 所有服务已停止"
}

# 启动所有服务
start_all() {
    echo "🚀 启动完整的实时行情系统..."
    
    # 检查依赖
    echo "📋 检查系统依赖..."
    
    if ! command -v redis-cli &> /dev/null; then
        echo "❌ Redis未安装或不在PATH中"
        exit 1
    fi
    
    if ! redis-cli ping > /dev/null 2>&1; then
        echo "❌ Redis服务未运行，请先启动Redis"
        exit 1
    fi
    
    if ! python3 -c "import flask_socketio, redis, zmq" 2>/dev/null; then
        echo "❌ Python依赖不完整，请运行: pip3 install flask-socketio redis pyzmq eventlet"
        exit 1
    fi
    
    echo "✅ 系统依赖检查通过"
    echo ""
    
    # 启动服务
    start_websocket
    echo ""
    start_bridge
    echo ""
    
    # 显示状态
    echo "📊 系统启动完成，当前状态:"
    show_status
    
    echo ""
    echo "🎯 使用说明:"
    echo "1. 打开浏览器访问: http://localhost:5502/realtime"
    echo "2. 点击'开始订阅'按钮开始接收行情"
    echo "3. 点击'发送测试数据'测试系统功能"
    echo "4. 查看日志: tail -f websocket.log bridge.log"
    echo ""
    echo "📝 管理命令:"
    echo "./start_realtime_system.sh status    - 查看状态"
    echo "./start_realtime_system.sh stop      - 停止服务"
    echo "./start_realtime_system.sh restart   - 重启服务"
}

# 重启服务
restart_all() {
    echo "🔄 重启实时行情系统..."
    stop_all
    sleep 3
    start_all
}

# 显示日志
show_logs() {
    echo "📄 显示服务日志..."
    
    if [ -f "websocket.log" ]; then
        echo "=== WebSocket服务器日志 (最近20行) ==="
        tail -20 websocket.log
        echo ""
    fi
    
    if [ -f "bridge.log" ]; then
        echo "=== 行情桥接服务日志 (最近20行) ==="
        tail -20 bridge.log
        echo ""
    fi
}

# 测试系统
test_system() {
    echo "🧪 测试实时行情系统..."
    
    # 测试WebSocket服务器
    echo "测试WebSocket服务器..."
    response=$(curl -s -o /dev/null -w "%{http_code}" http://localhost:5502/api/realtime/status)
    if [ "$response" = "200" ]; then
        echo "✅ WebSocket服务器响应正常"
    else
        echo "❌ WebSocket服务器无响应 (HTTP: $response)"
    fi
    
    # 测试Redis连接
    echo "测试Redis连接..."
    if redis-cli ping > /dev/null 2>&1; then
        echo "✅ Redis连接正常"
    else
        echo "❌ Redis连接失败"
    fi
    
    # 发送测试数据
    echo "发送测试行情数据..."
    response=$(curl -s http://localhost:5502/api/realtime/test)
    if echo "$response" | grep -q "success"; then
        echo "✅ 测试数据发送成功"
    else
        echo "❌ 测试数据发送失败"
    fi
    
    echo ""
    echo "测试完成，请在浏览器中查看: http://localhost:5502/realtime"
}

# 主菜单
case "$1" in
    "status")
        show_status
        ;;
    "start")
        start_all
        ;;
    "start-websocket")
        start_websocket
        ;;
    "start-bridge")
        start_bridge
        ;;
    "stop")
        stop_all
        ;;
    "restart")
        restart_all
        ;;
    "logs")
        show_logs
        ;;
    "test")
        test_system
        ;;
    *)
        echo "用法: $0 {start|stop|restart|status|logs|test|start-websocket|start-bridge}"
        echo ""
        echo "命令说明:"
        echo "  start             - 启动完整的实时行情系统"
        echo "  stop              - 停止所有服务"
        echo "  restart           - 重启所有服务"
        echo "  status            - 显示服务状态"
        echo "  logs              - 显示服务日志"
        echo "  test              - 测试系统功能"
        echo "  start-websocket   - 仅启动WebSocket服务器"
        echo "  start-bridge      - 仅启动行情桥接服务"
        echo ""
        echo "系统架构:"
        echo "  CTP行情 → ZMQ(9999) → 桥接服务 → Redis → WebSocket → 前端显示"
        echo ""
        show_status
        ;;
esac 