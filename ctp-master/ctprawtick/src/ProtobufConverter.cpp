#include "../include/ProtobufConverter.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <climits>
#include <cfloat>
#include <limits>

// ==================== 合约信息转换实现 ====================

ctp::InstrumentMessage ProtobufConverter::convertToProtobuf(const CThostFtdcInstrumentField& instrument) {
    ctp::InstrumentMessage message;
    
    // 设置各个字段
    message.set_instrument_id(instrument.InstrumentID);
    message.set_exchange_id(instrument.ExchangeID);
    message.set_instrument_name(instrument.InstrumentName);
    message.set_exchange_inst_id(instrument.ExchangeInstID);
    message.set_product_id(instrument.ProductID);
    message.set_product_class(std::string(1, instrument.ProductClass));
    message.set_delivery_year(instrument.DeliveryYear);
    message.set_delivery_month(instrument.DeliveryMonth);
    message.set_max_market_order_volume(instrument.MaxMarketOrderVolume);
    message.set_min_market_order_volume(instrument.MinMarketOrderVolume);
    message.set_max_limit_order_volume(instrument.MaxLimitOrderVolume);
    message.set_min_limit_order_volume(instrument.MinLimitOrderVolume);
    message.set_volume_multiple(instrument.VolumeMultiple);
    message.set_price_tick(instrument.PriceTick);
    message.set_create_date(instrument.CreateDate);
    message.set_open_date(instrument.OpenDate);
    message.set_expire_date(instrument.ExpireDate);
    message.set_start_deliv_date(instrument.StartDelivDate);
    message.set_end_deliv_date(instrument.EndDelivDate);
    message.set_inst_life_phase(std::string(1, instrument.InstLifePhase));
    message.set_is_trading(instrument.IsTrading);
    message.set_position_type(std::string(1, instrument.PositionType));
    message.set_position_date_type(std::string(1, instrument.PositionDateType));
    message.set_long_margin_ratio(instrument.LongMarginRatio);
    message.set_short_margin_ratio(instrument.ShortMarginRatio);
    message.set_max_margin_side_algorithm(std::string(1, instrument.MaxMarginSideAlgorithm));
    message.set_underlying_instr_id(instrument.UnderlyingInstrID);
    message.set_strike_price(instrument.StrikePrice);
    message.set_options_type(std::string(1, instrument.OptionsType));
    message.set_underlying_multiple(instrument.UnderlyingMultiple);
    message.set_combination_type(std::string(1, instrument.CombinationType));
    
    return message;
}

CThostFtdcInstrumentField ProtobufConverter::convertFromProtobuf(const ctp::InstrumentMessage& message) {
    CThostFtdcInstrumentField instrument;
    
    // 初始化结构体
    memset(&instrument, 0, sizeof(instrument));
    
    // 设置各个字段
    strncpy(instrument.InstrumentID, message.instrument_id().c_str(), sizeof(instrument.InstrumentID) - 1);
    strncpy(instrument.ExchangeID, message.exchange_id().c_str(), sizeof(instrument.ExchangeID) - 1);
    strncpy(instrument.InstrumentName, message.instrument_name().c_str(), sizeof(instrument.InstrumentName) - 1);
    strncpy(instrument.ExchangeInstID, message.exchange_inst_id().c_str(), sizeof(instrument.ExchangeInstID) - 1);
    strncpy(instrument.ProductID, message.product_id().c_str(), sizeof(instrument.ProductID) - 1);
    instrument.ProductClass = message.product_class().empty() ? '\0' : message.product_class()[0];
    instrument.DeliveryYear = message.delivery_year();
    instrument.DeliveryMonth = message.delivery_month();
    instrument.MaxMarketOrderVolume = message.max_market_order_volume();
    instrument.MinMarketOrderVolume = message.min_market_order_volume();
    instrument.MaxLimitOrderVolume = message.max_limit_order_volume();
    instrument.MinLimitOrderVolume = message.min_limit_order_volume();
    instrument.VolumeMultiple = message.volume_multiple();
    instrument.PriceTick = message.price_tick();
    strncpy(instrument.CreateDate, message.create_date().c_str(), sizeof(instrument.CreateDate) - 1);
    strncpy(instrument.OpenDate, message.open_date().c_str(), sizeof(instrument.OpenDate) - 1);
    strncpy(instrument.ExpireDate, message.expire_date().c_str(), sizeof(instrument.ExpireDate) - 1);
    strncpy(instrument.StartDelivDate, message.start_deliv_date().c_str(), sizeof(instrument.StartDelivDate) - 1);
    strncpy(instrument.EndDelivDate, message.end_deliv_date().c_str(), sizeof(instrument.EndDelivDate) - 1);
    instrument.InstLifePhase = message.inst_life_phase().empty() ? '\0' : message.inst_life_phase()[0];
    instrument.IsTrading = message.is_trading();
    instrument.PositionType = message.position_type().empty() ? '\0' : message.position_type()[0];
    instrument.PositionDateType = message.position_date_type().empty() ? '\0' : message.position_date_type()[0];
    instrument.LongMarginRatio = message.long_margin_ratio();
    instrument.ShortMarginRatio = message.short_margin_ratio();
    instrument.MaxMarginSideAlgorithm = message.max_margin_side_algorithm().empty() ? '\0' : message.max_margin_side_algorithm()[0];
    strncpy(instrument.UnderlyingInstrID, message.underlying_instr_id().c_str(), sizeof(instrument.UnderlyingInstrID) - 1);
    instrument.StrikePrice = message.strike_price();
    instrument.OptionsType = message.options_type().empty() ? '\0' : message.options_type()[0];
    instrument.UnderlyingMultiple = message.underlying_multiple();
    instrument.CombinationType = message.combination_type().empty() ? '\0' : message.combination_type()[0];
    
    return instrument;
}

