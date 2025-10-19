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

namespace core::faster_parser::binance::avx2 {
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

    // Find first occurrence of any character in a set (up to 4 characters)
    // Returns pointer to the found character and sets which_char to indicate which one was found (0-3)
    __attribute__((always_inline)) inline const char *find_char_set(const char *ptr, const char *end, const char *targets, size_t num_targets, int &which_char) {
        while (ptr + 32 <= end) {
            __m256i data = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(ptr));

            for (size_t i = 0; i < num_targets; ++i) {
                __m256i target_vec = _mm256_set1_epi8(targets[i]);
                __m256i cmp = _mm256_cmpeq_epi8(data, target_vec);
                int mask = _mm256_movemask_epi8(cmp);

                if (mask != 0) {
                    int offset = __builtin_ctz(mask);
                    which_char = static_cast<int>(i);
                    return ptr + offset;
                }
            }

            ptr += 32;
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
        __m256i comma_vec = _mm256_set1_epi8(',');
        __m256i quote_vec = _mm256_set1_epi8('"');

        while (ptr + 32 <= end) {
            __m256i data = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(ptr));
            __m256i comma_cmp = _mm256_cmpeq_epi8(data, comma_vec);
            __m256i quote_cmp = _mm256_cmpeq_epi8(data, quote_vec);

            int comma_mask = _mm256_movemask_epi8(comma_cmp);
            int quote_mask = _mm256_movemask_epi8(quote_cmp);

            if (comma_mask != 0 || quote_mask != 0) {
                int comma_offset = comma_mask ? __builtin_ctz(comma_mask) : 32;
                int quote_offset = quote_mask ? __builtin_ctz(quote_mask) : 32;

                if (comma_offset < quote_offset) {
                    is_comma = true;
                    return ptr + comma_offset;
                } else {
                    is_comma = false;
                    return ptr + quote_offset;
                }
            }

            ptr += 32;
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
} // namespace core::faster_parser::binance::avx2

#endif // FASTER_PARSER_BINANCE_AVX2_UTILS_AVX2_H
