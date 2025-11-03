/**
 * @file adaptive_dispatch.h
 * @author Kevin Rodrigues
 * @brief Adaptive dispatch between AVX-512 and AVX2 based on message size
 * @version 1.0
 * @date 10/10/2025
 */

#ifndef FASTER_PARSER_ADAPTIVE_DISPATCH_H
#define FASTER_PARSER_ADAPTIVE_DISPATCH_H

#include <chrono>
#include <string_view>

#include "faster_parser/binance/avx512/utils_avx512.h"
#include "faster_parser/binance/avx512/book_ticker_avx512.h"
#include "faster_parser/binance/avx2/utils_avx2.h"
#include "faster_parser/binance/avx2/book_ticker_avx2.h"

namespace core::faster_parser::binance::adaptive {
    constexpr size_t AVX512_THRESHOLD = 512;

    template<typename listener_t>
    __attribute__((always_inline)) inline bool process_book_ticker(std::chrono::system_clock::time_point const &now, std::string_view raw, listener_t &listener) {
        // For small messages, use AVX2 to avoid frequency throttling
        if (raw.size() < AVX512_THRESHOLD) {
            return avx2::process_book_ticker(now, raw, listener);
        }
        return avx512::process_book_ticker(now, raw, listener);
    }

    __attribute__((always_inline)) inline bool match_string(const char *ptr, const char *pattern, size_t len) {
        return avx2::match_string(ptr, pattern, len);
    }

    __attribute__((always_inline)) inline const char *find_char(const char *ptr, const char *end, char target) {
        size_t length = end - ptr;

        // For small searches, use AVX2
        if (length < AVX512_THRESHOLD) {
            return avx2::find_char(ptr, end, target);
        } else {
            return avx512::find_char(ptr, end, target);
        }
    }
} // core::faster_parser::binance::adaptive

#endif // FASTER_PARSER_ADAPTIVE_DISPATCH_H