ctp::InstrumentBatchMessage ProtobufConverter::convertBatchToProtobuf(const std::vector<CThostFtdcInstrumentField>& instruments) {
    ctp::InstrumentBatchMessage batchMessage;
    
    // 设置时间戳和消息类型
    batchMessage.set_timestamp(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count());
    batchMessage.set_message_type("CTP_INSTRUMENT_BATCH_UPDATE");
    
    // 转换每个合约
    for (const auto& instrument : instruments) {
        auto* message = batchMessage.add_instruments();
        *message = convertToProtobuf(instrument);
    }
    
    return batchMessage;
}

std::vector<CThostFtdcInstrumentField> ProtobufConverter::convertBatchFromProtobuf(const ctp::InstrumentBatchMessage& batchMessage) {
    std::vector<CThostFtdcInstrumentField> instruments;
    
    // 转换每个合约
    for (int i = 0; i < batchMessage.instruments_size(); ++i) {
        instruments.push_back(convertFromProtobuf(batchMessage.instruments(i)));
    }
    
    return instruments;
}

std::string ProtobufConverter::serializeToString(const ctp::InstrumentMessage& message) {
    return message.SerializeAsString();
}

std::string ProtobufConverter::serializeToString(const ctp::InstrumentBatchMessage& batchMessage) {
    return batchMessage.SerializeAsString();
}

bool ProtobufConverter::deserializeFromString(const std::string& data, ctp::InstrumentMessage& message) {
    return message.ParseFromString(data);
}

bool ProtobufConverter::deserializeFromString(const std::string& data, ctp::InstrumentBatchMessage& batchMessage) {
    return batchMessage.ParseFromString(data);
}

std::vector<uint8_t> ProtobufConverter::serializeToBytes(const ctp::InstrumentMessage& message) {
    std::string serialized = message.SerializeAsString();
    return std::vector<uint8_t>(serialized.begin(), serialized.end());
}

std::vector<uint8_t> ProtobufConverter::serializeToBytes(const ctp::InstrumentBatchMessage& batchMessage) {
    std::string serialized = batchMessage.SerializeAsString();
    return std::vector<uint8_t>(serialized.begin(), serialized.end());
}

bool ProtobufConverter::deserializeFromBytes(const std::vector<uint8_t>& data, ctp::InstrumentMessage& message) {
    std::string serialized(data.begin(), data.end());
    return message.ParseFromString(serialized);
}

bool ProtobufConverter::deserializeFromBytes(const std::vector<uint8_t>& data, ctp::InstrumentBatchMessage& batchMessage) {
    std::string serialized(data.begin(), data.end());
    return batchMessage.ParseFromString(serialized);
}



// ==================== 投资者持仓信息转换实现 ====================

