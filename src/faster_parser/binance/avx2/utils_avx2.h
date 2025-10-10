/**
 * @file utils_avx2.h
 * @author Kevin Rodrigues
 * @brief AVX2 SIMD utilities for Binance parser
 * @version 1.0
 * @date 10/10/2025
 */

#ifndef FASTER_PARSER_BINANCE_AVX2_UTILS_AVX2_H
#define FASTER_PARSER_BINANCE_AVX2_UTILS_AVX2_H

#include <cstddef>
#include <cstring>
#include <immintrin.h>

namespace core::binance::avx2 {
    __attribute__((always_inline)) inline bool match_string(const char *ptr, const char *pattern, size_t len) {
        if (len == 16) {
            __m128i data = _mm_loadu_si128(reinterpret_cast<const __m128i*>(ptr));
            __m128i pat = _mm_loadu_si128(reinterpret_cast<const __m128i*>(pattern));

            __m128i cmp = _mm_cmpeq_epi8(data, pat);
            int mask = _mm_movemask_epi8(cmp);
            return mask == 0xFFFF;
        } else if (len == 32) {
            __m256i data = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(ptr));
            __m256i pat = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(pattern));

            __m256i cmp = _mm256_cmpeq_epi8(data, pat);
            unsigned int mask = static_cast<unsigned int>(_mm256_movemask_epi8(cmp));
            return mask == 0xFFFFFFFFU;
        } else if (len == 8) {
            uint64_t data = *reinterpret_cast<const uint64_t*>(ptr);
            uint64_t pat = *reinterpret_cast<const uint64_t*>(pattern);
            return data == pat;
        }
        return std::memcmp(ptr, pattern, len) == 0;
    }

    __attribute__((always_inline)) inline const char *find_char(const char *ptr, const char *end, char target) {
        __m256i target_vec = _mm256_set1_epi8(target);

        while (ptr + 32 <= end) {
            __m256i data = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(ptr));
            __m256i cmp = _mm256_cmpeq_epi8(data, target_vec);

            int mask = _mm256_movemask_epi8(cmp);

            if (mask != 0) {
                int offset = __builtin_ctz(mask);
                return ptr + offset;
            }

            ptr += 32;
        }

        if (ptr + 16 <= end) {
            __m128i target_vec_128 = _mm_set1_epi8(target);
            __m128i data = _mm_loadu_si128(reinterpret_cast<const __m128i*>(ptr));
            __m128i cmp = _mm_cmpeq_epi8(data, target_vec_128);

            int mask = _mm_movemask_epi8(cmp);

            if (mask != 0) {
                int offset = __builtin_ctz(mask);
                return ptr + offset;
            }

            ptr += 16;
        }

        while (ptr < end) {
            if (*ptr == target) return ptr;
            ptr++;
        }
        return nullptr;
    }
} // namespace core::binance::avx2

#endif // FASTER_PARSER_BINANCE_AVX2_UTILS_AVX2_H
