-- 创建合约信息表
CREATE TABLE IF NOT EXISTS test_update_instrument (
    id BIGINT AUTO_INCREMENT PRIMARY KEY COMMENT '主键ID',
    InstrumentID VARCHAR(31) NOT NULL COMMENT '合约代码',
    ExchangeID VARCHAR(9) NOT NULL COMMENT '交易所代码',
    InstrumentName VARCHAR(21) NOT NULL COMMENT '合约名称',
    ExchangeInstID VARCHAR(31) NOT NULL COMMENT '合约在交易所的代码',
    ProductID VARCHAR(21) NOT NULL COMMENT '产品代码',
    ProductClass CHAR(1) NOT NULL COMMENT '产品类型',
    DeliveryYear INT NOT NULL COMMENT '交割年份',
    DeliveryMonth INT NOT NULL COMMENT '交割月份',
    MaxMarketOrderVolume INT NOT NULL COMMENT '市价单最大下单量',
    MinMarketOrderVolume INT NOT NULL COMMENT '市价单最小下单量',
    MaxLimitOrderVolume INT NOT NULL COMMENT '限价单最大下单量',
    MinLimitOrderVolume INT NOT NULL COMMENT '限价单最小下单量',
    VolumeMultiple INT NOT NULL COMMENT '合约数量乘数',
    PriceTick DECIMAL(10,8) NOT NULL COMMENT '最小变动价位',
    CreateDate VARCHAR(9) NOT NULL COMMENT '创建日',
    OpenDate VARCHAR(9) NOT NULL COMMENT '上市日',
    ExpireDate VARCHAR(9) NOT NULL COMMENT '到期日',
    StartDelivDate VARCHAR(9) NOT NULL COMMENT '开始交割日',
    EndDelivDate VARCHAR(9) NOT NULL COMMENT '结束交割日',
    InstLifePhase CHAR(1) NOT NULL COMMENT '合约生命周期状态',
    IsTrading TINYINT(1) NOT NULL COMMENT '当前是否交易',
    PositionType CHAR(1) NOT NULL COMMENT '持仓类型',
    PositionDateType CHAR(1) NOT NULL COMMENT '持仓日期类型',
    LongMarginRatio DECIMAL(10,4) NOT NULL COMMENT '多头保证金率',
    ShortMarginRatio DECIMAL(10,4) NOT NULL COMMENT '空头保证金率',
    MaxMarginSideAlgorithm CHAR(1) NOT NULL COMMENT '最大保证金算法',
    UnderlyingInstrID VARCHAR(31) NOT NULL COMMENT '基础商品代码',
    StrikePrice DECIMAL(10,4) NOT NULL COMMENT '执行价',
    OptionsType CHAR(1) NOT NULL COMMENT '期权类型',
    UnderlyingMultiple DECIMAL(10,4) NOT NULL DEFAULT 1.0000 COMMENT '基础商品乘数',
    CombinationType CHAR(1) NOT NULL DEFAULT '0' COMMENT '组合类型',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
    
    -- 索引
    UNIQUE KEY uk_instrument_id (InstrumentID),
    INDEX idx_exchange_id (ExchangeID),
    INDEX idx_product_id (ProductID),
    INDEX idx_instrument_name (InstrumentName),
    INDEX idx_is_trading (IsTrading),
    INDEX idx_product_class (ProductClass),
    INDEX idx_delivery_year_month (DeliveryYear, DeliveryMonth),
    INDEX idx_expire_date (ExpireDate),
    INDEX idx_created_at (created_at),
    INDEX idx_updated_at (updated_at)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='合约信息表';

-- 插入示例数据（可选）
-- INSERT INTO test_update_instrument (
--     InstrumentID, ExchangeID, InstrumentName, ExchangeInstID, ProductID, ProductClass,
--     DeliveryYear, DeliveryMonth, MaxMarketOrderVolume, MinMarketOrderVolume, 
--     MaxLimitOrderVolume, MinLimitOrderVolume, VolumeMultiple, PriceTick, CreateDate,
--     OpenDate, ExpireDate, StartDelivDate, EndDelivDate, InstLifePhase, IsTrading,
--     PositionType, PositionDateType, LongMarginRatio, ShortMarginRatio, 
--     MaxMarginSideAlgorithm, UnderlyingInstrID, StrikePrice, OptionsType,
--     UnderlyingMultiple, CombinationType
-- ) VALUES (
--     'CU2508', 'SHFE', '沪铜2508', 'cu2508', 'cu', '1', 2025, 8, 500, 1, 500, 1,
--     5, 10.0000, '20240801', '20240801', '20250815', '20250816', '20250820', '1',
--     1, '1', '1', 0.0800, 0.0800, '1', '', 0.0000, '0', 1.0000, '0'
-- );
--
-- INSERT INTO test_update_instrument (
--     InstrumentID, ExchangeID, InstrumentName, ExchangeInstID, ProductID, ProductClass,
--     DeliveryYear, DeliveryMonth, MaxMarketOrderVolume, MinMarketOrderVolume,
--     MaxLimitOrderVolume, MinLimitOrderVolume, VolumeMultiple, PriceTick, CreateDate,
--     OpenDate, ExpireDate, StartDelivDate, EndDelivDate, InstLifePhase, IsTrading,
--     PositionType, PositionDateType, LongMarginRatio, ShortMarginRatio,
--     MaxMarginSideAlgorithm, UnderlyingInstrID, StrikePrice, OptionsType,
--     UnderlyingMultiple, CombinationType
-- ) VALUES (
--     'IF2508', 'CFFEX', '沪深300指数期货2508', 'IF2508', 'IF', '1', 2025, 8, 20, 1, 20, 1,
--     300, 0.2000, '20240701', '20240701', '20250821', '20250821', '20250821', '1',
--     1, '1', '1', 0.1200, 0.1200, '1', '', 0.0000, '0', 1.0000, '0'
-- ); 