ctp::InvestorPositionMessage ProtobufConverter::convertToProtobuf(const CThostFtdcInvestorPositionField& position) {
    ctp::InvestorPositionMessage message;
    
    // 设置各个字段
    message.set_instrument_id(position.InstrumentID);
    message.set_broker_id(position.BrokerID);
    message.set_investor_id(position.InvestorID);
    message.set_posi_direction(std::string(1, position.PosiDirection));
    message.set_hedge_flag(std::string(1, position.HedgeFlag));
    message.set_position_date(std::string(1, position.PositionDate));
    message.set_yd_position(position.YdPosition);
    message.set_position(position.Position);
    message.set_long_frozen(position.LongFrozen);
    message.set_short_frozen(position.ShortFrozen);
    message.set_long_frozen_amount(position.LongFrozenAmount);
    message.set_short_frozen_amount(position.ShortFrozenAmount);
    message.set_open_volume(position.OpenVolume);
    message.set_close_volume(position.CloseVolume);
    message.set_open_amount(position.OpenAmount);
    message.set_close_amount(position.CloseAmount);
    message.set_position_cost(position.PositionCost);
    message.set_pre_margin(position.PreMargin);
    message.set_use_margin(position.UseMargin);
    message.set_frozen_margin(position.FrozenMargin);
    message.set_frozen_cash(position.FrozenCash);
    message.set_frozen_commission(position.FrozenCommission);
    message.set_cash_in(position.CashIn);
    message.set_commission(position.Commission);
    message.set_close_profit(position.CloseProfit);
    message.set_position_profit(position.PositionProfit);
    message.set_pre_settlement_price(position.PreSettlementPrice);
    message.set_settlement_price(position.SettlementPrice);
    message.set_trading_day(position.TradingDay);
    message.set_settlement_id(position.SettlementID);
    message.set_open_cost(position.OpenCost);
    message.set_exchange_margin(position.ExchangeMargin);
    message.set_comb_position(position.CombPosition);
    message.set_comb_long_frozen(position.CombLongFrozen);
    message.set_comb_short_frozen(position.CombShortFrozen);
    message.set_close_profit_by_date(position.CloseProfitByDate);
    message.set_close_profit_by_trade(position.CloseProfitByTrade);
    message.set_today_position(position.TodayPosition);
    message.set_margin_rate_by_money(position.MarginRateByMoney);
    message.set_margin_rate_by_volume(position.MarginRateByVolume);
    message.set_strike_frozen(position.StrikeFrozen);
    message.set_strike_frozen_amount(position.StrikeFrozenAmount);
    message.set_abandon_frozen(position.AbandonFrozen);
    message.set_exchange_id(position.ExchangeID);
    message.set_yd_strike_frozen(position.YdStrikeFrozen);
    message.set_invest_unit_id(position.InvestUnitID);
    message.set_position_cost_offset(position.PositionCostOffset);
    
    return message;
}

ctp::InvestorPositionBatchMessage ProtobufConverter::convertBatchToProtobuf(const std::vector<CThostFtdcInvestorPositionField>& positions) {
    ctp::InvestorPositionBatchMessage batchMessage;
    
    // 设置时间戳和消息类型
    batchMessage.set_timestamp(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count());
    batchMessage.set_message_type("CTP_INVESTOR_POSITION_BATCH_UPDATE");
    
    // 转换每个持仓
    for (const auto& position : positions) {
        auto* message = batchMessage.add_positions();
        *message = convertToProtobuf(position);
    }
    
    return batchMessage;
}

std::string ProtobufConverter::serializeToString(const ctp::InvestorPositionMessage& message) {
    return message.SerializeAsString();
}

std::string ProtobufConverter::serializeToString(const ctp::InvestorPositionBatchMessage& batchMessage) {
    return batchMessage.SerializeAsString();
}

// ==================== 行情数据转换方法 (从MarketDataProtobufConverter迁移) ====================

