/**
 * @file float_parser_avx512.h
 * @author Kevin Rodrigues
 * @brief AVX-512 optimized floating-point parser implementation
 * @version 1.0
 * @date 10/10/2025
 */

#ifndef FASTER_PARSER_CORE_AVX512_FLOAT_PARSER_AVX512_H
#define FASTER_PARSER_CORE_AVX512_FLOAT_PARSER_AVX512_H

#include <cstdint>
#include <string_view>

#ifdef __AVX512F__
#include <immintrin.h>
#endif

namespace core::avx512 {
    bool all_digits(std::string_view str);

    uint64_t parse_8_digits(std::string_view str);

    double parse_float(std::string_view str);

    uint64_t parse_uint64(std::string_view str);
} // namespace core::avx512

#endif // FASTER_PARSER_CORE_AVX512_FLOAT_PARSER_AVX512_H
