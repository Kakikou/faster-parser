/**
 * @file binance_future_parser_benchmark.cpp
 * @author Kevin Rodrigues
 * @brief Benchmark for Binance Future Parser with SIMD optimizations
 * @version 1.0
 * @date 10/10/2025
 */

#include <chrono>
#include <string>
#include <vector>
#include <benchmark/benchmark.h>

#include <faster_parser/binance/future.h>

using namespace core::binance;
using namespace core::binance::types;

class BenchmarkListener {
public:
    book_ticker_t last_ticker;

    void on_book_ticker(const book_ticker_t& ticker) {
        last_ticker = ticker;
    }
};

const std::vector<std::string> book_ticker_messages = {
    R"({"e":"bookTicker","u":8822354685185,"s":"ASTERUSDT","b":"1.5822000","B":"457","a":"1.5823000","A":"112","T":1760083106579,"E":1760083106579})",
    R"({"e":"bookTicker","u":123456789,"s":"BTCUSDT","b":"45123.78900000","B":"10.5","a":"45124.12300000","A":"5.25","T":1234567890123,"E":1234567890123})",
    R"({"e":"bookTicker","u":999999,"s":"DOGEUSDT","b":"0.00012345","B":"1000000","a":"0.00012346","A":"999999","T":9999999999,"E":9999999999})",
    R"({"e":"bookTicker","u":111111111,"s":"ETHUSDT","b":"3000","B":"100","a":"3001","A":"200","T":1111111111111,"E":1111111111111})",
    R"({"e":"bookTicker","u":12345,"s":"BTCUSDT","b":"50000.00000000","B":"1.00000000","a":"50001.00000000","A":"2.00000000","T":1234567890,"E":1234567890})",
    R"({"e":"bookTicker","u":777,"s":"ETHUSDT","b":"3500.5","B":"10.25","a":"3500.75","A":"20.5","T":777777,"E":777777})",
    R"({"e":"bookTicker","u":555555,"s":"ADAUSDT","b":"0.45678","B":"5000","a":"0.45679","A":"4500","T":555555555,"E":555555555})",
    R"({"e":"bookTicker","u":666666,"s":"SOLUSDT","b":"123.456","B":"25.5","a":"123.457","A":"30.25","T":666666666,"E":666666666})",
};

static void bm_binance_future_parse_book_ticker(benchmark::State &state) {
    BenchmarkListener listener;
    auto now = std::chrono::system_clock::now();
    size_t index = 0;

    for (auto _ : state) {
        const auto& message = book_ticker_messages[index % book_ticker_messages.size()];
        bool result = binance_future_parser_t::parse(now, message, listener);
        benchmark::DoNotOptimize(result);
        benchmark::DoNotOptimize(listener.last_ticker);
        ++index;
    }
}

static void bm_binance_future_parse_btc_high_frequency(benchmark::State &state) {
    BenchmarkListener listener;
    auto now = std::chrono::system_clock::now();
    std::string_view message = R"({"e":"bookTicker","u":123456789,"s":"BTCUSDT","b":"45123.78900000","B":"10.5","a":"45124.12300000","A":"5.25","T":1234567890123,"E":1234567890123})";

    for (auto _ : state) {
        bool result = binance_future_parser_t::parse(now, message, listener);
        benchmark::DoNotOptimize(result);
        benchmark::DoNotOptimize(listener.last_ticker);
    }
}

static void bm_binance_future_parse_small_prices(benchmark::State &state) {
    BenchmarkListener listener;
    auto now = std::chrono::system_clock::now();
    std::string_view message = R"({"e":"bookTicker","u":999999,"s":"DOGEUSDT","b":"0.00012345","B":"1000000","a":"0.00012346","A":"999999","T":9999999999,"E":9999999999})";

    for (auto _ : state) {
        bool result = binance_future_parser_t::parse(now, message, listener);
        benchmark::DoNotOptimize(result);
        benchmark::DoNotOptimize(listener.last_ticker);
    }
}

static void bm_binance_future_parse_large_volumes(benchmark::State &state) {
    BenchmarkListener listener;
    auto now = std::chrono::system_clock::now();
    std::string_view message = R"({"e":"bookTicker","u":12345,"s":"BTCUSDT","b":"50000.00000000","B":"1.00000000","a":"50001.00000000","A":"2.00000000","T":1234567890,"E":1234567890})";

    for (auto _ : state) {
        bool result = binance_future_parser_t::parse(now, message, listener);
        benchmark::DoNotOptimize(result);
        benchmark::DoNotOptimize(listener.last_ticker);
    }
}