ctp::MarketDataMessage ProtobufConverter::convertToProtobuf(
    const CThostFtdcDepthMarketDataField& marketData, 
    const std::string& localTimestamp) {
    
    ctp::MarketDataMessage message;
    
    // 基本信息
    message.set_trading_day(marketData.TradingDay ? marketData.TradingDay : "");
    message.set_instrument_id(marketData.InstrumentID ? marketData.InstrumentID : "");
    message.set_exchange_id(marketData.ExchangeID ? marketData.ExchangeID : "");
    message.set_exchange_inst_id(marketData.ExchangeInstID ? marketData.ExchangeInstID : "");
    
    // 价格信息
    message.set_last_price(safeDouble(marketData.LastPrice));
    message.set_pre_settlement_price(safeDouble(marketData.PreSettlementPrice));
    message.set_pre_close_price(safeDouble(marketData.PreClosePrice));
    message.set_pre_open_interest(safeDouble(marketData.PreOpenInterest));
    message.set_open_price(safeDouble(marketData.OpenPrice));
    message.set_highest_price(safeDouble(marketData.HighestPrice));
    message.set_lowest_price(safeDouble(marketData.LowestPrice));
    
    // 成交信息
    message.set_volume(safeInt(marketData.Volume));
    message.set_turnover(safeDouble(marketData.Turnover));
    message.set_open_interest(safeDouble(marketData.OpenInterest));
    message.set_close_price(safeDouble(marketData.ClosePrice));
    message.set_settlement_price(safeDouble(marketData.SettlementPrice));
    
    // 涨跌停信息
    message.set_upper_limit_price(safeDouble(marketData.UpperLimitPrice));
    message.set_lower_limit_price(safeDouble(marketData.LowerLimitPrice));
    
    // 虚实度
    message.set_pre_delta(safeDouble(marketData.PreDelta));
    message.set_curr_delta(safeDouble(marketData.CurrDelta));
    
    // 时间信息
    message.set_update_time(marketData.UpdateTime ? marketData.UpdateTime : "");
    message.set_update_millisec(safeInt(marketData.UpdateMillisec));
    
    // 买卖盘口信息
    message.set_bid_price1(safeDouble(marketData.BidPrice1));
    message.set_bid_volume1(safeInt(marketData.BidVolume1));
    message.set_ask_price1(safeDouble(marketData.AskPrice1));
    message.set_ask_volume1(safeInt(marketData.AskVolume1));
    
    message.set_bid_price2(safeDouble(marketData.BidPrice2));
    message.set_bid_volume2(safeInt(marketData.BidVolume2));
    message.set_ask_price2(safeDouble(marketData.AskPrice2));
    message.set_ask_volume2(safeInt(marketData.AskVolume2));
    
    message.set_bid_price3(safeDouble(marketData.BidPrice3));
    message.set_bid_volume3(safeInt(marketData.BidVolume3));
    message.set_ask_price3(safeDouble(marketData.AskPrice3));
    message.set_ask_volume3(safeInt(marketData.AskVolume3));
    
    message.set_bid_price4(safeDouble(marketData.BidPrice4));
    message.set_bid_volume4(safeInt(marketData.BidVolume4));
    message.set_ask_price4(safeDouble(marketData.AskPrice4));
    message.set_ask_volume4(safeInt(marketData.AskVolume4));
    
    message.set_bid_price5(safeDouble(marketData.BidPrice5));
    message.set_bid_volume5(safeInt(marketData.BidVolume5));
    message.set_ask_price5(safeDouble(marketData.AskPrice5));
    message.set_ask_volume5(safeInt(marketData.AskVolume5));
    
    // 其他信息
    message.set_average_price(safeDouble(marketData.AveragePrice));
    message.set_action_day(marketData.ActionDay ? marketData.ActionDay : "");
    
    // 时间戳信息
    message.set_local_timestamp(localTimestamp);
    message.set_timestamp(generateTimestamp());
    
    return message;
}

std::string ProtobufConverter::serializeToString(const ctp::MarketDataMessage& message) {
    std::string serialized;
    if (message.SerializeToString(&serialized)) {
        return serialized;
    }
    return "";
}

// ==================== 工具方法 (从MarketDataProtobufConverter迁移) ====================

std::string ProtobufConverter::generateLocalTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    ss << "." << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
}

int64_t ProtobufConverter::generateTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    return ms.count();
}

double ProtobufConverter::safeDouble(double value) {
    // 检查是否为有效的double值
    if (value == DBL_MAX || value == -DBL_MAX || value != value || // NaN check
        value == std::numeric_limits<double>::infinity() || 
        value == -std::numeric_limits<double>::infinity()) {
        return 0.0;
    }
    return value;
}

int32_t ProtobufConverter::safeInt(int value) {
    // 检查是否为有效的int值
    if (value == INT_MAX || value == INT_MIN) {
        return 0;
    }
    return static_cast<int32_t>(value);
} 