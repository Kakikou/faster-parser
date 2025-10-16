/**
 * @file utils_avx512.h
 * @author Kevin Rodrigues
 * @brief AVX-512 SIMD utilities for Binance parser
 * @version 1.0
 * @date 10/10/2025
 */

#ifndef FASTER_PARSER_BINANCE_AVX512_UTILS_AVX512_H
#define FASTER_PARSER_BINANCE_AVX512_UTILS_AVX512_H

#include <cstddef>
#include <cstring>
#include <immintrin.h>

namespace core::faster_parser::binance::avx512 {
    __attribute__((always_inline)) inline bool match_string(const char *ptr, const char *pattern, size_t len) {
        if (len == 64) {
            __m512i data = _mm512_loadu_si512(reinterpret_cast<const __m512i*>(ptr));
            __m512i pat = _mm512_loadu_si512(reinterpret_cast<const __m512i*>(pattern));

            __mmask64 cmp_mask = _mm512_cmpeq_epi8_mask(data, pat);
            return cmp_mask == 0xFFFFFFFFFFFFFFFFULL;
        } else if (len == 32) {
            __m256i data = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(ptr));
            __m256i pat = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(pattern));

            __mmask32 cmp_mask = _mm256_cmpeq_epi8_mask(data, pat);
            return cmp_mask == 0xFFFFFFFFU;
        } else if (len == 16) {
            __m128i data = _mm_loadu_si128(reinterpret_cast<const __m128i*>(ptr));
            __m128i pat = _mm_loadu_si128(reinterpret_cast<const __m128i*>(pattern));

            __mmask16 cmp_mask = _mm_cmpeq_epi8_mask(data, pat);
            return cmp_mask == 0xFFFFU;
        } else if (len == 8) {
            uint64_t data = *reinterpret_cast<const uint64_t*>(ptr);
            uint64_t pat = *reinterpret_cast<const uint64_t*>(pattern);
            return data == pat;
        }
        return std::memcmp(ptr, pattern, len) == 0;
    }

    __attribute__((always_inline)) inline const char *find_char(const char *ptr, const char *end, char target) {
        __m512i target_vec = _mm512_set1_epi8(target);

        while (ptr + 64 <= end) {
            __m512i data = _mm512_loadu_si512(reinterpret_cast<const __m512i*>(ptr));
            __mmask64 cmp_mask = _mm512_cmpeq_epi8_mask(data, target_vec);

            if (cmp_mask != 0) {
                int offset = __builtin_ctzll(cmp_mask);
                return ptr + offset;
            }

            ptr += 64;
        }

        if (ptr + 32 <= end) {
            __m256i target_vec_256 = _mm256_set1_epi8(target);
            __m256i data = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(ptr));
            __mmask32 cmp_mask = _mm256_cmpeq_epi8_mask(data, target_vec_256);

            if (cmp_mask != 0) {
                int offset = __builtin_ctz(cmp_mask);
                return ptr + offset;
            }

            ptr += 32;
        }

        if (ptr + 16 <= end) {
            __m128i target_vec_128 = _mm_set1_epi8(target);
            __m128i data = _mm_loadu_si128(reinterpret_cast<const __m128i*>(ptr));
            __mmask16 cmp_mask = _mm_cmpeq_epi8_mask(data, target_vec_128);

            if (cmp_mask != 0) {
                int offset = __builtin_ctz(cmp_mask);
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
} // namespace core::faster_parser::binance::avx512

#endif // FASTER_PARSER_BINANCE_AVX512_UTILS_AVX512_H
