/**
 * @file float_parser_avx2.h
 * @author Kevin Rodrigues
 * @brief AVX2 optimized floating-point parser implementation
 * @version 1.0
 * @date 10/10/2025
 */

#ifndef FASTER_PARSER_CORE_AVX2_FLOAT_PARSER_AVX2_H
#define FASTER_PARSER_CORE_AVX2_FLOAT_PARSER_AVX2_H

#include <cstdint>
#include <string_view>

#ifdef __AVX2__
#include <immintrin.h>
#endif

namespace core::avx2 {
    bool all_digits(std::string_view str);

    uint64_t parse_8_digits(std::string_view str);

    double parse_float(std::string_view str);

    uint64_t parse_uint64(std::string_view str);
} // namespace core::avx2

#endif // FASTER_PARSER_CORE_AVX2_FLOAT_PARSER_AVX2_H
