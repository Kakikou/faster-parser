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

    // Find first occurrence of any character in a set (up to 4 characters)
    // Returns pointer to the found character and sets which_char to indicate which one was found (0-3)
    __attribute__((always_inline)) inline const char *find_char_set(const char *ptr, const char *end, const char *targets, size_t num_targets, int &which_char) {
        while (ptr < end) {
            for (size_t i = 0; i < num_targets; ++i) {
                if (*ptr == targets[i]) {
                    which_char = static_cast<int>(i);
                    return ptr;
                }
            }
            ptr++;
        }
        return nullptr;
    }

    // Specialized version for finding either comma or quote
    __attribute__((always_inline)) inline const char *find_comma_or_quote(const char *ptr, const char *end, bool &is_comma) {
        while (ptr < end) {
            if (*ptr == ',') {
                is_comma = true;
                return ptr;
            }
            if (*ptr == '"') {
                is_comma = false;
                return ptr;
            }
            ptr++;
        }
        return nullptr;
    }
} // namespace core::faster_parser::binance::scalar

#endif // FASTER_PARSER_BINANCE_SCALAR_UTILS_SCALAR_H
