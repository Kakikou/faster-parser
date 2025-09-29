/**
 * @file parsers.cpp
 * @author Kevin Rodrigues
 * @brief Fast floating-point parser implementation with SIMD optimizations
 * @version 1.0
 * @date 11/09/2025
 */

#include "parsers.h"
#include <string>

namespace core::fast_float_parser {

    const double powers_of_10[19] = {
        1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9,
        1e10, 1e11, 1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18
    };

    const double negative_powers_of_10[19] = {
        1e0, 1e-1, 1e-2, 1e-3, 1e-4, 1e-5, 1e-6, 1e-7, 1e-8, 1e-9,
        1e-10, 1e-11, 1e-12, 1e-13, 1e-14, 1e-15, 1e-16, 1e-17, 1e-18
    };

    static bool all_digits_simd(std::string_view str) {
        const char* data = str.data();
        const size_t len = str.size();
#ifdef HAS_AVX512
        if (len >= 64) {
            __m512i chunk = _mm512_loadu_si512(data);
            __m512i zero = _mm512_set1_epi8('0');
            __m512i nine = _mm512_set1_epi8('9');

            __mmask64 ge_zero = _mm512_cmpge_epi8_mask(chunk, zero);
            __mmask64 le_nine = _mm512_cmple_epi8_mask(chunk, nine);
            __mmask64 valid = ge_zero & le_nine;

            __mmask64 len_mask = (1ULL << len) - 1;
            return (valid & len_mask) == len_mask;
        }
#endif

#ifdef HAS_AVX2
        if (len >= 32) {
            __m256i chunk = _mm256_loadu_si256((__m256i *) data);
            __m256i zero = _mm256_set1_epi8('0');
            __m256i nine = _mm256_set1_epi8('9');

            __m256i ge_zero = _mm256_cmpgt_epi8(chunk, _mm256_sub_epi8(zero, _mm256_set1_epi8(1)));
            __m256i le_nine = _mm256_cmpgt_epi8(_mm256_add_epi8(nine, _mm256_set1_epi8(1)), chunk);
            __m256i valid = _mm256_and_si256(ge_zero, le_nine);

            uint32_t mask = _mm256_movemask_epi8(valid);
            uint32_t len_mask = (len >= 32) ? 0xFFFFFFFF : ((1U << len) - 1);
            return (mask & len_mask) == len_mask;
        }
#endif

#ifdef HAS_SSE42
        if (len >= 16) {
            __m128i chunk = _mm_loadu_si128((__m128i *) data);
            __m128i zero = _mm_set1_epi8('0');
            __m128i nine = _mm_set1_epi8('9');

            __m128i ge_zero = _mm_cmpgt_epi8(chunk, _mm_subs_epi8(zero, _mm_set1_epi8(1)));
            __m128i le_nine = _mm_cmplt_epi8(chunk, _mm_adds_epi8(nine, _mm_set1_epi8(1)));
            __m128i valid = _mm_and_si128(ge_zero, le_nine);

            uint32_t mask = _mm_movemask_epi8(valid);
            uint32_t len_mask = (len >= 16) ? 0xFFFF : ((1U << len) - 1);
            return (mask & len_mask) == len_mask;
        }
#elif defined(HAS_NEON)
        if (len >= 16) {
            uint8x16_t chunk = vld1q_u8((uint8_t*)data);
            uint8x16_t zero = vdupq_n_u8('0');
            uint8x16_t nine = vdupq_n_u8('9');

            uint8x16_t ge_zero = vcgeq_u8(chunk, zero);
            uint8x16_t le_nine = vcleq_u8(chunk, nine);
            uint8x16_t valid = vandq_u8(ge_zero, le_nine);

#ifdef __aarch64__
            uint8_t min_result = vminvq_u8(valid);
            return min_result == 0xFF;
#else
            uint64x2_t valid_u64 = vreinterpretq_u64_u8(valid);
            uint64_t low = vgetq_lane_u64(valid_u64, 0);
            uint64_t high = vgetq_lane_u64(valid_u64, 1);

            if (len <= 8) {
                uint64_t mask = (1ULL << (len * 8)) - 1;
                return (low & mask) == mask;
            } else {
                uint64_t high_mask = (len >= 16) ? 0xFFFFFFFFFFFFFFFFULL : ((1ULL << ((len - 8) * 8)) - 1);
                return (low == 0xFFFFFFFFFFFFFFFFULL) && ((high & high_mask) == high_mask);
            }
#endif
        }

        if (len >= 8) {
            uint8x8_t chunk = vld1_u8((uint8_t*)data);
            uint8x8_t zero = vdup_n_u8('0');
            uint8x8_t nine = vdup_n_u8('9');

            uint8x8_t ge_zero = vcge_u8(chunk, zero);
            uint8x8_t le_nine = vcle_u8(chunk, nine);
            uint8x8_t valid = vand_u8(ge_zero, le_nine);

            uint64_t result_u64 = vget_lane_u64(vreinterpret_u64_u8(valid), 0);
            uint64_t mask = (len >= 8) ? 0xFFFFFFFFFFFFFFFFULL : ((1ULL << (len * 8)) - 1);
            return (result_u64 & mask) == mask;
        }
#endif

        size_t i = 0;
        for (; i + 4 <= len; i += 4) {
            char c0 = data[i];
            char c1 = data[i + 1];
            char c2 = data[i + 2];
            char c3 = data[i + 3];
            if (((c0 - '0') | ('9' - c0) |
                 (c1 - '0') | ('9' - c1) |
                 (c2 - '0') | ('9' - c2) |
                 (c3 - '0') | ('9' - c3)) & 0x80) {
                return false;
            }
        }

        for (; i < len; i++) {
            char c = data[i];
            if (((c - '0') | ('9' - c)) & 0x80) {
                return false;
            }
        }

        return true;
    }

