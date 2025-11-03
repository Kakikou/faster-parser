/**
 * @file float_parser.cpp
 * @author Kevin Rodrigues
 * @brief NEON-optimized floating-point parser implementation
 * @version 1.0
 * @date 10/10/2025
 */

#include "float_parser_neon.h"
#include <string>

#ifdef __aarch64__
#include <arm_neon.h>
#endif

namespace core::neon {
    const double powers_of_10[19] = {
        1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9,
        1e10, 1e11, 1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18
    };

    bool all_digits(std::string_view str) {
        const char *data = str.data();
        const size_t len = str.size();

#ifdef __aarch64__
        if (len >= 16) {
            uint8x16_t chunk = vld1q_u8((uint8_t*)data);
            uint8x16_t zero = vdupq_n_u8('0');
            uint8x16_t nine = vdupq_n_u8('9');

            uint8x16_t ge_zero = vcgeq_u8(chunk, zero);
            uint8x16_t le_nine = vcleq_u8(chunk, nine);
            uint8x16_t valid = vandq_u8(ge_zero, le_nine);

            uint8_t min_result = vminvq_u8(valid);
            return min_result == 0xFF;
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

    uint64_t parse_8_digits(std::string_view str) {
        const char *data = str.data();

#ifdef __aarch64__
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

        uint32_t sum_low = vaddvq_u32(products_low);
        uint32_t sum_high = vaddvq_u32(products_high);
        return static_cast<uint64_t>(sum_low) + static_cast<uint64_t>(sum_high);
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
        while (p + 8 <= end && all_digits(std::string_view(p, 8))) {
            if (integer_digits >= 10) {
                return standard_parse(str);
            }
            integer_part = integer_part * 100000000ULL + parse_8_digits(std::string_view(p, 8));
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

        p++;
        uint64_t fractional_part = 0;
        int fractional_digits = 0;
        if (p + 8 <= end && all_digits(std::string_view(p, 8))) {
            fractional_part = parse_8_digits(std::string_view(p, 8));
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
            result += static_cast<double>(fractional_part) / powers_of_10[fractional_digits];
        }

        return negative ? -result : result;
    }

    uint64_t parse_uint64(std::string_view str) {
        uint64_t result = 0;
        const char *ptr = str.data();
        const char *end = str.data() + str.size();

        while (ptr + 8 <= end) {
            bool all_digits = true;
            for (int i = 0; i < 8; i++) {
                if (ptr[i] < '0' || ptr[i] > '9') {
                    all_digits = false;
                    break;
                }
            }

            if (all_digits) {
                result = result * 100000000ULL + parse_8_digits(ptr);
                ptr += 8;
            } else {
                break;
            }
        }

        while (ptr < end && *ptr >= '0' && *ptr <= '9') {
            result = result * 10 + (*ptr - '0');
            ptr++;
        }

        return result;
    }
} // namespace core::neon