static void bm_binance_future_process_book_ticker(benchmark::State &state) {
    BenchmarkListener listener;
    auto now = std::chrono::system_clock::now();
    std::string_view message = R"({"e":"bookTicker","u":8822354685185,"s":"ASTERUSDT","b":"1.5822000","B":"457","a":"1.5823000","A":"112","T":1760083106579,"E":1760083106579})";

    for (auto _ : state) {
        binance_future_parser_t::process_book_ticker(now, message, listener);
        benchmark::DoNotOptimize(listener.last_ticker);
    }
}

static void bm_binance_future_parse_mixed_workload(benchmark::State &state) {
    BenchmarkListener listener;
    auto now = std::chrono::system_clock::now();
    size_t index = 0;

    for (auto _ : state) {
        const auto& message = book_ticker_messages[index % book_ticker_messages.size()];
        bool result = binance_future_parser_t::parse(now, message, listener);
        benchmark::DoNotOptimize(result);
        benchmark::DoNotOptimize(listener.last_ticker);
        ++index;
    }
}

static void bm_binance_future_throughput(benchmark::State &state) {
    BenchmarkListener listener;
    auto now = std::chrono::system_clock::now();
    size_t messages_processed = 0;

    for (auto _ : state) {
        for (const auto& message : book_ticker_messages) {
            bool result = binance_future_parser_t::parse(now, message, listener);
            benchmark::DoNotOptimize(result);
            benchmark::DoNotOptimize(listener.last_ticker);
            ++messages_processed;
        }
    }

    state.SetItemsProcessed(messages_processed);
}

static void bm_binance_future_latency(benchmark::State &state) {
    BenchmarkListener listener;
    auto now = std::chrono::system_clock::now();
    std::string_view message = R"({"e":"bookTicker","u":8822354685185,"s":"ASTERUSDT","b":"1.5822000","B":"457","a":"1.5823000","A":"112","T":1760083106579,"E":1760083106579})";

    for (auto _ : state) {
        auto start = std::chrono::high_resolution_clock::now();
        bool result = binance_future_parser_t::parse(now, message, listener);
        auto end = std::chrono::high_resolution_clock::now();

        benchmark::DoNotOptimize(result);
        benchmark::DoNotOptimize(listener.last_ticker);

        auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
        state.SetIterationTime(elapsed.count() / 1e9);
    }
}

static void bm_binance_future_parse_short_symbol(benchmark::State &state) {
    BenchmarkListener listener;
    auto now = std::chrono::system_clock::now();
    std::string_view message = R"({"e":"bookTicker","u":1,"s":"BTC","b":"1.0","B":"1","a":"1.1","A":"1","T":1,"E":1})";

    for (auto _ : state) {
        bool result = binance_future_parser_t::parse(now, message, listener);
        benchmark::DoNotOptimize(result);
        benchmark::DoNotOptimize(listener.last_ticker);
    }
}

static void bm_binance_future_parse_long_symbol(benchmark::State &state) {
    BenchmarkListener listener;
    auto now = std::chrono::system_clock::now();
    std::string_view message = R"({"e":"bookTicker","u":123456789012345,"s":"LONGSYMBOLUSDT","b":"99999.99999999","B":"99999.99999999","a":"100000.00000000","A":"100000.00000000","T":9999999999999,"E":9999999999999})";

    for (auto _ : state) {
        bool result = binance_future_parser_t::parse(now, message, listener);
        benchmark::DoNotOptimize(result);
        benchmark::DoNotOptimize(listener.last_ticker);
    }
}

BENCHMARK(bm_binance_future_parse_book_ticker);
BENCHMARK(bm_binance_future_parse_btc_high_frequency);
BENCHMARK(bm_binance_future_parse_small_prices);
BENCHMARK(bm_binance_future_parse_large_volumes);
BENCHMARK(bm_binance_future_process_book_ticker);
BENCHMARK(bm_binance_future_parse_mixed_workload);
BENCHMARK(bm_binance_future_throughput);
BENCHMARK(bm_binance_future_latency)->UseManualTime();
BENCHMARK(bm_binance_future_parse_short_symbol);
BENCHMARK(bm_binance_future_parse_long_symbol);

BENCHMARK_MAIN();