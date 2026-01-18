/**
 * @file binance_future_parser_tests.cpp
 * @author Kevin Rodrigues
 * @brief Tests for Binance Future Parser with SIMD optimizations
 * @version 1.0
 * @date 10/10/2025
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <chrono>
#include <vector>
#include <string>

#include <faster_parser/binance/future.h>

using namespace core::faster_parser::binance;
using namespace core::faster_parser::binance::types;

class MockListener {
public:
    std::vector<book_ticker_t> book_tickers;
    std::vector<trade_t> agg_trades;
    std::vector<ticker_t> tickers;

    void on_book_ticker(const book_ticker_t& ticker) {
        book_tickers.push_back(ticker);
    }

    void on_trade(const trade_t& trade) {
        agg_trades.push_back(trade);
    }

    void on_ticker(const ticker_t& ticker) {
        tickers.push_back(ticker);
    }

    void reset() {
        book_tickers.clear();
        agg_trades.clear();
        tickers.clear();
    }
};

class binance_future_parser_test_t : public ::testing::Test {
protected:
    MockListener listener;

    void SetUp() override {
        listener.reset();
    }

    auto now() {
        return std::chrono::system_clock::now();
    }
};

TEST_F(binance_future_parser_test_t, ParseValidBookTickerMessage) {
    std::string_view message = R"({"e":"bookTicker","u":8822354685185,"s":"ASTERUSDT","b":"1.5822000","B":"457","a":"1.5823000","A":"112","T":1760083106579,"E":1760083106579})";

    bool result = binance_future_parser_t::parse(now(), message, listener);

    EXPECT_TRUE(result);
    ASSERT_EQ(listener.book_tickers.size(), 1);

    const auto& ticker = listener.book_tickers[0];

    EXPECT_EQ(ticker.symbol, "ASTERUSDT");

    EXPECT_DOUBLE_EQ(ticker.bid.price, 1.5822000);
    EXPECT_DOUBLE_EQ(ticker.bid.volume, 457.0);
    EXPECT_DOUBLE_EQ(ticker.ask.price, 1.5823000);
    EXPECT_DOUBLE_EQ(ticker.ask.volume, 112.0);

    EXPECT_EQ(ticker.exchange_timestamp, 1760083106579ULL);
    EXPECT_EQ(ticker.bid.sequence, 8822354685185ULL);
    EXPECT_EQ(ticker.ask.sequence, 8822354685185ULL);
}

TEST_F(binance_future_parser_test_t, ParseDifferentPrices) {
    std::string_view message = R"({"e":"bookTicker","u":123456789,"s":"BTCUSDT","b":"45123.78900000","B":"10.5","a":"45124.12300000","A":"5.25","T":1234567890123,"E":1234567890123})";

    bool result = binance_future_parser_t::parse(now(), message, listener);

    EXPECT_TRUE(result);
    ASSERT_EQ(listener.book_tickers.size(), 1);

    const auto& ticker = listener.book_tickers[0];

    EXPECT_EQ(ticker.symbol, "BTCUSDT");

    EXPECT_DOUBLE_EQ(ticker.bid.price, 45123.789);
    EXPECT_DOUBLE_EQ(ticker.bid.volume, 10.5);
    EXPECT_DOUBLE_EQ(ticker.ask.price, 45124.123);
    EXPECT_DOUBLE_EQ(ticker.ask.volume, 5.25);
}

TEST_F(binance_future_parser_test_t, ParseSmallPrices) {
    std::string_view message = R"({"e":"bookTicker","u":999999,"s":"DOGEUSDT","b":"0.00012345","B":"1000000","a":"0.00012346","A":"999999","T":9999999999,"E":9999999999})";

    bool result = binance_future_parser_t::parse(now(), message, listener);

    EXPECT_TRUE(result);
    ASSERT_EQ(listener.book_tickers.size(), 1);

    const auto& ticker = listener.book_tickers[0];

    EXPECT_DOUBLE_EQ(ticker.bid.price, 0.00012345);
    EXPECT_DOUBLE_EQ(ticker.ask.price, 0.00012346);
    EXPECT_DOUBLE_EQ(ticker.bid.volume, 1000000.0);
    EXPECT_DOUBLE_EQ(ticker.ask.volume, 999999.0);
}

TEST_F(binance_future_parser_test_t, ParseIntegerPrices) {
    std::string_view message = R"({"e":"bookTicker","u":111111111,"s":"ETHUSDT","b":"3000","B":"100","a":"3001","A":"200","T":1111111111111,"E":1111111111111})";

    bool result = binance_future_parser_t::parse(now(), message, listener);

    EXPECT_TRUE(result);
    ASSERT_EQ(listener.book_tickers.size(), 1);

    const auto& ticker = listener.book_tickers[0];

    EXPECT_DOUBLE_EQ(ticker.bid.price, 3000.0);
    EXPECT_DOUBLE_EQ(ticker.ask.price, 3001.0);
    EXPECT_DOUBLE_EQ(ticker.bid.volume, 100.0);
    EXPECT_DOUBLE_EQ(ticker.ask.volume, 200.0);
}

TEST_F(binance_future_parser_test_t, ParseVeryLargeNumbers) {
    std::string_view message = R"({"e":"bookTicker","u":18446744073709551615,"s":"TESTUSDT","b":"99999.99999999","B":"99999.99999999","a":"100000.00000000","A":"100000.00000000","T":9999999999999,"E":9999999999999})";

    bool result = binance_future_parser_t::parse(now(), message, listener);

    EXPECT_TRUE(result);
    ASSERT_EQ(listener.book_tickers.size(), 1);

    const auto& ticker = listener.book_tickers[0];

    EXPECT_EQ(ticker.bid.sequence, 18446744073709551615ULL);
    EXPECT_DOUBLE_EQ(ticker.bid.price, 99999.99999999);
    EXPECT_DOUBLE_EQ(ticker.ask.price, 100000.0);
}

TEST_F(binance_future_parser_test_t, ParseMultipleMessages) {
    std::vector<std::string> messages = {
        R"({"e":"bookTicker","u":1,"s":"A","b":"1.0","B":"1","a":"1.1","A":"1","T":1,"E":1})",
        R"({"e":"bookTicker","u":2,"s":"B","b":"2.0","B":"2","a":"2.1","A":"2","T":2,"E":2})",
        R"({"e":"bookTicker","u":3,"s":"C","b":"3.0","B":"3","a":"3.1","A":"3","T":3,"E":3})"
    };

    for (const auto& msg : messages) {
        bool result = binance_future_parser_t::parse(now(), msg, listener);
        EXPECT_TRUE(result);
    }

    ASSERT_EQ(listener.book_tickers.size(), 3);

    EXPECT_EQ(listener.book_tickers[0].symbol, "A");
    EXPECT_EQ(listener.book_tickers[0].bid.sequence, 1ULL);
    EXPECT_DOUBLE_EQ(listener.book_tickers[0].bid.price, 1.0);

    EXPECT_EQ(listener.book_tickers[1].symbol, "B");
    EXPECT_EQ(listener.book_tickers[1].bid.sequence, 2ULL);
    EXPECT_DOUBLE_EQ(listener.book_tickers[1].bid.price, 2.0);

    EXPECT_EQ(listener.book_tickers[2].symbol, "C");
    EXPECT_EQ(listener.book_tickers[2].bid.sequence, 3ULL);
    EXPECT_DOUBLE_EQ(listener.book_tickers[2].bid.price, 3.0);
}

TEST_F(binance_future_parser_test_t, ParseWithTrailingZeros) {
    std::string_view message = R"({"e":"bookTicker","u":12345,"s":"BTCUSDT","b":"50000.00000000","B":"1.00000000","a":"50001.00000000","A":"2.00000000","T":1234567890,"E":1234567890})";

    bool result = binance_future_parser_t::parse(now(), message, listener);

    EXPECT_TRUE(result);
    ASSERT_EQ(listener.book_tickers.size(), 1);

    const auto& ticker = listener.book_tickers[0];

    EXPECT_DOUBLE_EQ(ticker.bid.price, 50000.0);
    EXPECT_DOUBLE_EQ(ticker.ask.price, 50001.0);
    EXPECT_DOUBLE_EQ(ticker.bid.volume, 1.0);
    EXPECT_DOUBLE_EQ(ticker.ask.volume, 2.0);
}

TEST_F(binance_future_parser_test_t, ParseFewerDecimals) {
    std::string_view message = R"({"e":"bookTicker","u":777,"s":"ETHUSDT","b":"3500.5","B":"10.25","a":"3500.75","A":"20.5","T":777777,"E":777777})";

    bool result = binance_future_parser_t::parse(now(), message, listener);

    EXPECT_TRUE(result);
    ASSERT_EQ(listener.book_tickers.size(), 1);

    const auto& ticker = listener.book_tickers[0];

    EXPECT_DOUBLE_EQ(ticker.bid.price, 3500.5);
    EXPECT_DOUBLE_EQ(ticker.ask.price, 3500.75);
    EXPECT_DOUBLE_EQ(ticker.bid.volume, 10.25);
    EXPECT_DOUBLE_EQ(ticker.ask.volume, 20.5);
}

TEST_F(binance_future_parser_test_t, RejectInvalidMessage) {
    std::string_view invalid_message = R"({"e":"trade","t":123456})";

    bool result = binance_future_parser_t::parse(now(), invalid_message, listener);

    EXPECT_FALSE(result);
    EXPECT_EQ(listener.book_tickers.size(), 0);
}

TEST_F(binance_future_parser_test_t, RejectTooShortMessage) {
    std::string_view short_message = R"({"e":"book"})";

    bool result = binance_future_parser_t::parse(now(), short_message, listener);

    EXPECT_FALSE(result);
    EXPECT_EQ(listener.book_tickers.size(), 0);
}

TEST_F(binance_future_parser_test_t, ConsistencyTest) {
    std::string_view message = R"({"e":"bookTicker","u":8822354685185,"s":"ASTERUSDT","b":"1.5822000","B":"457","a":"1.5823000","A":"112","T":1760083106579,"E":1760083106579})";

    for (int i = 0; i < 100; ++i) {
        listener.reset();
        bool result = binance_future_parser_t::parse(now(), message, listener);

        EXPECT_TRUE(result);
        ASSERT_EQ(listener.book_tickers.size(), 1);

        const auto& ticker = listener.book_tickers[0];

        EXPECT_DOUBLE_EQ(ticker.bid.price, 1.5822);
        EXPECT_DOUBLE_EQ(ticker.bid.volume, 457.0);
        EXPECT_DOUBLE_EQ(ticker.ask.price, 1.5823);
        EXPECT_DOUBLE_EQ(ticker.ask.volume, 112.0);
        EXPECT_EQ(ticker.exchange_timestamp, 1760083106579ULL);
        EXPECT_EQ(ticker.bid.sequence, 8822354685185ULL);
    }
}

// ============================================================================
// Aggregate Trade Tests
// ============================================================================

TEST_F(binance_future_parser_test_t, ParseValidAggTradeMessage) {
    std::string_view message = R"({"e":"aggTrade","E":123456789,"s":"BTCUSDT","a":5933014,"p":"0.001","q":"100","f":100,"l":105,"T":123456785,"m":true})";

    bool result = binance_future_parser_t::parse(now(), message, listener);

    EXPECT_TRUE(result);
    ASSERT_EQ(listener.agg_trades.size(), 1);
    EXPECT_EQ(listener.book_tickers.size(), 0);

    const auto& trade = listener.agg_trades[0];

    EXPECT_EQ(trade.symbol, "BTCUSDT");
    EXPECT_EQ(trade.event_time, 123456789ULL);
    EXPECT_EQ(trade.agg_trade_id, 5933014ULL);
    EXPECT_DOUBLE_EQ(trade.price, 0.001);
    EXPECT_DOUBLE_EQ(trade.quantity, 100.0);
    EXPECT_EQ(trade.first_trade_id, 100ULL);
    EXPECT_EQ(trade.last_trade_id, 105ULL);
    EXPECT_EQ(trade.trade_time, 123456785ULL);
    EXPECT_TRUE(trade.is_buyer_maker);
}

TEST_F(binance_future_parser_test_t, ParseAggTradeBuyerNotMaker) {
    std::string_view message = R"({"e":"aggTrade","E":987654321,"s":"ETHUSDT","a":8888888,"p":"3500.50","q":"10.5","f":200,"l":210,"T":987654320,"m":false})";

    bool result = binance_future_parser_t::parse(now(), message, listener);

    EXPECT_TRUE(result);
    ASSERT_EQ(listener.agg_trades.size(), 1);

    const auto& trade = listener.agg_trades[0];

    EXPECT_EQ(trade.symbol, "ETHUSDT");
    EXPECT_DOUBLE_EQ(trade.price, 3500.50);
    EXPECT_DOUBLE_EQ(trade.quantity, 10.5);
    EXPECT_FALSE(trade.is_buyer_maker);
}

TEST_F(binance_future_parser_test_t, ParseAggTradeSmallPrice) {
    std::string_view message = R"({"e":"aggTrade","E":111111111,"s":"DOGEUSDT","a":99999,"p":"0.00012345","q":"1000000","f":50000,"l":50010,"T":111111110,"m":true})";

    bool result = binance_future_parser_t::parse(now(), message, listener);

    EXPECT_TRUE(result);
    ASSERT_EQ(listener.agg_trades.size(), 1);

    const auto& trade = listener.agg_trades[0];

    EXPECT_EQ(trade.symbol, "DOGEUSDT");
    EXPECT_DOUBLE_EQ(trade.price, 0.00012345);
    EXPECT_DOUBLE_EQ(trade.quantity, 1000000.0);
    EXPECT_EQ(trade.agg_trade_id, 99999ULL);
}

TEST_F(binance_future_parser_test_t, ParseAggTradeLargeNumbers) {
    std::string_view message = R"({"e":"aggTrade","E":9999999999999,"s":"BTCUSDT","a":18446744073709551615,"p":"99999.99999999","q":"999.999","f":18446744073709551600,"l":18446744073709551615,"T":9999999999998,"m":false})";

    bool result = binance_future_parser_t::parse(now(), message, listener);

    EXPECT_TRUE(result);
    ASSERT_EQ(listener.agg_trades.size(), 1);

    const auto& trade = listener.agg_trades[0];

    EXPECT_EQ(trade.agg_trade_id, 18446744073709551615ULL);
    EXPECT_DOUBLE_EQ(trade.price, 99999.99999999);
    EXPECT_DOUBLE_EQ(trade.quantity, 999.999);
    EXPECT_EQ(trade.first_trade_id, 18446744073709551600ULL);
    EXPECT_EQ(trade.last_trade_id, 18446744073709551615ULL);
}

TEST_F(binance_future_parser_test_t, ParseAggTradeIntegerPrices) {
    std::string_view message = R"({"e":"aggTrade","E":222222222,"s":"BNBUSDT","a":777777,"p":"500","q":"25","f":1000,"l":1005,"T":222222221,"m":true})";

    bool result = binance_future_parser_t::parse(now(), message, listener);

    EXPECT_TRUE(result);
    ASSERT_EQ(listener.agg_trades.size(), 1);

    const auto& trade = listener.agg_trades[0];

    EXPECT_DOUBLE_EQ(trade.price, 500.0);
    EXPECT_DOUBLE_EQ(trade.quantity, 25.0);
}

TEST_F(binance_future_parser_test_t, ParseMultipleAggTrades) {
    std::vector<std::string> messages = {
        R"({"e":"aggTrade","E":1,"s":"A","a":1,"p":"1.0","q":"1","f":1,"l":1,"T":1,"m":true})",
        R"({"e":"aggTrade","E":2,"s":"B","a":2,"p":"2.0","q":"2","f":2,"l":2,"T":2,"m":false})",
        R"({"e":"aggTrade","E":3,"s":"C","a":3,"p":"3.0","q":"3","f":3,"l":3,"T":3,"m":true})"
    };

    for (const auto& msg : messages) {
        bool result = binance_future_parser_t::parse(now(), msg, listener);
        EXPECT_TRUE(result);
    }

    ASSERT_EQ(listener.agg_trades.size(), 3);

    EXPECT_EQ(listener.agg_trades[0].symbol, "A");
    EXPECT_DOUBLE_EQ(listener.agg_trades[0].price, 1.0);
    EXPECT_TRUE(listener.agg_trades[0].is_buyer_maker);

    EXPECT_EQ(listener.agg_trades[1].symbol, "B");
    EXPECT_DOUBLE_EQ(listener.agg_trades[1].price, 2.0);
    EXPECT_FALSE(listener.agg_trades[1].is_buyer_maker);

    EXPECT_EQ(listener.agg_trades[2].symbol, "C");
    EXPECT_DOUBLE_EQ(listener.agg_trades[2].price, 3.0);
    EXPECT_TRUE(listener.agg_trades[2].is_buyer_maker);
}

TEST_F(binance_future_parser_test_t, ParseMixedBookTickerAndAggTrade) {
    std::vector<std::string> messages = {
        R"({"e":"bookTicker","u":1,"s":"BTCUSDT","b":"50000","B":"1","a":"50001","A":"1","T":1,"E":1})",
        R"({"e":"aggTrade","E":2,"s":"ETHUSDT","a":2,"p":"3500.5","q":"10","f":2,"l":2,"T":2,"m":true})",
        R"({"e":"bookTicker","u":3,"s":"BNBUSDT","b":"400","B":"5","a":"401","A":"5","T":3,"E":3})"
    };

    for (const auto& msg : messages) {
        bool result = binance_future_parser_t::parse(now(), msg, listener);
        EXPECT_TRUE(result);
    }

    ASSERT_EQ(listener.book_tickers.size(), 2);
    ASSERT_EQ(listener.agg_trades.size(), 1);

    EXPECT_EQ(listener.book_tickers[0].symbol, "BTCUSDT");
    EXPECT_EQ(listener.agg_trades[0].symbol, "ETHUSDT");
    EXPECT_EQ(listener.book_tickers[1].symbol, "BNBUSDT");
}

TEST_F(binance_future_parser_test_t, AggTradeConsistencyTest) {
    std::string_view message = R"({"e":"aggTrade","E":123456789,"s":"BTCUSDT","a":5933014,"p":"0.001","q":"100","f":100,"l":105,"T":123456785,"m":true})";

    for (int i = 0; i < 100; ++i) {
        listener.reset();
        bool result = binance_future_parser_t::parse(now(), message, listener);

        EXPECT_TRUE(result);
        ASSERT_EQ(listener.agg_trades.size(), 1);

        const auto& trade = listener.agg_trades[0];

        EXPECT_EQ(trade.symbol, "BTCUSDT");
        EXPECT_DOUBLE_EQ(trade.price, 0.001);
        EXPECT_DOUBLE_EQ(trade.quantity, 100.0);
        EXPECT_EQ(trade.agg_trade_id, 5933014ULL);
        EXPECT_TRUE(trade.is_buyer_maker);
    }
}

TEST_F(binance_future_parser_test_t, ProcessAggTradeDirectly) {
    std::string_view message = R"({"e":"aggTrade","E":999999999,"s":"ADAUSDT","a":123456,"p":"0.45","q":"5000","f":100000,"l":100050,"T":999999998,"m":false})";

    bool result = binance_future_parser_t::process_agg_trade(now(), message, listener);

    EXPECT_TRUE(result);
    ASSERT_EQ(listener.agg_trades.size(), 1);

    const auto& trade = listener.agg_trades[0];

    EXPECT_EQ(trade.symbol, "ADAUSDT");
    EXPECT_DOUBLE_EQ(trade.price, 0.45);
    EXPECT_DOUBLE_EQ(trade.quantity, 5000.0);
    EXPECT_FALSE(trade.is_buyer_maker);
}

// ============================================================================
// 24hr Ticker Tests
// ============================================================================

TEST_F(binance_future_parser_test_t, ParseValid24hrTickerMessage) {
    std::string_view message = R"({"e":"24hrTicker","E":123456789,"s":"BTCUSDT","p":"0.0015","P":"250.00","w":"0.0018","c":"0.0025","Q":"10","o":"0.0010","h":"0.0025","l":"0.0010","v":"10000","q":"18","O":0,"C":86400000,"F":0,"L":18150,"n":18151})";

    bool result = binance_future_parser_t::parse(now(), message, listener);

    EXPECT_TRUE(result);
    ASSERT_EQ(listener.tickers.size(), 1);
    EXPECT_EQ(listener.book_tickers.size(), 0);
    EXPECT_EQ(listener.agg_trades.size(), 0);

    const auto& ticker = listener.tickers[0];

    EXPECT_EQ(ticker.symbol, "BTCUSDT");
    EXPECT_EQ(ticker.event_time, 123456789ULL);
    EXPECT_DOUBLE_EQ(ticker.price_change, 0.0015);
    EXPECT_DOUBLE_EQ(ticker.price_change_percent, 250.00);
    EXPECT_DOUBLE_EQ(ticker.weighted_avg_price, 0.0018);
    EXPECT_DOUBLE_EQ(ticker.last_price, 0.0025);
    EXPECT_DOUBLE_EQ(ticker.last_quantity, 10.0);
    EXPECT_DOUBLE_EQ(ticker.open_price, 0.0010);
    EXPECT_DOUBLE_EQ(ticker.high_price, 0.0025);
    EXPECT_DOUBLE_EQ(ticker.low_price, 0.0010);
    EXPECT_DOUBLE_EQ(ticker.total_traded_base_volume, 10000.0);
    EXPECT_DOUBLE_EQ(ticker.total_traded_quote_volume, 18.0);
    EXPECT_EQ(ticker.statistics_open_time, 0ULL);
    EXPECT_EQ(ticker.statistics_close_time, 86400000ULL);
    EXPECT_EQ(ticker.first_trade_id, 0ULL);
    EXPECT_EQ(ticker.last_trade_id, 18150ULL);
    EXPECT_EQ(ticker.total_trades, 18151ULL);
}

TEST_F(binance_future_parser_test_t, ParseTickerWithNormalPrices) {
    std::string_view message = R"({"e":"24hrTicker","E":1234567890,"s":"ETHUSDT","p":"150.50","P":"4.52","w":"3320.75","c":"3500.50","Q":"25.5","o":"3350.00","h":"3600.00","l":"3300.00","v":"125000.5","q":"415000000.25","O":1234467890,"C":1234567890,"F":1000000,"L":1050000,"n":50001})";

    bool result = binance_future_parser_t::parse(now(), message, listener);

    EXPECT_TRUE(result);
    ASSERT_EQ(listener.tickers.size(), 1);

    const auto& ticker = listener.tickers[0];

    EXPECT_EQ(ticker.symbol, "ETHUSDT");
    EXPECT_EQ(ticker.event_time, 1234567890ULL);
    EXPECT_DOUBLE_EQ(ticker.price_change, 150.50);
    EXPECT_DOUBLE_EQ(ticker.price_change_percent, 4.52);
    EXPECT_DOUBLE_EQ(ticker.weighted_avg_price, 3320.75);
    EXPECT_DOUBLE_EQ(ticker.last_price, 3500.50);
    EXPECT_DOUBLE_EQ(ticker.last_quantity, 25.5);
    EXPECT_DOUBLE_EQ(ticker.open_price, 3350.00);
    EXPECT_DOUBLE_EQ(ticker.high_price, 3600.00);
    EXPECT_DOUBLE_EQ(ticker.low_price, 3300.00);
    EXPECT_DOUBLE_EQ(ticker.total_traded_base_volume, 125000.5);
    EXPECT_DOUBLE_EQ(ticker.total_traded_quote_volume, 415000000.25);
    EXPECT_EQ(ticker.statistics_open_time, 1234467890ULL);
    EXPECT_EQ(ticker.statistics_close_time, 1234567890ULL);
    EXPECT_EQ(ticker.first_trade_id, 1000000ULL);
    EXPECT_EQ(ticker.last_trade_id, 1050000ULL);
    EXPECT_EQ(ticker.total_trades, 50001ULL);
}

TEST_F(binance_future_parser_test_t, ParseTickerWithSmallPrices) {
    std::string_view message = R"({"e":"24hrTicker","E":999999999,"s":"DOGEUSDT","p":"0.000012","P":"5.50","w":"0.000220","c":"0.000230","Q":"1000000","o":"0.000218","h":"0.000250","l":"0.000200","v":"5000000000","q":"1100000","O":999899999,"C":999999999,"F":5000000,"L":5100000,"n":100001})";

    bool result = binance_future_parser_t::parse(now(), message, listener);

    EXPECT_TRUE(result);
    ASSERT_EQ(listener.tickers.size(), 1);

    const auto& ticker = listener.tickers[0];

    EXPECT_EQ(ticker.symbol, "DOGEUSDT");
    EXPECT_DOUBLE_EQ(ticker.price_change, 0.000012);
    EXPECT_DOUBLE_EQ(ticker.price_change_percent, 5.50);
    EXPECT_DOUBLE_EQ(ticker.weighted_avg_price, 0.000220);
    EXPECT_DOUBLE_EQ(ticker.last_price, 0.000230);
    EXPECT_DOUBLE_EQ(ticker.last_quantity, 1000000.0);
    EXPECT_DOUBLE_EQ(ticker.open_price, 0.000218);
    EXPECT_DOUBLE_EQ(ticker.high_price, 0.000250);
    EXPECT_DOUBLE_EQ(ticker.low_price, 0.000200);
    EXPECT_DOUBLE_EQ(ticker.total_traded_base_volume, 5000000000.0);
    EXPECT_DOUBLE_EQ(ticker.total_traded_quote_volume, 1100000.0);
}

TEST_F(binance_future_parser_test_t, ParseTickerWithNegativePriceChange) {
    std::string_view message = R"({"e":"24hrTicker","E":888888888,"s":"BNBUSDT","p":"-25.50","P":"-5.12","w":"475.25","c":"472.50","Q":"50","o":"498.00","h":"510.00","l":"470.00","v":"250000","q":"118812500","O":888788888,"C":888888888,"F":2000000,"L":2010000,"n":10001})";

    bool result = binance_future_parser_t::parse(now(), message, listener);

    EXPECT_TRUE(result);
    ASSERT_EQ(listener.tickers.size(), 1);

    const auto& ticker = listener.tickers[0];

    EXPECT_EQ(ticker.symbol, "BNBUSDT");
    EXPECT_DOUBLE_EQ(ticker.price_change, -25.50);
    EXPECT_DOUBLE_EQ(ticker.price_change_percent, -5.12);
    EXPECT_DOUBLE_EQ(ticker.weighted_avg_price, 475.25);
    EXPECT_DOUBLE_EQ(ticker.last_price, 472.50);
    EXPECT_DOUBLE_EQ(ticker.open_price, 498.00);
}

TEST_F(binance_future_parser_test_t, ParseTickerWithIntegerValues) {
    std::string_view message = R"({"e":"24hrTicker","E":777777777,"s":"LINKUSDT","p":"2","P":"10","w":"20","c":"22","Q":"100","o":"20","h":"25","l":"19","v":"1000000","q":"21000000","O":777677777,"C":777777777,"F":3000000,"L":3050000,"n":50001})";

    bool result = binance_future_parser_t::parse(now(), message, listener);

    EXPECT_TRUE(result);
    ASSERT_EQ(listener.tickers.size(), 1);

    const auto& ticker = listener.tickers[0];

    EXPECT_EQ(ticker.symbol, "LINKUSDT");
    EXPECT_DOUBLE_EQ(ticker.price_change, 2.0);
    EXPECT_DOUBLE_EQ(ticker.price_change_percent, 10.0);
    EXPECT_DOUBLE_EQ(ticker.weighted_avg_price, 20.0);
    EXPECT_DOUBLE_EQ(ticker.last_price, 22.0);
    EXPECT_DOUBLE_EQ(ticker.last_quantity, 100.0);
    EXPECT_DOUBLE_EQ(ticker.open_price, 20.0);
    EXPECT_DOUBLE_EQ(ticker.high_price, 25.0);
    EXPECT_DOUBLE_EQ(ticker.low_price, 19.0);
}

TEST_F(binance_future_parser_test_t, ParseTickerWithLargeNumbers) {
    std::string_view message = R"({"e":"24hrTicker","E":9999999999999,"s":"BTCUSDT","p":"5000.50","P":"12.5","w":"45000.75","c":"50000.50","Q":"1.5","o":"45000.00","h":"51000.00","l":"44000.00","v":"50000.25","q":"2250000000.50","O":9999899999999,"C":9999999999999,"F":18446744073709551600,"L":18446744073709551615,"n":18446744073709551615})";

    bool result = binance_future_parser_t::parse(now(), message, listener);

    EXPECT_TRUE(result);
    ASSERT_EQ(listener.tickers.size(), 1);

    const auto& ticker = listener.tickers[0];

    EXPECT_EQ(ticker.symbol, "BTCUSDT");
    EXPECT_EQ(ticker.event_time, 9999999999999ULL);
    EXPECT_DOUBLE_EQ(ticker.price_change, 5000.50);
    EXPECT_DOUBLE_EQ(ticker.total_traded_base_volume, 50000.25);
    EXPECT_DOUBLE_EQ(ticker.total_traded_quote_volume, 2250000000.50);
    EXPECT_EQ(ticker.first_trade_id, 18446744073709551600ULL);
    EXPECT_EQ(ticker.last_trade_id, 18446744073709551615ULL);
    EXPECT_EQ(ticker.total_trades, 18446744073709551615ULL);
}

TEST_F(binance_future_parser_test_t, ParseTickerWithZeroValues) {
    std::string_view message = R"({"e":"24hrTicker","E":111111111,"s":"TESTUSDT","p":"0","P":"0","w":"100","c":"100","Q":"0","o":"100","h":"100","l":"100","v":"0","q":"0","O":111011111,"C":111111111,"F":0,"L":0,"n":1})";

    bool result = binance_future_parser_t::parse(now(), message, listener);

    EXPECT_TRUE(result);
    ASSERT_EQ(listener.tickers.size(), 1);

    const auto& ticker = listener.tickers[0];

    EXPECT_EQ(ticker.symbol, "TESTUSDT");
    EXPECT_DOUBLE_EQ(ticker.price_change, 0.0);
    EXPECT_DOUBLE_EQ(ticker.price_change_percent, 0.0);
    EXPECT_DOUBLE_EQ(ticker.weighted_avg_price, 100.0);
    EXPECT_DOUBLE_EQ(ticker.last_price, 100.0);
    EXPECT_DOUBLE_EQ(ticker.last_quantity, 0.0);
    EXPECT_DOUBLE_EQ(ticker.total_traded_base_volume, 0.0);
    EXPECT_DOUBLE_EQ(ticker.total_traded_quote_volume, 0.0);
    EXPECT_EQ(ticker.first_trade_id, 0ULL);
    EXPECT_EQ(ticker.last_trade_id, 0ULL);
    EXPECT_EQ(ticker.total_trades, 1ULL);
}

TEST_F(binance_future_parser_test_t, ParseMultipleTickers) {
    std::vector<std::string> messages = {
        R"({"e":"24hrTicker","E":1,"s":"A","p":"1","P":"1","w":"1","c":"1","Q":"1","o":"1","h":"1","l":"1","v":"1","q":"1","O":0,"C":1,"F":0,"L":1,"n":2})",
        R"({"e":"24hrTicker","E":2,"s":"B","p":"2","P":"2","w":"2","c":"2","Q":"2","o":"2","h":"2","l":"2","v":"2","q":"2","O":0,"C":2,"F":0,"L":2,"n":3})",
        R"({"e":"24hrTicker","E":3,"s":"C","p":"3","P":"3","w":"3","c":"3","Q":"3","o":"3","h":"3","l":"3","v":"3","q":"3","O":0,"C":3,"F":0,"L":3,"n":4})"
    };

    for (const auto& msg : messages) {
        bool result = binance_future_parser_t::parse(now(), msg, listener);
        EXPECT_TRUE(result);
    }

    ASSERT_EQ(listener.tickers.size(), 3);

    EXPECT_EQ(listener.tickers[0].symbol, "A");
    EXPECT_DOUBLE_EQ(listener.tickers[0].price_change, 1.0);
    EXPECT_EQ(listener.tickers[0].total_trades, 2ULL);

    EXPECT_EQ(listener.tickers[1].symbol, "B");
    EXPECT_DOUBLE_EQ(listener.tickers[1].price_change, 2.0);
    EXPECT_EQ(listener.tickers[1].total_trades, 3ULL);

    EXPECT_EQ(listener.tickers[2].symbol, "C");
    EXPECT_DOUBLE_EQ(listener.tickers[2].price_change, 3.0);
    EXPECT_EQ(listener.tickers[2].total_trades, 4ULL);
}

TEST_F(binance_future_parser_test_t, ParseMixedBookTickerAggTradeAndTicker) {
    std::vector<std::string> messages = {
        R"({"e":"bookTicker","u":1,"s":"BTCUSDT","b":"50000","B":"1","a":"50001","A":"1","T":1,"E":1})",
        R"({"e":"24hrTicker","E":2,"s":"ETHUSDT","p":"100","P":"2.5","w":"4000","c":"4100","Q":"10","o":"4000","h":"4150","l":"3950","v":"100000","q":"400000000","O":0,"C":86400000,"F":0,"L":100000,"n":100001})",
        R"({"e":"aggTrade","E":3,"s":"BNBUSDT","a":3,"p":"500","q":"25","f":3,"l":3,"T":3,"m":true})"
    };

    for (const auto& msg : messages) {
        bool result = binance_future_parser_t::parse(now(), msg, listener);
        EXPECT_TRUE(result);
    }

    ASSERT_EQ(listener.book_tickers.size(), 1);
    ASSERT_EQ(listener.tickers.size(), 1);
    ASSERT_EQ(listener.agg_trades.size(), 1);

    EXPECT_EQ(listener.book_tickers[0].symbol, "BTCUSDT");
    EXPECT_EQ(listener.tickers[0].symbol, "ETHUSDT");
    EXPECT_EQ(listener.agg_trades[0].symbol, "BNBUSDT");

    EXPECT_DOUBLE_EQ(listener.tickers[0].price_change, 100.0);
    EXPECT_DOUBLE_EQ(listener.tickers[0].last_price, 4100.0);
}

TEST_F(binance_future_parser_test_t, TickerConsistencyTest) {
    std::string_view message = R"({"e":"24hrTicker","E":123456789,"s":"BTCUSDT","p":"0.0015","P":"250.00","w":"0.0018","c":"0.0025","Q":"10","o":"0.0010","h":"0.0025","l":"0.0010","v":"10000","q":"18","O":0,"C":86400000,"F":0,"L":18150,"n":18151})";

    for (int i = 0; i < 100; ++i) {
        listener.reset();
        bool result = binance_future_parser_t::parse(now(), message, listener);

        EXPECT_TRUE(result);
        ASSERT_EQ(listener.tickers.size(), 1);

        const auto& ticker = listener.tickers[0];

        EXPECT_EQ(ticker.symbol, "BTCUSDT");
        EXPECT_EQ(ticker.event_time, 123456789ULL);
        EXPECT_DOUBLE_EQ(ticker.price_change, 0.0015);
        EXPECT_DOUBLE_EQ(ticker.price_change_percent, 250.00);
        EXPECT_DOUBLE_EQ(ticker.last_price, 0.0025);
        EXPECT_EQ(ticker.total_trades, 18151ULL);
    }
}

TEST_F(binance_future_parser_test_t, ProcessTickerDirectly) {
    std::string_view message = R"({"e":"24hrTicker","E":555555555,"s":"ADAUSDT","p":"0.05","P":"8.33","w":"0.6","c":"0.65","Q":"1000","o":"0.60","h":"0.70","l":"0.55","v":"10000000","q":"6000000","O":555455555,"C":555555555,"F":500000,"L":550000,"n":50001})";

    bool result = binance_future_parser_t::process_ticker(now(), message, listener);

    EXPECT_TRUE(result);
    ASSERT_EQ(listener.tickers.size(), 1);

    const auto& ticker = listener.tickers[0];

    EXPECT_EQ(ticker.symbol, "ADAUSDT");
    EXPECT_DOUBLE_EQ(ticker.price_change, 0.05);
    EXPECT_DOUBLE_EQ(ticker.price_change_percent, 8.33);
    EXPECT_DOUBLE_EQ(ticker.weighted_avg_price, 0.6);
    EXPECT_DOUBLE_EQ(ticker.last_price, 0.65);
    EXPECT_DOUBLE_EQ(ticker.last_quantity, 1000.0);
    EXPECT_EQ(ticker.total_trades, 50001ULL);
}

// ============================================================================
// 24hr Ticker Array Tests
// ============================================================================

TEST_F(binance_future_parser_test_t, ParseTickerArrayWithMultipleTickers) {
    std::string_view message = R"([{"e":"24hrTicker","E":123456789,"s":"BTCUSDT","p":"100.50","P":"0.5","w":"50000","c":"50100","Q":"1.5","o":"50000","h":"51000","l":"49000","v":"10000","q":"500000000","O":123356789,"C":123456789,"F":1000,"L":2000,"n":1001},{"e":"24hrTicker","E":123456790,"s":"ETHUSDT","p":"50.25","P":"1.5","w":"3500","c":"3550","Q":"10","o":"3500","h":"3600","l":"3400","v":"50000","q":"175000000","O":123356790,"C":123456790,"F":2000,"L":3000,"n":1001}])";

    bool result = binance_future_parser_t::parse(now(), message, listener);

    EXPECT_TRUE(result);
    ASSERT_EQ(listener.tickers.size(), 2);
    EXPECT_EQ(listener.book_tickers.size(), 0);
    EXPECT_EQ(listener.agg_trades.size(), 0);

    // First ticker - BTCUSDT
    const auto& ticker1 = listener.tickers[0];
    EXPECT_EQ(ticker1.symbol, "BTCUSDT");
    EXPECT_EQ(ticker1.event_time, 123456789ULL);
    EXPECT_DOUBLE_EQ(ticker1.price_change, 100.50);
    EXPECT_DOUBLE_EQ(ticker1.price_change_percent, 0.5);
    EXPECT_DOUBLE_EQ(ticker1.last_price, 50100.0);
    EXPECT_EQ(ticker1.total_trades, 1001ULL);

    // Second ticker - ETHUSDT
    const auto& ticker2 = listener.tickers[1];
    EXPECT_EQ(ticker2.symbol, "ETHUSDT");
    EXPECT_EQ(ticker2.event_time, 123456790ULL);
    EXPECT_DOUBLE_EQ(ticker2.price_change, 50.25);
    EXPECT_DOUBLE_EQ(ticker2.price_change_percent, 1.5);
    EXPECT_DOUBLE_EQ(ticker2.last_price, 3550.0);
    EXPECT_EQ(ticker2.total_trades, 1001ULL);
}

TEST_F(binance_future_parser_test_t, ParseTickerArrayWithSingleTicker) {
    std::string_view message = R"([{"e":"24hrTicker","E":999999999,"s":"BNBUSDT","p":"5.00","P":"1.0","w":"500","c":"505","Q":"100","o":"500","h":"510","l":"490","v":"100000","q":"50000000","O":999899999,"C":999999999,"F":5000,"L":6000,"n":1001}])";

    bool result = binance_future_parser_t::parse(now(), message, listener);

    EXPECT_TRUE(result);
    ASSERT_EQ(listener.tickers.size(), 1);

    const auto& ticker = listener.tickers[0];
    EXPECT_EQ(ticker.symbol, "BNBUSDT");
    EXPECT_EQ(ticker.event_time, 999999999ULL);
    EXPECT_DOUBLE_EQ(ticker.price_change, 5.00);
    EXPECT_DOUBLE_EQ(ticker.last_price, 505.0);
    EXPECT_EQ(ticker.total_trades, 1001ULL);
}

TEST_F(binance_future_parser_test_t, ProcessTickerArrayDirectly) {
    std::string_view message = R"([{"e":"24hrTicker","E":111111111,"s":"ADAUSDT","p":"0.01","P":"2.0","w":"0.5","c":"0.51","Q":"5000","o":"0.50","h":"0.55","l":"0.48","v":"1000000000","q":"500000000","O":111011111,"C":111111111,"F":100000,"L":200000,"n":100001},{"e":"24hrTicker","E":222222222,"s":"DOTUSDT","p":"1.5","P":"3.0","w":"50","c":"51.5","Q":"200","o":"50","h":"53","l":"48","v":"5000000","q":"250000000","O":222122222,"C":222222222,"F":300000,"L":400000,"n":100001}])";

    bool result = binance_future_parser_t::process_ticker_array(now(), message, listener);

    EXPECT_TRUE(result);
    ASSERT_EQ(listener.tickers.size(), 2);

    EXPECT_EQ(listener.tickers[0].symbol, "ADAUSDT");
    EXPECT_DOUBLE_EQ(listener.tickers[0].price_change, 0.01);
    EXPECT_DOUBLE_EQ(listener.tickers[0].last_price, 0.51);

    EXPECT_EQ(listener.tickers[1].symbol, "DOTUSDT");
    EXPECT_DOUBLE_EQ(listener.tickers[1].price_change, 1.5);
    EXPECT_DOUBLE_EQ(listener.tickers[1].last_price, 51.5);
}

TEST_F(binance_future_parser_test_t, ParseTickerArrayWithThreeTickers) {
    std::string_view message = R"([{"e":"24hrTicker","E":1,"s":"A","p":"1","P":"1","w":"1","c":"1","Q":"1","o":"1","h":"1","l":"1","v":"1","q":"1","O":0,"C":1,"F":0,"L":1,"n":2},{"e":"24hrTicker","E":2,"s":"B","p":"2","P":"2","w":"2","c":"2","Q":"2","o":"2","h":"2","l":"2","v":"2","q":"2","O":0,"C":2,"F":0,"L":2,"n":3},{"e":"24hrTicker","E":3,"s":"C","p":"3","P":"3","w":"3","c":"3","Q":"3","o":"3","h":"3","l":"3","v":"3","q":"3","O":0,"C":3,"F":0,"L":3,"n":4}])";

    bool result = binance_future_parser_t::parse(now(), message, listener);

    EXPECT_TRUE(result);
    ASSERT_EQ(listener.tickers.size(), 3);

    EXPECT_EQ(listener.tickers[0].symbol, "A");
    EXPECT_DOUBLE_EQ(listener.tickers[0].price_change, 1.0);
    EXPECT_EQ(listener.tickers[0].total_trades, 2ULL);

    EXPECT_EQ(listener.tickers[1].symbol, "B");
    EXPECT_DOUBLE_EQ(listener.tickers[1].price_change, 2.0);
    EXPECT_EQ(listener.tickers[1].total_trades, 3ULL);

    EXPECT_EQ(listener.tickers[2].symbol, "C");
    EXPECT_DOUBLE_EQ(listener.tickers[2].price_change, 3.0);
    EXPECT_EQ(listener.tickers[2].total_trades, 4ULL);
}

TEST_F(binance_future_parser_test_t, ParseTickerArrayWithNegativePriceChange) {
    std::string_view message = R"([{"e":"24hrTicker","E":888888888,"s":"BTCUSDT","p":"-500.00","P":"-1.0","w":"49750","c":"49500","Q":"0.5","o":"50000","h":"50500","l":"49000","v":"25000","q":"1243750000","O":888788888,"C":888888888,"F":1000000,"L":1100000,"n":100001}])";

    bool result = binance_future_parser_t::parse(now(), message, listener);

    EXPECT_TRUE(result);
    ASSERT_EQ(listener.tickers.size(), 1);

    const auto& ticker = listener.tickers[0];
    EXPECT_EQ(ticker.symbol, "BTCUSDT");
    EXPECT_DOUBLE_EQ(ticker.price_change, -500.00);
    EXPECT_DOUBLE_EQ(ticker.price_change_percent, -1.0);
    EXPECT_DOUBLE_EQ(ticker.last_price, 49500.0);
    EXPECT_DOUBLE_EQ(ticker.open_price, 50000.0);
}

TEST_F(binance_future_parser_test_t, TickerArrayConsistencyTest) {
    std::string_view message = R"([{"e":"24hrTicker","E":123456789,"s":"BTCUSDT","p":"100","P":"0.2","w":"50000","c":"50100","Q":"1","o":"50000","h":"51000","l":"49000","v":"10000","q":"500000000","O":123356789,"C":123456789,"F":1000,"L":2000,"n":1001},{"e":"24hrTicker","E":123456790,"s":"ETHUSDT","p":"50","P":"1.5","w":"3500","c":"3550","Q":"10","o":"3500","h":"3600","l":"3400","v":"50000","q":"175000000","O":123356790,"C":123456790,"F":2000,"L":3000,"n":1001}])";

    for (int i = 0; i < 100; ++i) {
        listener.reset();
        bool result = binance_future_parser_t::parse(now(), message, listener);

        EXPECT_TRUE(result);
        ASSERT_EQ(listener.tickers.size(), 2);

        EXPECT_EQ(listener.tickers[0].symbol, "BTCUSDT");
        EXPECT_DOUBLE_EQ(listener.tickers[0].price_change, 100.0);
        EXPECT_EQ(listener.tickers[0].total_trades, 1001ULL);

        EXPECT_EQ(listener.tickers[1].symbol, "ETHUSDT");
        EXPECT_DOUBLE_EQ(listener.tickers[1].price_change, 50.0);
        EXPECT_EQ(listener.tickers[1].total_trades, 1001ULL);
    }
}

TEST_F(binance_future_parser_test_t, ParseEmptyTickerArray) {
    std::string_view message = R"([])";

    bool result = binance_future_parser_t::process_ticker_array(now(), message, listener);

    EXPECT_TRUE(result);
    EXPECT_EQ(listener.tickers.size(), 0);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}