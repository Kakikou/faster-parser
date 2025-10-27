/**
 * @file future_benchmark_comparison.cpp
 * @author Kevin Rodrigues
 * @brief Comparison benchmark between faster-parser and simdjson
 * @version 1.0
 * @date 10/10/2025
 */

#include <chrono>
#include <string>
#include <vector>
#include <benchmark/benchmark.h>
#include <simdjson.h>

#include <faster_parser/binance/future.h>

using namespace core::faster_parser::binance;
using namespace core::faster_parser::binance::types;

class BenchmarkListener {
public:
    book_ticker_t last_ticker;
    trade_t last_trade;

    void on_book_ticker(const book_ticker_t& ticker) {
        last_ticker = ticker;
    }

    void on_trade(const trade_t& trade) {
        last_trade = trade;
    }
};

// Test messages
const std::vector<std::string> book_ticker_messages = {
    R"({"e":"bookTicker","u":8822354685185,"s":"ASTERUSDT","b":"1.5822000","B":"457","a":"1.5823000","A":"112","T":1760083106579,"E":1760083106579})",
    R"({"e":"bookTicker","u":123456789,"s":"BTCUSDT","b":"45123.78900000","B":"10.5","a":"45124.12300000","A":"5.25","T":1234567890123,"E":1234567890123})",
    R"({"e":"bookTicker","u":999999,"s":"DOGEUSDT","b":"0.00012345","B":"1000000","a":"0.00012346","A":"999999","T":9999999999,"E":9999999999})",
    R"({"e":"bookTicker","u":111111111,"s":"ETHUSDT","b":"3000","B":"100","a":"3001","A":"200","T":1111111111111,"E":1111111111111})",
};

const std::vector<std::string> agg_trade_messages = {
    R"({"e":"aggTrade","E":123456789,"s":"BTCUSDT","a":5933014,"p":"0.001","q":"100","f":100,"l":105,"T":123456785,"m":true})",
    R"({"e":"aggTrade","E":987654321,"s":"ETHUSDT","a":8888888,"p":"3500.50","q":"10.5","f":200,"l":210,"T":987654320,"m":false})",
    R"({"e":"aggTrade","E":111111111,"s":"DOGEUSDT","a":99999,"p":"0.00012345","q":"1000000","f":50000,"l":50010,"T":111111110,"m":true})",
    R"({"e":"aggTrade","E":222222222,"s":"BNBUSDT","a":777777,"p":"500","q":"25","f":1000,"l":1005,"T":222222221,"m":true})",
};

const std::vector<std::string> mixed_messages = {
    R"({"e":"bookTicker","u":8822354685185,"s":"ASTERUSDT","b":"1.5822000","B":"457","a":"1.5823000","A":"112","T":1760083106579,"E":1760083106579})",
    R"({"e":"aggTrade","E":123456789,"s":"BTCUSDT","a":5933014,"p":"0.001","q":"100","f":100,"l":105,"T":123456785,"m":true})",
    R"({"e":"bookTicker","u":123456789,"s":"BTCUSDT","b":"45123.78900000","B":"10.5","a":"45124.12300000","A":"5.25","T":1234567890123,"E":1234567890123})",
    R"({"e":"aggTrade","E":987654321,"s":"ETHUSDT","a":8888888,"p":"3500.50","q":"10.5","f":200,"l":210,"T":987654320,"m":false})",
    R"({"e":"bookTicker","u":999999,"s":"DOGEUSDT","b":"0.00012345","B":"1000000","a":"0.00012346","A":"999999","T":9999999999,"E":9999999999})",
    R"({"e":"aggTrade","E":111111111,"s":"DOGEUSDT","a":99999,"p":"0.00012345","q":"1000000","f":50000,"l":50010,"T":111111110,"m":true})",
};

// ============================================================================
// faster-parser benchmarks
// ============================================================================

static void bm_faster_parser_single_message(benchmark::State &state) {
    BenchmarkListener listener;
    auto now = std::chrono::system_clock::now();
    std::string_view message = book_ticker_messages[0];

    for (auto _ : state) {
        bool result = binance_future_parser_t::parse(now, message, listener);
        benchmark::DoNotOptimize(result);
        benchmark::DoNotOptimize(listener.last_ticker);
    }

    state.SetBytesProcessed(state.iterations() * message.size());
}

static void bm_faster_parser_mixed_workload(benchmark::State &state) {
    BenchmarkListener listener;
    auto now = std::chrono::system_clock::now();
    size_t index = 0;
    size_t total_bytes = 0;

    for (auto _ : state) {
        const auto& message = mixed_messages[index % mixed_messages.size()];
        bool result = binance_future_parser_t::parse(now, message, listener);
        benchmark::DoNotOptimize(result);
        benchmark::DoNotOptimize(listener.last_ticker);
        benchmark::DoNotOptimize(listener.last_trade);
        total_bytes += message.size();
        ++index;
    }

    state.SetBytesProcessed(total_bytes);
}