    static uint64_t parse_8_digits_simd(std::string_view str) {
        const char* data = str.data();
#ifdef HAS_AVX512
        __m512i chunk = _mm512_loadu_si512(data);
        __m512i zero = _mm512_set1_epi8('0');
        __m512i digits = _mm512_sub_epi8(chunk, zero);

        __m128i low_128 = _mm512_extracti64x2_epi64(digits, 0);
        uint64_t raw = _mm_extract_epi64(low_128, 0);

        uint64_t result = 0;
        result += ((raw >> 0) & 0xFF) * 10000000ULL;   // data[0]
        result += ((raw >> 8) & 0xFF) * 1000000ULL;    // data[1]
        result += ((raw >> 16) & 0xFF) * 100000ULL;    // data[2]
        result += ((raw >> 24) & 0xFF) * 10000ULL;     // data[3]
        result += ((raw >> 32) & 0xFF) * 1000ULL;      // data[4]
        result += ((raw >> 40) & 0xFF) * 100ULL;       // data[5]
        result += ((raw >> 48) & 0xFF) * 10ULL;        // data[6]
        result += ((raw >> 56) & 0xFF) * 1ULL;         // data[7]

        return result;
#elif defined(HAS_AVX2)
        __m256i chunk = _mm256_set1_epi64x(*reinterpret_cast<const uint64_t *>(data));
        __m256i zero = _mm256_set1_epi8('0');
        __m256i digits = _mm256_sub_epi8(chunk, zero);

        uint64_t raw = _mm256_extract_epi64(digits, 0);

        constexpr uint64_t powers[8] = {10000000, 1000000, 100000, 10000, 1000, 100, 10, 1};
        uint64_t result = 0;

        for (int i = 0; i < 8; i++) {
            result += ((raw >> (i * 8)) & 0xFF) * powers[i];
        }

        return result;
#elif defined(HAS_SSE42)
        __m128i chunk = _mm_loadl_epi64(reinterpret_cast<const __m128i *>(data));
        __m128i zero = _mm_set1_epi8('0');
        __m128i digits = _mm_sub_epi8(chunk, zero);

        uint64_t raw = _mm_extract_epi64(digits, 0);

        uint64_t result = 0;
        result += ((raw >> 0) & 0xFF) * 10000000ULL;   // data[0]
        result += ((raw >> 8) & 0xFF) * 1000000ULL;    // data[1]
        result += ((raw >> 16) & 0xFF) * 100000ULL;    // data[2]
        result += ((raw >> 24) & 0xFF) * 10000ULL;     // data[3]
        result += ((raw >> 32) & 0xFF) * 1000ULL;      // data[4]
        result += ((raw >> 40) & 0xFF) * 100ULL;       // data[5]
        result += ((raw >> 48) & 0xFF) * 10ULL;        // data[6]
        result += ((raw >> 56) & 0xFF) * 1ULL;         // data[7]

        return result;
#elif defined(HAS_NEON)
        uint8x8_t chunk = vld1_u8(reinterpret_cast<const uint8_t*>(data));
        uint8x8_t zero = vdup_n_u8('0');
        uint8x8_t digits = vsub_u8(chunk, zero);

        uint16x8_t digits_wide = vmovl_u8(digits);
        uint32x4_t digits_low = vmovl_u16(vget_low_u16(digits_wide));
        uint32x4_t digits_high = vmovl_u16(vget_high_u16(digits_wide));

        static const uint32x4_t multipliers_low = {10000000, 1000000, 100000, 10000};
        static const uint32x4_t multipliers_high = {1000, 100, 10, 1};

        uint32x4_t products_low = vmulq_u32(digits_low, multipliers_low);
        uint32x4_t products_high = vmulq_u32(digits_high, multipliers_high);
#ifdef __aarch64__
        uint32_t sum_low = vaddvq_u32(products_low);
        uint32_t sum_high = vaddvq_u32(products_high);
        return static_cast<uint64_t>(sum_low) + static_cast<uint64_t>(sum_high);
#else
        uint64x2_t sum_low_pairs = vpaddlq_u32(products_low);
        uint64x2_t sum_high_pairs = vpaddlq_u32(products_high);

        uint64_t result = vgetq_lane_u64(sum_low_pairs, 0) + vgetq_lane_u64(sum_low_pairs, 1);
        result += vgetq_lane_u64(sum_high_pairs, 0) + vgetq_lane_u64(sum_high_pairs, 1);
        return result;
#endif

#else
        uint64_t result = 0;

        result += (data[0] - '0') * 10000000ULL;
        result += (data[1] - '0') * 1000000ULL;
        result += (data[2] - '0') * 100000ULL;
        result += (data[3] - '0') * 10000ULL;
        result += (data[4] - '0') * 1000ULL;
        result += (data[5] - '0') * 100ULL;
        result += (data[6] - '0') * 10ULL;
        result += (data[7] - '0') * 1ULL;

        return result;
#endif
    }

