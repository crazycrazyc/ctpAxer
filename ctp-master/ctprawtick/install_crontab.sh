#!/bin/bash

# CTP分离架构定时任务安装脚本
# 根据用户需求优化：合约一天2次，持仓5分钟一次，行情10分钟一次

echo "========== CTP分离架构定时任务安装程序 =========="
echo ""

# 获取当前目录
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
echo "当前脚本目录: $SCRIPT_DIR"

# 检查是否存在优化的crontab配置文件
CRONTAB_FILE="$SCRIPT_DIR/crontab_optimized.txt"
if [ ! -f "$CRONTAB_FILE" ]; then
    echo "错误: 找不到 $CRONTAB_FILE"
    exit 1
fi

echo "发现定时任务配置文件: $CRONTAB_FILE"
echo ""

# 显示当前的crontab任务
echo "当前的定时任务:"
crontab -l 2>/dev/null || echo "  (无定时任务)"
echo ""

# 询问是否继续
read -p "是否要安装CTP分离架构的定时任务? (y/N): " confirm
if [[ $confirm != [yY] && $confirm != [yY][eE][sS] ]]; then
    echo "安装取消"
    exit 0
fi

# 备份现有的crontab
echo "备份现有定时任务..."
crontab -l > "$SCRIPT_DIR/crontab_backup_$(date +%Y%m%d_%H%M%S).txt" 2>/dev/null
echo "备份完成: crontab_backup_$(date +%Y%m%d_%H%M%S).txt"

# 安装新的crontab
echo "安装新的定时任务..."
crontab "$CRONTAB_FILE"

if [ $? -eq 0 ]; then
    echo "✅ 定时任务安装成功!"
    echo ""
    echo "新的定时任务配置:"
    crontab -l
    echo ""
    echo "========== 定时任务说明 =========="
    echo "1. 合约查询器: 每日8:30和15:30运行"
    echo "2. 持仓资金监控: 交易时间内每5分钟运行"  
    echo "3. 行情数据推送: 交易时间内每10分钟重启"
    echo "4. 系统维护: 自动清理进程和日志"
    echo ""
    echo "========== 监控命令 =========="
    echo "查看程序状态: $SCRIPT_DIR/start_system.sh status"
    echo "查看行情日志: tail -f $SCRIPT_DIR/bin/debug/market.log"
    echo "查看状态日志: tail -f /tmp/ctp_status_\$(date +%Y%m%d).log"
    echo ""
    echo "========== 手动控制 =========="
    echo "启动所有程序: $SCRIPT_DIR/start_system.sh start-all"
    echo "停止所有程序: $SCRIPT_DIR/start_system.sh stop"
    echo "重启程序: $SCRIPT_DIR/start_system.sh restart"
    echo ""
else
    echo "❌ 定时任务安装失败!"
    exit 1
fi 