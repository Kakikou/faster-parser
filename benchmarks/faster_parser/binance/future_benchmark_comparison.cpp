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
    book_ticker_t last_book_ticker;
    trade_t last_trade;
    ticker_t last_ticker;

    void on_book_ticker(const book_ticker_t& ticker) {
        last_book_ticker = ticker;
    }

    void on_trade(const trade_t& trade) {
        last_trade = trade;
    }

    void on_ticker(const ticker_t& ticker) {
        last_ticker = ticker;
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

const std::vector<std::string> ticker_messages = {
    R"({"e":"24hrTicker","E":123456789,"s":"BTCUSDT","p":"0.0015","P":"250.00","w":"0.0018","c":"0.0025","Q":"10","o":"0.0010","h":"0.0025","l":"0.0010","v":"10000","q":"18","O":0,"C":86400000,"F":0,"L":18150,"n":18151})",
    R"({"e":"24hrTicker","E":1234567890,"s":"ETHUSDT","p":"150.50","P":"4.52","w":"3320.75","c":"3500.50","Q":"25.5","o":"3350.00","h":"3600.00","l":"3300.00","v":"125000.5","q":"415000000.25","O":1234467890,"C":1234567890,"F":1000000,"L":1050000,"n":50001})",
    R"({"e":"24hrTicker","E":999999999,"s":"DOGEUSDT","p":"0.000012","P":"5.50","w":"0.000220","c":"0.000230","Q":"1000000","o":"0.000218","h":"0.000250","l":"0.000200","v":"5000000000","q":"1100000","O":999899999,"C":999999999,"F":5000000,"L":5100000,"n":100001})",
    R"({"e":"24hrTicker","E":888888888,"s":"BNBUSDT","p":"-25.50","P":"-5.12","w":"475.25","c":"472.50","Q":"50","o":"498.00","h":"510.00","l":"470.00","v":"250000","q":"118812500","O":888788888,"C":888888888,"F":2000000,"L":2010000,"n":10001})",
};

const std::vector<std::string> mixed_messages = {
    R"({"e":"bookTicker","u":8822354685185,"s":"ASTERUSDT","b":"1.5822000","B":"457","a":"1.5823000","A":"112","T":1760083106579,"E":1760083106579})",
    R"({"e":"aggTrade","E":123456789,"s":"BTCUSDT","a":5933014,"p":"0.001","q":"100","f":100,"l":105,"T":123456785,"m":true})",
    R"({"e":"24hrTicker","E":123456789,"s":"BTCUSDT","p":"0.0015","P":"250.00","w":"0.0018","c":"0.0025","Q":"10","o":"0.0010","h":"0.0025","l":"0.0010","v":"10000","q":"18","O":0,"C":86400000,"F":0,"L":18150,"n":18151})",
    R"({"e":"bookTicker","u":123456789,"s":"BTCUSDT","b":"45123.78900000","B":"10.5","a":"45124.12300000","A":"5.25","T":1234567890123,"E":1234567890123})",
    R"({"e":"aggTrade","E":987654321,"s":"ETHUSDT","a":8888888,"p":"3500.50","q":"10.5","f":200,"l":210,"T":987654320,"m":false})",
    R"({"e":"24hrTicker","E":1234567890,"s":"ETHUSDT","p":"150.50","P":"4.52","w":"3320.75","c":"3500.50","Q":"25.5","o":"3350.00","h":"3600.00","l":"3300.00","v":"125000.5","q":"415000000.25","O":1234467890,"C":1234567890,"F":1000000,"L":1050000,"n":50001})",
    R"({"e":"bookTicker","u":999999,"s":"DOGEUSDT","b":"0.00012345","B":"1000000","a":"0.00012346","A":"999999","T":9999999999,"E":9999999999})",
    R"({"e":"aggTrade","E":111111111,"s":"DOGEUSDT","a":99999,"p":"0.00012345","q":"1000000","f":50000,"l":50010,"T":111111110,"m":true})",
    R"({"e":"24hrTicker","E":999999999,"s":"DOGEUSDT","p":"0.000012","P":"5.50","w":"0.000220","c":"0.000230","Q":"1000000","o":"0.000218","h":"0.000250","l":"0.000200","v":"5000000000","q":"1100000","O":999899999,"C":999999999,"F":5000000,"L":5100000,"n":100001})",
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
        benchmark::DoNotOptimize(listener.last_book_ticker);
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
        benchmark::DoNotOptimize(listener.last_book_ticker);
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
        benchmark::DoNotOptimize(listener.last_book_ticker);
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
    book_ticker_t book_ticker;
    trade_t trade;
    ticker_t ticker;
    auto now = std::chrono::system_clock::now();
    size_t index = 0;
    size_t total_bytes = 0;

    for (auto _ : state) {
        const auto& message = mixed_messages[index % mixed_messages.size()];
        simdjson::padded_string padded(message);
        auto doc = parser.iterate(padded);

        std::string_view event_type = doc["e"].get_string().value();

        if (event_type == "bookTicker") {
            book_ticker.bid.sequence = doc["u"].get_uint64().value();
            book_ticker.ask.sequence = book_ticker.bid.sequence;

            std::string_view symbol = doc["s"].get_string().value();

            std::string_view bid_price_str = doc["b"].get_string().value();
            book_ticker.bid.price = std::stod(std::string(bid_price_str));

            std::string_view bid_volume_str = doc["B"].get_string().value();
            book_ticker.bid.volume = std::stod(std::string(bid_volume_str));

            std::string_view ask_price_str = doc["a"].get_string().value();
            book_ticker.ask.price = std::stod(std::string(ask_price_str));

            std::string_view ask_volume_str = doc["A"].get_string().value();
            book_ticker.ask.volume = std::stod(std::string(ask_volume_str));

            book_ticker.exchange_timestamp = doc["E"].get_uint64().value();
            book_ticker.time = now;

            benchmark::DoNotOptimize(book_ticker);
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
        } else if (event_type == "24hrTicker") {
            ticker.event_time = doc["E"].get_uint64().value();
            ticker.symbol = doc["s"].get_string().value();

            std::string_view p_str = doc["p"].get_string().value();
            ticker.price_change = std::stod(std::string(p_str));

            std::string_view P_str = doc["P"].get_string().value();
            ticker.price_change_percent = std::stod(std::string(P_str));

            std::string_view w_str = doc["w"].get_string().value();
            ticker.weighted_avg_price = std::stod(std::string(w_str));

            std::string_view c_str = doc["c"].get_string().value();
            ticker.last_price = std::stod(std::string(c_str));

            std::string_view Q_str = doc["Q"].get_string().value();
            ticker.last_quantity = std::stod(std::string(Q_str));

            std::string_view o_str = doc["o"].get_string().value();
            ticker.open_price = std::stod(std::string(o_str));

            std::string_view h_str = doc["h"].get_string().value();
            ticker.high_price = std::stod(std::string(h_str));

            std::string_view l_str = doc["l"].get_string().value();
            ticker.low_price = std::stod(std::string(l_str));

            std::string_view v_str = doc["v"].get_string().value();
            ticker.total_traded_base_volume = std::stod(std::string(v_str));

            std::string_view q_str = doc["q"].get_string().value();
            ticker.total_traded_quote_volume = std::stod(std::string(q_str));

            ticker.statistics_open_time = doc["O"].get_uint64().value();
            ticker.statistics_close_time = doc["C"].get_uint64().value();
            ticker.first_trade_id = doc["F"].get_uint64().value();
            ticker.last_trade_id = doc["L"].get_uint64().value();
            ticker.total_trades = doc["n"].get_uint64().value();

            ticker.time = now;

            benchmark::DoNotOptimize(ticker);
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
// 24hr Ticker Comparison Benchmarks
// ============================================================================

static void bm_faster_parser_ticker_single(benchmark::State &state) {
    BenchmarkListener listener;
    auto now = std::chrono::system_clock::now();
    std::string_view message = ticker_messages[0];

    for (auto _ : state) {
        bool result = binance_future_parser_t::parse(now, message, listener);
        benchmark::DoNotOptimize(result);
        benchmark::DoNotOptimize(listener.last_ticker);
    }

    state.SetBytesProcessed(state.iterations() * message.size());
}

static void bm_simdjson_ticker_single(benchmark::State &state) {
    simdjson::ondemand::parser parser;
    std::string_view message = ticker_messages[0];
    ticker_t ticker;
    auto now = std::chrono::system_clock::now();

    for (auto _ : state) {
        simdjson::padded_string padded(message);
        auto doc = parser.iterate(padded);

        std::string_view event_type = doc["e"].get_string().value();
        ticker.event_time = doc["E"].get_uint64().value();
        ticker.symbol = doc["s"].get_string().value();

        std::string_view p_str = doc["p"].get_string().value();
        ticker.price_change = std::stod(std::string(p_str));

        std::string_view P_str = doc["P"].get_string().value();
        ticker.price_change_percent = std::stod(std::string(P_str));

        std::string_view w_str = doc["w"].get_string().value();
        ticker.weighted_avg_price = std::stod(std::string(w_str));

        std::string_view c_str = doc["c"].get_string().value();
        ticker.last_price = std::stod(std::string(c_str));

        std::string_view Q_str = doc["Q"].get_string().value();
        ticker.last_quantity = std::stod(std::string(Q_str));

        std::string_view o_str = doc["o"].get_string().value();
        ticker.open_price = std::stod(std::string(o_str));

        std::string_view h_str = doc["h"].get_string().value();
        ticker.high_price = std::stod(std::string(h_str));

        std::string_view l_str = doc["l"].get_string().value();
        ticker.low_price = std::stod(std::string(l_str));

        std::string_view v_str = doc["v"].get_string().value();
        ticker.total_traded_base_volume = std::stod(std::string(v_str));

        std::string_view q_str = doc["q"].get_string().value();
        ticker.total_traded_quote_volume = std::stod(std::string(q_str));

        ticker.statistics_open_time = doc["O"].get_uint64().value();
        ticker.statistics_close_time = doc["C"].get_uint64().value();
        ticker.first_trade_id = doc["F"].get_uint64().value();
        ticker.last_trade_id = doc["L"].get_uint64().value();
        ticker.total_trades = doc["n"].get_uint64().value();

        ticker.time = now;

        benchmark::DoNotOptimize(ticker);
    }

    state.SetBytesProcessed(state.iterations() * message.size());
}

static void bm_faster_parser_ticker_workload(benchmark::State &state) {
    BenchmarkListener listener;
    auto now = std::chrono::system_clock::now();
    size_t index = 0;
    size_t total_bytes = 0;

    for (auto _ : state) {
        const auto& message = ticker_messages[index % ticker_messages.size()];
        bool result = binance_future_parser_t::parse(now, message, listener);
        benchmark::DoNotOptimize(result);
        benchmark::DoNotOptimize(listener.last_ticker);
        total_bytes += message.size();
        ++index;
    }

    state.SetBytesProcessed(total_bytes);
}

static void bm_simdjson_ticker_workload(benchmark::State &state) {
    simdjson::ondemand::parser parser;
    ticker_t ticker;
    auto now = std::chrono::system_clock::now();
    size_t index = 0;
    size_t total_bytes = 0;

    for (auto _ : state) {
        const auto& message = ticker_messages[index % ticker_messages.size()];
        simdjson::padded_string padded(message);
        auto doc = parser.iterate(padded);

        std::string_view event_type = doc["e"].get_string().value();
        ticker.event_time = doc["E"].get_uint64().value();
        ticker.symbol = doc["s"].get_string().value();

        std::string_view p_str = doc["p"].get_string().value();
        ticker.price_change = std::stod(std::string(p_str));

        std::string_view P_str = doc["P"].get_string().value();
        ticker.price_change_percent = std::stod(std::string(P_str));

        std::string_view w_str = doc["w"].get_string().value();
        ticker.weighted_avg_price = std::stod(std::string(w_str));

        std::string_view c_str = doc["c"].get_string().value();
        ticker.last_price = std::stod(std::string(c_str));

        std::string_view Q_str = doc["Q"].get_string().value();
        ticker.last_quantity = std::stod(std::string(Q_str));

        std::string_view o_str = doc["o"].get_string().value();
        ticker.open_price = std::stod(std::string(o_str));

        std::string_view h_str = doc["h"].get_string().value();
        ticker.high_price = std::stod(std::string(h_str));

        std::string_view l_str = doc["l"].get_string().value();
        ticker.low_price = std::stod(std::string(l_str));

        std::string_view v_str = doc["v"].get_string().value();
        ticker.total_traded_base_volume = std::stod(std::string(v_str));

        std::string_view q_str = doc["q"].get_string().value();
        ticker.total_traded_quote_volume = std::stod(std::string(q_str));

        ticker.statistics_open_time = doc["O"].get_uint64().value();
        ticker.statistics_close_time = doc["C"].get_uint64().value();
        ticker.first_trade_id = doc["F"].get_uint64().value();
        ticker.last_trade_id = doc["L"].get_uint64().value();
        ticker.total_trades = doc["n"].get_uint64().value();

        ticker.time = now;

        benchmark::DoNotOptimize(ticker);
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

// 24hrTicker: faster-parser vs simdjson
BENCHMARK(bm_faster_parser_ticker_single);
BENCHMARK(bm_simdjson_ticker_single);
BENCHMARK(bm_faster_parser_ticker_workload);
BENCHMARK(bm_simdjson_ticker_workload);

// Mixed workload: faster-parser vs simdjson
BENCHMARK(bm_faster_parser_mixed_workload);
BENCHMARK(bm_simdjson_mixed_workload);

BENCHMARK_MAIN();