    static double standard_parse(std::string_view str) {
        std::string null_terminated(str);
        char *end;
        return strtod(null_terminated.c_str(), &end);
    }

    double parse_float(std::string_view str) {
        const char *p = str.data();
        const char *end = str.data() + str.size();

        bool negative = false;
        if (*p == '-') {
            negative = true;
            p++;
        } else if (*p == '+') {
            p++;
        }

        uint64_t integer_part = 0;
        int integer_digits = 0;
        while (p + 8 <= end && all_digits_simd(std::string_view(p, 8))) {
            if (integer_digits >= 10) {
                return standard_parse(str);
            }
            integer_part = integer_part * 100000000ULL + parse_8_digits_simd(std::string_view(p, 8));
            p += 8;
            integer_digits += 8;
        }

        while (p < end && *p >= '0' && *p <= '9') {
            integer_part = integer_part * 10 + (*p - '0');
            p++;
            integer_digits++;
        }

        if (p >= end || *p != '.') {
            double result = static_cast<double>(integer_part);
            return negative ? -result : result;
        }

        p++; // Skip '.'
        uint64_t fractional_part = 0;
        int fractional_digits = 0;
        if (p + 8 <= end && all_digits_simd(std::string_view(p, 8))) {
            fractional_part = parse_8_digits_simd(std::string_view(p, 8));
            fractional_digits = 8;
            p += 8;
            while (p < end && *p == '0') p++;
        } else {
            while (p < end && *p >= '0' && *p <= '9' && fractional_digits < 18) {
                fractional_part = fractional_part * 10 + (*p - '0');
                p++;
                fractional_digits++;
            }
        }

        double result = static_cast<double>(integer_part);
        if (fractional_digits > 0) {
            result += static_cast<double>(fractional_part) /
                    powers_of_10[fractional_digits];
        }

        return negative ? -result : result;
    }

} // namespace core::fast_float_parser