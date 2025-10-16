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
    std::vector<agg_trade_t> agg_trades;

    void on_book_ticker(const book_ticker_t& ticker) {
        book_tickers.push_back(ticker);
    }

    void on_agg_trade(const agg_trade_t& trade) {
        agg_trades.push_back(trade);
    }

    void reset() {
        book_tickers.clear();
        agg_trades.clear();
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

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}