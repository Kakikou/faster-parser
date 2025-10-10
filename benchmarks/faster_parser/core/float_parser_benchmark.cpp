/**
 * @file float_parser_benchmark.cpp
 * @author Kevin Rodrigues
 * @brief Benchmark comparing core::fast_float_parser vs strtod vs std::stod
 * @version 1.0
 * @date 22/09/2025
 */

#include <benchmark/benchmark.h>
#include <chrono>
#include <string>
#include <vector>
#include <random>

#include <faster_parser/core/fast_scalar_parser.h>

const std::vector<std::string> financial_price_samples = {
    "25.35190000",
    "0.00001234",
    "67890.12345678",
    "1.00000000",
    "999.99999999",
    "0.12345678",
    "12345.00000000",
    "0.00000001",
    "999999.99999999",
    "123.45678900",
    "456.78900000",
    "0.98765432",
    "100.50000000",
    "0.00123456",
    "88888.88888888"
};

const std::vector<std::string> volume_samples = {
    "31.21000000",
    "40.66000000",
    "12.00000000",
    "1000.00000000",
    "0.10000000",
    "999.99999999",
    "1.00000000",
    "100000.00000000",
    "0.00100000",
    "50.75000000"
};

std::vector<std::string> generate_random_prices(size_t count) {
    std::vector<std::string> prices;
    prices.reserve(count);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> price_dist(0.00000001, 999999.99999999);

    for (size_t i = 0; i < count; ++i) {
        double price = price_dist(gen);
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "%.8f", price);
        prices.emplace_back(buffer);
    }

    return prices;
}

const auto random_prices = generate_random_prices(1000);

static void bm_fast_float_parser_financial_prices(benchmark::State &state) {
    size_t index = 0;
    double sum = 0.0;

    for (auto _: state) {
        const auto &price_str = financial_price_samples[index % financial_price_samples.size()];
        double result = core::fast_scalar_parser::parse_float(price_str);
        sum += result;
        ++index;
        benchmark::DoNotOptimize(result);
    }

    benchmark::DoNotOptimize(sum);
}

static void bm_strtod_financial_prices(benchmark::State &state) {
    size_t index = 0;
    double sum = 0.0;

    for (auto _: state) {
        const auto &price_str = financial_price_samples[index % financial_price_samples.size()];
        char *end;
        double result = strtod(price_str.c_str(), &end);
        sum += result;
        ++index;
        benchmark::DoNotOptimize(result);
    }

    benchmark::DoNotOptimize(sum);
}

static void bm_std_stod_financial_prices(benchmark::State &state) {
    size_t index = 0;
    double sum = 0.0;

    for (auto _: state) {
        const auto &price_str = financial_price_samples[index % financial_price_samples.size()];
        double result = std::stod(price_str);
        sum += result;
        ++index;
        benchmark::DoNotOptimize(result);
    }

    benchmark::DoNotOptimize(sum);
}

static void bm_fast_float_parser_fixed_8_decimals(benchmark::State &state) {
    size_t index = 0;
    double sum = 0.0;

    for (auto _: state) {
        const auto &price_str = financial_price_samples[index % financial_price_samples.size()];
        double result = core::fast_scalar_parser::parse_float(price_str);
        sum += result;
        ++index;
        benchmark::DoNotOptimize(result);
    }

    benchmark::DoNotOptimize(sum);
}

static void bm_fast_float_parser_random_data(benchmark::State &state) {
    size_t index = 0;
    double sum = 0.0;

    for (auto _: state) {
        const auto &price_str = random_prices[index % random_prices.size()];
        double result = core::fast_scalar_parser::parse_float(price_str);
        sum += result;
        ++index;
        benchmark::DoNotOptimize(result);
    }

    benchmark::DoNotOptimize(sum);
}

static void bm_strtod_random_data(benchmark::State &state) {
    size_t index = 0;
    double sum = 0.0;

    for (auto _: state) {
        const auto &price_str = random_prices[index % random_prices.size()];
        char *end;
        double result = strtod(price_str.c_str(), &end);
        sum += result;
        ++index;
        benchmark::DoNotOptimize(result);
    }

    benchmark::DoNotOptimize(sum);
}

static void bm_std_stod_random_data(benchmark::State &state) {
    size_t index = 0;
    double sum = 0.0;

    for (auto _: state) {
        const auto &price_str = random_prices[index % random_prices.size()];
        double result = std::stod(price_str);
        sum += result;
        ++index;
        benchmark::DoNotOptimize(result);
    }

    benchmark::DoNotOptimize(sum);
}

static void bm_fast_float_parser_volumes(benchmark::State &state) {
    size_t index = 0;
    double sum = 0.0;

    for (auto _: state) {
        const auto &volume_str = volume_samples[index % volume_samples.size()];
        double result = core::fast_scalar_parser::parse_float(volume_str);
        sum += result;
        ++index;
        benchmark::DoNotOptimize(result);
    }

    benchmark::DoNotOptimize(sum);
}

static void bm_strtod_volumes(benchmark::State &state) {
    size_t index = 0;
    double sum = 0.0;

    for (auto _: state) {
        const auto &volume_str = volume_samples[index % volume_samples.size()];
        char *end;
        double result = strtod(volume_str.c_str(), &end);
        sum += result;
        ++index;
        benchmark::DoNotOptimize(result);
    }

    benchmark::DoNotOptimize(sum);
}

static void bm_fast_float_parser_mixed_workload(benchmark::State &state) {
    size_t price_index = 0;
    size_t volume_index = 0;
    double sum = 0.0;

    for (auto _: state) {
        if ((price_index + volume_index) % 2 == 0) {
            const auto &price_str = financial_price_samples[price_index % financial_price_samples.size()];
            double result = core::fast_scalar_parser::parse_float(price_str);
            sum += result;
            ++price_index;
        } else {
            const auto &volume_str = volume_samples[volume_index % volume_samples.size()];
            double result = core::fast_scalar_parser::parse_float(volume_str);
            sum += result;
            ++volume_index;
        }
        benchmark::DoNotOptimize(sum);
    }

    benchmark::DoNotOptimize(sum);
}

static void bm_strtod_mixed_workload(benchmark::State &state) {
    size_t price_index = 0;
    size_t volume_index = 0;
    double sum = 0.0;

    for (auto _: state) {
        char *end;
        if ((price_index + volume_index) % 2 == 0) {
            const auto &price_str = financial_price_samples[price_index % financial_price_samples.size()];
            double result = strtod(price_str.c_str(), &end);
            sum += result;
            ++price_index;
        } else {
            const auto &volume_str = volume_samples[volume_index % volume_samples.size()];
            double result = strtod(volume_str.c_str(), &end);
            sum += result;
            ++volume_index;
        }
        benchmark::DoNotOptimize(sum);
    }

    benchmark::DoNotOptimize(sum);
}

BENCHMARK(bm_fast_float_parser_financial_prices);
BENCHMARK(bm_strtod_financial_prices);
BENCHMARK(bm_std_stod_financial_prices);

BENCHMARK(bm_fast_float_parser_fixed_8_decimals);

BENCHMARK(bm_fast_float_parser_random_data);
BENCHMARK(bm_strtod_random_data);
BENCHMARK(bm_std_stod_random_data);

BENCHMARK(bm_fast_float_parser_volumes);
BENCHMARK(bm_strtod_volumes);

BENCHMARK(bm_fast_float_parser_mixed_workload);
BENCHMARK(bm_strtod_mixed_workload);

BENCHMARK_MAIN();
