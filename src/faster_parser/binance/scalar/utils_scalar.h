/**
 * @file simd_utils_scalar.h
 * @author Kevin Rodrigues
 * @brief Scalar utilities for Binance parser (fallback)
 * @version 1.0
 * @date 10/10/2025
 */

#ifndef FASTER_PARSER_BINANCE_SCALAR_UTILS_SCALAR_H
#define FASTER_PARSER_BINANCE_SCALAR_UTILS_SCALAR_H

#include <cstddef>
#include <cstring>

namespace core::faster_parser::binance::scalar {
    __attribute__((always_inline)) inline bool match_string(const char* ptr, const char* pattern, size_t len) {
        return std::memcmp(ptr, pattern, len) == 0;
    }

    __attribute__((always_inline)) inline const char* find_char(const char* ptr, const char* end, char target) {
        while (ptr < end) {
            if (*ptr == target) return ptr;
            ptr++;
        }
        return nullptr;
    }
} // namespace core::faster_parser::binance::scalar

#endif // FASTER_PARSER_BINANCE_SCALAR_UTILS_SCALAR_H