static void bm_faster_parser_btc_high_frequency(benchmark::State &state) {
    BenchmarkListener listener;
    auto now = std::chrono::system_clock::now();
    std::string_view message = book_ticker_messages[1];

    for (auto _ : state) {
        bool result = binance_future_parser_t::parse(now, message, listener);
        benchmark::DoNotOptimize(result);
        benchmark::DoNotOptimize(listener.last_ticker);
    }

    state.SetBytesProcessed(state.iterations() * message.size());
}

// ============================================================================
// simdjson benchmarks
// ============================================================================

static void bm_simdjson_single_message(benchmark::State &state) {
    simdjson::ondemand::parser parser;
    std::string_view message = book_ticker_messages[0];
    book_ticker_t ticker;
    auto now = std::chrono::system_clock::now();

    for (auto _ : state) {
        simdjson::padded_string padded(message);
        auto doc = parser.iterate(padded);

        // Parse all fields
        std::string_view event_type = doc["e"].get_string().value();
        ticker.bid.sequence = doc["u"].get_uint64().value();
        ticker.ask.sequence = ticker.bid.sequence;

        // Skip symbol
        std::string_view symbol = doc["s"].get_string().value();

        // Parse prices and volumes
        std::string_view bid_price_str = doc["b"].get_string().value();
        ticker.bid.price = std::stod(std::string(bid_price_str));

        std::string_view bid_volume_str = doc["B"].get_string().value();
        ticker.bid.volume = std::stod(std::string(bid_volume_str));

        std::string_view ask_price_str = doc["a"].get_string().value();
        ticker.ask.price = std::stod(std::string(ask_price_str));

        std::string_view ask_volume_str = doc["A"].get_string().value();
        ticker.ask.volume = std::stod(std::string(ask_volume_str));

        ticker.exchange_timestamp = doc["E"].get_uint64().value();
        ticker.time = now;

        benchmark::DoNotOptimize(ticker);
    }

    state.SetBytesProcessed(state.iterations() * message.size());
}

static void bm_simdjson_mixed_workload(benchmark::State &state) {
    simdjson::ondemand::parser parser;
    book_ticker_t ticker;
    trade_t trade;
    auto now = std::chrono::system_clock::now();
    size_t index = 0;
    size_t total_bytes = 0;

    for (auto _ : state) {
        const auto& message = mixed_messages[index % mixed_messages.size()];
        simdjson::padded_string padded(message);
        auto doc = parser.iterate(padded);

        std::string_view event_type = doc["e"].get_string().value();

        if (event_type == "bookTicker") {
            ticker.bid.sequence = doc["u"].get_uint64().value();
            ticker.ask.sequence = ticker.bid.sequence;

            std::string_view symbol = doc["s"].get_string().value();

            std::string_view bid_price_str = doc["b"].get_string().value();
            ticker.bid.price = std::stod(std::string(bid_price_str));

            std::string_view bid_volume_str = doc["B"].get_string().value();
            ticker.bid.volume = std::stod(std::string(bid_volume_str));

            std::string_view ask_price_str = doc["a"].get_string().value();
            ticker.ask.price = std::stod(std::string(ask_price_str));

            std::string_view ask_volume_str = doc["A"].get_string().value();
            ticker.ask.volume = std::stod(std::string(ask_volume_str));

            ticker.exchange_timestamp = doc["E"].get_uint64().value();
            ticker.time = now;

            benchmark::DoNotOptimize(ticker);
        } else if (event_type == "aggTrade") {
            trade.event_time = doc["E"].get_uint64().value();
            trade.symbol = doc["s"].get_string().value();
            trade.agg_trade_id = doc["a"].get_uint64().value();

            std::string_view price_str = doc["p"].get_string().value();
            trade.price = std::stod(std::string(price_str));

            std::string_view qty_str = doc["q"].get_string().value();
            trade.quantity = std::stod(std::string(qty_str));

            trade.first_trade_id = doc["f"].get_uint64().value();
            trade.last_trade_id = doc["l"].get_uint64().value();
            trade.trade_time = doc["T"].get_uint64().value();
            trade.is_buyer_maker = doc["m"].get_bool().value();
            trade.time = now;

            benchmark::DoNotOptimize(trade);
        }

        total_bytes += message.size();
        ++index;
    }

    state.SetBytesProcessed(total_bytes);
}

static void bm_simdjson_btc_high_frequency(benchmark::State &state) {
    simdjson::ondemand::parser parser;
    std::string_view message = book_ticker_messages[1];
    book_ticker_t ticker;
    auto now = std::chrono::system_clock::now();

    for (auto _ : state) {
        simdjson::padded_string padded(message);
        auto doc = parser.iterate(padded);

        std::string_view event_type = doc["e"].get_string().value();
        ticker.bid.sequence = doc["u"].get_uint64().value();
        ticker.ask.sequence = ticker.bid.sequence;

        std::string_view symbol = doc["s"].get_string().value();

        std::string_view bid_price_str = doc["b"].get_string().value();
        ticker.bid.price = std::stod(std::string(bid_price_str));

        std::string_view bid_volume_str = doc["B"].get_string().value();
        ticker.bid.volume = std::stod(std::string(bid_volume_str));

        std::string_view ask_price_str = doc["a"].get_string().value();
        ticker.ask.price = std::stod(std::string(ask_price_str));

        std::string_view ask_volume_str = doc["A"].get_string().value();
        ticker.ask.volume = std::stod(std::string(ask_volume_str));

        ticker.exchange_timestamp = doc["E"].get_uint64().value();
        ticker.time = now;

        benchmark::DoNotOptimize(ticker);
    }

    state.SetBytesProcessed(state.iterations() * message.size());
}

