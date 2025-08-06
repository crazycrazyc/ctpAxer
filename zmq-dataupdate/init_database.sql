-- 创建数据库
CREATE DATABASE IF NOT EXISTS zmq_data CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;

USE zmq_data;

-- 创建消息表
CREATE TABLE IF NOT EXISTS messages (
    id BIGINT AUTO_INCREMENT PRIMARY KEY,
    message_type VARCHAR(50) NOT NULL COMMENT '消息类型',
    message_content TEXT NOT NULL COMMENT '消息内容',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    INDEX idx_message_type (message_type),
    INDEX idx_created_at (created_at)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='消息存储表';

-- 创建Tick数据表
CREATE TABLE IF NOT EXISTS tick_data (
    id BIGINT AUTO_INCREMENT PRIMARY KEY,
    symbol VARCHAR(20) NOT NULL COMMENT '交易品种',
    price DECIMAL(10,4) NOT NULL COMMENT '价格',
    volume INT NOT NULL COMMENT '成交量',
    timestamp VARCHAR(30) NOT NULL COMMENT '时间戳',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    INDEX idx_symbol (symbol),
    INDEX idx_timestamp (timestamp),
    INDEX idx_created_at (created_at)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='Tick数据表';

-- 创建合约信息表 (基于ProtobufConverter结构)
CREATE TABLE IF NOT EXISTS test_update_instrument (
    id BIGINT AUTO_INCREMENT PRIMARY KEY,
    instrument_id VARCHAR(31) NOT NULL COMMENT '合约代码',
    exchange_id VARCHAR(9) NOT NULL COMMENT '交易所代码',
    instrument_name VARCHAR(21) NOT NULL COMMENT '合约名称',
    exchange_inst_id VARCHAR(31) NOT NULL COMMENT '合约在交易所的代码',
    product_id VARCHAR(21) NOT NULL COMMENT '产品代码',
    product_class CHAR(1) NOT NULL COMMENT '产品类型',
    delivery_year INT NOT NULL COMMENT '交割年份',
    delivery_month INT NOT NULL COMMENT '交割月份',
    max_market_order_volume INT NOT NULL COMMENT '市价单最大下单量',
    min_market_order_volume INT NOT NULL COMMENT '市价单最小下单量',
    max_limit_order_volume INT NOT NULL COMMENT '限价单最大下单量',
    min_limit_order_volume INT NOT NULL COMMENT '限价单最小下单量',
    volume_multiple INT NOT NULL COMMENT '合约数量乘数',
    price_tick DECIMAL(10,8) NOT NULL COMMENT '最小变动价位',
    create_date VARCHAR(9) NOT NULL COMMENT '创建日',
    open_date VARCHAR(9) NOT NULL COMMENT '上市日',
    expire_date VARCHAR(9) NOT NULL COMMENT '到期日',
    start_deliv_date VARCHAR(9) NOT NULL COMMENT '开始交割日',
    end_deliv_date VARCHAR(9) NOT NULL COMMENT '结束交割日',
    inst_life_phase CHAR(1) NOT NULL COMMENT '合约生命周期状态',
    is_trading TINYINT(1) NOT NULL COMMENT '当前是否交易',
    position_type CHAR(1) NOT NULL COMMENT '持仓类型',
    position_date_type CHAR(1) NOT NULL COMMENT '持仓日期类型',
    long_margin_ratio DECIMAL(10,4) NOT NULL COMMENT '多头保证金率',
    short_margin_ratio DECIMAL(10,4) NOT NULL COMMENT '空头保证金率',
    max_margin_side_algorithm CHAR(1) NOT NULL COMMENT '最大保证金算法',
    underlying_instr_id VARCHAR(31) NOT NULL COMMENT '基础商品代码',
    strike_price DECIMAL(10,4) NOT NULL COMMENT '执行价',
    options_type CHAR(1) NOT NULL COMMENT '期权类型',
    underlying_multiple DECIMAL(10,4) NOT NULL COMMENT '基础商品乘数',
    combination_type CHAR(1) NOT NULL COMMENT '组合类型',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
    UNIQUE KEY uk_instrument_exchange (instrument_id, exchange_id),
    INDEX idx_instrument_id (instrument_id),
    INDEX idx_exchange_id (exchange_id),
    INDEX idx_product_id (product_id),
    INDEX idx_is_trading (is_trading),
    INDEX idx_created_at (created_at)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='合约信息更新表';

-- 创建系统状态表
CREATE TABLE IF NOT EXISTS system_status (
    id INT AUTO_INCREMENT PRIMARY KEY,
    component VARCHAR(50) NOT NULL COMMENT '组件名称',
    status VARCHAR(20) NOT NULL COMMENT '状态',
    message TEXT COMMENT '状态信息',
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
    INDEX idx_component (component),
    INDEX idx_status (status)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='系统状态表';

-- 插入初始状态记录
INSERT INTO system_status (component, status, message) VALUES 
('zmq_subscriber', 'initialized', 'ZMQ订阅者已初始化'),
('database', 'connected', '数据库连接正常');

-- 创建用户（如果需要）
-- CREATE USER IF NOT EXISTS 'zmq_user'@'localhost' IDENTIFIED BY 'zmq_password';
-- GRANT ALL PRIVILEGES ON zmq_data.* TO 'zmq_user'@'localhost';
-- FLUSH PRIVILEGES; 