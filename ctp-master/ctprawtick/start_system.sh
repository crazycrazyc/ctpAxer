#!/bin/bash

# CTP分离架构启动脚本
# 作者: AI助手
# 日期: 2024-07-29

cd /home/zyc/projects/ctprade/ctp-master/ctprawtick/bin/debug

echo "========== CTP分离架构管理脚本 =========="
echo "当前时间: $(date '+%Y-%m-%d %H:%M:%S')"
echo ""

# 显示程序状态
show_status() {
    echo "程序运行状态:"
    echo "合约查询器 (ctpinstrument): $(pgrep -f ctpinstrument > /dev/null && echo '运行中' || echo '未运行')"
    echo "持仓资金监控器 (ctpmonitor): $(pgrep -f ctpmonitor > /dev/null && echo '运行中' || echo '未运行')"
    echo "行情数据推送器 (ctpmarket): $(pgrep -f ctpmarket > /dev/null && echo '运行中' || echo '未运行')"
    echo ""
}

# 启动合约查询器
start_instrument() {
    echo "启动合约查询器..."
    if pgrep -f ctpinstrument > /dev/null; then
        echo "合约查询器已在运行"
    else
        ./ctpinstrument &
        echo "合约查询器已启动 (后台运行)"
    fi
}

# 启动持仓资金监控器
start_monitor() {
    echo "启动持仓资金监控器..."
    if pgrep -f ctpmonitor > /dev/null; then
        echo "持仓资金监控器已在运行"
    else
        nohup ./ctpmonitor > monitor.log 2>&1 &
        echo "持仓资金监控器已启动 (后台运行，日志: monitor.log)"
    fi
}

# 启动行情推送器
start_market() {
    echo "启动行情数据推送器..."
    if pgrep -f ctpmarket > /dev/null; then
        echo "行情数据推送器已在运行"
    else
        nohup ./ctpmarket > market.log 2>&1 &
        echo "行情数据推送器已启动 (后台运行，日志: market.log)"
    fi
}

# 停止所有程序
stop_all() {
    echo "停止所有CTP程序..."
    pkill -f ctpinstrument
    pkill -f ctpmonitor
    pkill -f ctpmarket
    echo "所有程序已停止"
}

# 主菜单
case "$1" in
    "status")
        show_status
        ;;
    "start-instrument")
        start_instrument
        ;;
    "start-monitor")
        start_monitor
        ;;
    "start-market")
        start_market
        ;;
    "start-all")
        echo "启动所有程序..."
        start_instrument
        sleep 2
        start_monitor
        sleep 2
        start_market
        echo ""
        show_status
        ;;
    "stop")
        stop_all
        ;;
    "restart")
        stop_all
        sleep 3
        start_monitor
        sleep 2
        start_market
        echo ""
        show_status
        ;;
    *)
        echo "用法: $0 {status|start-instrument|start-monitor|start-market|start-all|stop|restart}"
        echo ""
        echo "命令说明:"
        echo "  status           - 显示程序运行状态"
        echo "  start-instrument - 启动合约查询器 (每日运行2次)"
        echo "  start-monitor    - 启动持仓资金监控器 (持续运行)"
        echo "  start-market     - 启动行情数据推送器 (持续运行)"
        echo "  start-all        - 启动所有程序"
        echo "  stop             - 停止所有程序"
        echo "  restart          - 重启监控和行情程序"
        echo ""
        echo "推荐使用方式:"
        echo "  1. 每日8:30和15:30运行: ./start_system.sh start-instrument"
        echo "  2. 交易日开盘前启动: ./start_system.sh start-all"
        echo "  3. 查看运行状态: ./start_system.sh status"
        echo ""
        echo "架构特性:"
        echo "  • 行情数据使用protobuf格式传输 (高效+类型安全)"
        echo "  • 数据库30秒批量写入 (降低压力90%+)"
        echo "  • 分离式设计 (合约查询器 + 监控器 + 行情推送器)"
        echo ""
        show_status
        ;;
esac 