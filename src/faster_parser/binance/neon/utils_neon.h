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
} // namespace core::faster_parser::binance::neon

#endif // FASTER_PARSER_BINANCE_NEON_UTILS_NEON_H