// ============================================================================
// Aggregate Trade Comparison Benchmarks
// ============================================================================

static void bm_faster_parser_agg_trade_single(benchmark::State &state) {
    BenchmarkListener listener;
    auto now = std::chrono::system_clock::now();
    std::string_view message = agg_trade_messages[0];

    for (auto _ : state) {
        bool result = binance_future_parser_t::parse(now, message, listener);
        benchmark::DoNotOptimize(result);
        benchmark::DoNotOptimize(listener.last_trade);
    }

    state.SetBytesProcessed(state.iterations() * message.size());
}

static void bm_simdjson_agg_trade_single(benchmark::State &state) {
    simdjson::ondemand::parser parser;
    std::string_view message = agg_trade_messages[0];
    trade_t trade;
    auto now = std::chrono::system_clock::now();

    for (auto _ : state) {
        simdjson::padded_string padded(message);
        auto doc = parser.iterate(padded);

        std::string_view event_type = doc["e"].get_string().value();
        trade.event_time = doc["E"].get_uint64().value();
        trade.symbol = doc["s"].get_string().value();
        trade.agg_trade_id = doc["a"].get_uint64().value();

        std::string_view price_str = doc["p"].get_string().value();
        trade.price = std::stod(std::string(price_str));

        std::string_view qty_str = doc["q"].get_string().value();
        trade.quantity = std::stod(std::string(qty_str));

        trade.first_trade_id = doc["f"].get_uint64().value();
        trade.last_trade_id = doc["l"].get_uint64().value();
        trade.trade_time = doc["T"].get_uint64().value();
        trade.is_buyer_maker = doc["m"].get_bool().value();
        trade.time = now;

        benchmark::DoNotOptimize(trade);
    }

    state.SetBytesProcessed(state.iterations() * message.size());
}

static void bm_faster_parser_agg_trade_workload(benchmark::State &state) {
    BenchmarkListener listener;
    auto now = std::chrono::system_clock::now();
    size_t index = 0;
    size_t total_bytes = 0;

    for (auto _ : state) {
        const auto& message = agg_trade_messages[index % agg_trade_messages.size()];
        bool result = binance_future_parser_t::parse(now, message, listener);
        benchmark::DoNotOptimize(result);
        benchmark::DoNotOptimize(listener.last_trade);
        total_bytes += message.size();
        ++index;
    }

    state.SetBytesProcessed(total_bytes);
}

static void bm_simdjson_agg_trade_workload(benchmark::State &state) {
    simdjson::ondemand::parser parser;
    trade_t trade;
    auto now = std::chrono::system_clock::now();
    size_t index = 0;
    size_t total_bytes = 0;

    for (auto _ : state) {
        const auto& message = agg_trade_messages[index % agg_trade_messages.size()];
        simdjson::padded_string padded(message);
        auto doc = parser.iterate(padded);

        std::string_view event_type = doc["e"].get_string().value();
        trade.event_time = doc["E"].get_uint64().value();
        trade.symbol = doc["s"].get_string().value();
        trade.agg_trade_id = doc["a"].get_uint64().value();

        std::string_view price_str = doc["p"].get_string().value();
        trade.price = std::stod(std::string(price_str));

        std::string_view qty_str = doc["q"].get_string().value();
        trade.quantity = std::stod(std::string(qty_str));

        trade.first_trade_id = doc["f"].get_uint64().value();
        trade.last_trade_id = doc["l"].get_uint64().value();
        trade.trade_time = doc["T"].get_uint64().value();
        trade.is_buyer_maker = doc["m"].get_bool().value();
        trade.time = now;

        benchmark::DoNotOptimize(trade);
        total_bytes += message.size();
        ++index;
    }

    state.SetBytesProcessed(total_bytes);
}

// ============================================================================
// Register benchmarks
// ============================================================================

// BookTicker: faster-parser vs simdjson
BENCHMARK(bm_faster_parser_single_message);
BENCHMARK(bm_simdjson_single_message);
BENCHMARK(bm_faster_parser_btc_high_frequency);
BENCHMARK(bm_simdjson_btc_high_frequency);

// AggTrade: faster-parser vs simdjson
BENCHMARK(bm_faster_parser_agg_trade_single);
BENCHMARK(bm_simdjson_agg_trade_single);
BENCHMARK(bm_faster_parser_agg_trade_workload);
BENCHMARK(bm_simdjson_agg_trade_workload);

// Mixed workload: faster-parser vs simdjson
BENCHMARK(bm_faster_parser_mixed_workload);
BENCHMARK(bm_simdjson_mixed_workload);

BENCHMARK_MAIN();
