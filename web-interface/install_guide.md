# CTP合约查询系统 - 安装指导

## 系统要求

您的系统需要安装以下Python包才能运行此应用：

## 方法1：使用系统包管理器安装（推荐）

```bash
# Ubuntu/Debian系统
sudo apt update
sudo apt install python3-pip python3-flask python3-mysql.connector

# 或者使用apt安装所有依赖
sudo apt install python3-pip
sudo apt install python3-flask
sudo apt install python3-jinja2
sudo apt install python3-mysql.connector
sudo apt install python3-pandas
```

## 方法2：安装pip后使用pip安装

```bash
# 安装pip
sudo apt install python3-pip

# 然后安装依赖
pip3 install -r requirements.txt
```

## 方法3：使用conda（如果已安装）

```bash
conda install flask mysql-connector-python pandas
```

## 验证安装

运行以下命令验证所需包是否已安装：

```bash
python3 -c "import flask; print('✓ Flask已安装')"
python3 -c "import mysql.connector; print('✓ MySQL Connector已安装')"
python3 -c "import pandas; print('✓ Pandas已安装')"
```

## 启动应用

安装完成后，运行：

```bash
python3 websocket_app.py
```

然后在浏览器中访问：`http://localhost:5000`

## 数据库配置

确保您的MySQL数据库：
1. 服务正在运行
2. 有正确的连接参数（在websocket_app.py中配置）
3. 包含instruments表和数据

## 故障排除

如果遇到权限问题，可能需要：
```bash
sudo apt install python3-pip
```

如果遇到网络问题，可以尝试：
```bash
pip3 install -i https://pypi.tuna.tsinghua.edu.cn/simple -r requirements.txt
``` 