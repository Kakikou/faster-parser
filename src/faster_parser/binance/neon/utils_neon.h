/**
 * @file simd_utils_neon.h
 * @author Kevin Rodrigues
 * @brief NEON SIMD utilities for Binance parser
 * @version 1.0
 * @date 10/10/2025
 */

#ifndef FASTER_PARSER_BINANCE_NEON_UTILS_NEON_H
#define FASTER_PARSER_BINANCE_NEON_UTILS_NEON_H

#include <cstddef>
#include <cstring>
#include <arm_neon.h>

namespace core::faster_parser::binance::neon {
    __attribute__((always_inline)) inline bool match_string(const char *ptr, const char *pattern, size_t len) {
        if (len == 16) {
            uint8x16_t data = vld1q_u8(reinterpret_cast<const uint8_t*>(ptr));
            uint8x16_t pat = vld1q_u8(reinterpret_cast<const uint8_t*>(pattern));
            uint8x16_t cmp = vceqq_u8(data, pat);
            uint8_t result = vminvq_u8(cmp);
            return result == 0xFF;
        } else if (len == 8) {
            uint8x8_t data = vld1_u8(reinterpret_cast<const uint8_t*>(ptr));
            uint8x8_t pat = vld1_u8(reinterpret_cast<const uint8_t*>(pattern));
            uint8x8_t cmp = vceq_u8(data, pat);
            uint64_t result = vget_lane_u64(vreinterpret_u64_u8(cmp), 0);
            return result == 0xFFFFFFFFFFFFFFFFULL;
        }
        return std::memcmp(ptr, pattern, len) == 0;
    }

    __attribute__((always_inline)) inline const char *find_char(const char *ptr, const char *end, char target) {
        uint8x16_t target_vec = vdupq_n_u8(target);

        while (ptr + 16 <= end) {
            uint8x16_t data = vld1q_u8(reinterpret_cast<const uint8_t*>(ptr));
            uint8x16_t cmp = vceqq_u8(data, target_vec);

            uint64x2_t cmp_u64 = vreinterpretq_u64_u8(cmp);
            uint64_t low = vgetq_lane_u64(cmp_u64, 0);
            uint64_t high = vgetq_lane_u64(cmp_u64, 1);

            if (low != 0) {
                for (int i = 0; i < 8; i++) {
                    if (ptr[i] == target) return &ptr[i];
                }
            }
            if (high != 0) {
                for (int i = 8; i < 16; i++) {
                    if (ptr[i] == target) return &ptr[i];
                }
            }
            ptr += 16;
        }

        while (ptr < end) {
            if (*ptr == target) return ptr;
            ptr++;
        }
        return nullptr;
    }

    // Find first occurrence of any character in a set (up to 4 characters)
    // Returns pointer to the found character and sets which_char to indicate which one was found (0-3)
    __attribute__((always_inline)) inline const char *find_char_set(const char *ptr, const char *end, const char *targets, size_t num_targets, int &which_char) {
        while (ptr + 16 <= end) {
            uint8x16_t data = vld1q_u8(reinterpret_cast<const uint8_t*>(ptr));

            for (size_t i = 0; i < num_targets; ++i) {
                uint8x16_t target_vec = vdupq_n_u8(targets[i]);
                uint8x16_t cmp = vceqq_u8(data, target_vec);

                uint64x2_t cmp_u64 = vreinterpretq_u64_u8(cmp);
                uint64_t low = vgetq_lane_u64(cmp_u64, 0);
                uint64_t high = vgetq_lane_u64(cmp_u64, 1);

                if (low != 0) {
                    for (int j = 0; j < 8; j++) {
                        if (ptr[j] == targets[i]) {
                            which_char = static_cast<int>(i);
                            return &ptr[j];
                        }
                    }
                }
                if (high != 0) {
                    for (int j = 8; j < 16; j++) {
                        if (ptr[j] == targets[i]) {
                            which_char = static_cast<int>(i);
                            return &ptr[j];
                        }
                    }
                }
            }

            ptr += 16;
        }

        // Fallback for remaining bytes
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
        uint8x16_t comma_vec = vdupq_n_u8(',');
        uint8x16_t quote_vec = vdupq_n_u8('"');

        while (ptr + 16 <= end) {
            uint8x16_t data = vld1q_u8(reinterpret_cast<const uint8_t*>(ptr));
            uint8x16_t comma_cmp = vceqq_u8(data, comma_vec);
            uint8x16_t quote_cmp = vceqq_u8(data, quote_vec);

            uint64x2_t comma_u64 = vreinterpretq_u64_u8(comma_cmp);
            uint64x2_t quote_u64 = vreinterpretq_u64_u8(quote_cmp);

            uint64_t comma_low = vgetq_lane_u64(comma_u64, 0);
            uint64_t comma_high = vgetq_lane_u64(comma_u64, 1);
            uint64_t quote_low = vgetq_lane_u64(quote_u64, 0);
            uint64_t quote_high = vgetq_lane_u64(quote_u64, 1);

            if (comma_low != 0 || comma_high != 0 || quote_low != 0 || quote_high != 0) {
                for (int i = 0; i < 16; i++) {
                    if (ptr[i] == ',') {
                        is_comma = true;
                        return &ptr[i];
                    }
                    if (ptr[i] == '"') {
                        is_comma = false;
                        return &ptr[i];
                    }
                }
            }

            ptr += 16;
        }

        // Fallback for remaining bytes
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
} // namespace core::faster_parser::binance::neon

#endif // FASTER_PARSER_BINANCE_NEON_UTILS_NEON_H
