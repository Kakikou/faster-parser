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

using namespace core::binance;
using namespace core::binance::types;

class MockListener {
public:
    std::vector<book_ticker_t> book_tickers;

    void on_book_ticker(const book_ticker_t& ticker) {
        book_tickers.push_back(ticker);
    }

    void reset() {
        book_tickers.clear();
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

    EXPECT_EQ(listener.book_tickers[0].bid.sequence, 1ULL);
    EXPECT_DOUBLE_EQ(listener.book_tickers[0].bid.price, 1.0);
    EXPECT_EQ(listener.book_tickers[1].bid.sequence, 2ULL);
    EXPECT_DOUBLE_EQ(listener.book_tickers[1].bid.price, 2.0);
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

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}