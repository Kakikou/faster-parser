/**
 * @file book_ticker_avx2.h
 * @author Kevin Rodrigues
 * @brief AVX2 SIMD-optimized Binance book ticker parser
 * @version 1.0
 * @date 10/10/2025
 */

#ifndef FASTER_PARSER_BOOK_TICKER_AVX2_H
#define FASTER_PARSER_BOOK_TICKER_AVX2_H

#include <chrono>
#include <string_view>

#include "faster_parser/binance/concepts.h"
#include "faster_parser/binance/avx2/utils_avx2.h"
#include "faster_parser/core/fast_scalar_parser.h"
#include "faster_parser/binance/types/book_ticker.h"

namespace core::faster_parser::binance::avx2 {
    template<BinanceFutureListener listener_t>
    __attribute__((always_inline)) inline bool process_book_ticker(std::chrono::system_clock::time_point const &now, std::string_view raw, listener_t &listener) {
        types::book_ticker_t ticker;
        ticker.time = now;

        const char *ptr = raw.data();
        const char *end = raw.data() + raw.size();

        ptr = find_char(ptr, end, 'u');
        if (!ptr) return false;
        ptr += 3;

        const char *value_start = ptr;
        ptr = find_char(ptr, end, ',');
        if (!ptr) return false;
        uint64_t update_id = fast_scalar_parser::parse_uint64(std::string_view(value_start, ptr));

        ptr = find_char(ptr, end, 's');
        if (!ptr) return false;
        ptr += 4;

        value_start = ptr;
        ptr = find_char(ptr, end, '"');
        if (!ptr) return false;
        ticker.symbol = std::string_view(value_start, ptr - value_start);
        ptr++;

        ptr = find_char(ptr, end, 'b');
        if (!ptr) return false;
        ptr += 4;

        value_start = ptr;
        ptr = find_char(ptr, end, '"');
        if (!ptr) return false;
        ticker.bid.price = fast_scalar_parser::parse_float(std::string_view(value_start, ptr));
        ptr++;

        ptr = find_char(ptr, end, 'B');
        if (!ptr) return false;
        ptr += 4;

        value_start = ptr;
        ptr = find_char(ptr, end, '"');
        if (!ptr) return false;
        ticker.bid.volume = fast_scalar_parser::parse_float(std::string_view(value_start, ptr));
        ptr++;

        ptr = find_char(ptr, end, 'a');
        if (!ptr) return false;
        ptr += 4;

        value_start = ptr;
        ptr = find_char(ptr, end, '"');
        if (!ptr) return false;
        ticker.ask.price = fast_scalar_parser::parse_float(std::string_view(value_start, ptr));
        ptr++;

        ptr = find_char(ptr, end, 'A');
        if (!ptr) return false;
        ptr += 4;

        value_start = ptr;
        ptr = find_char(ptr, end, '"');
        if (!ptr) return false;
        ticker.ask.volume = fast_scalar_parser::parse_float(std::string_view(value_start, ptr));
        ptr++;

        ptr = find_char(ptr, end, 'T');
        if (!ptr) return false;
        ptr = find_char(ptr, end, ',');
        if (!ptr) return false;
        ptr++;

        ptr = find_char(ptr, end, 'E');
        if (!ptr) return false;
        ptr += 3;

        value_start = ptr;
        ptr = find_char(ptr, end, '}');
        if (!ptr) return false;
        ticker.exchange_timestamp = fast_scalar_parser::parse_uint64(std::string_view(value_start, ptr));

        ticker.bid.sequence = update_id;
        ticker.ask.sequence = update_id;

        listener.on_book_ticker(ticker);
        return true;
    }
} // namespace core::faster_parser::binance::avx2

#endif //FASTER_PARSER_BOOK_TICKER_AVX2_H
