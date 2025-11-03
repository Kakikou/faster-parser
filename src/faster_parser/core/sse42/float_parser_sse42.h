/**
 * @file float_parser_sse42.h
 * @author Kevin Rodrigues
 * @brief SSE4.2 optimized floating-point parser implementation
 * @version 1.0
 * @date 10/10/2025
 */

#ifndef FASTER_PARSER_CORE_SSE42_FLOAT_PARSER_SSE42_H
#define FASTER_PARSER_CORE_SSE42_FLOAT_PARSER_SSE42_H

#include <cstdint>
#include <string_view>

#ifdef __SSE4_2__
#include <nmmintrin.h>
#endif

namespace core::sse42 {
    bool all_digits(std::string_view str);

    uint64_t parse_8_digits(std::string_view str);

    double parse_float(std::string_view str);

    uint64_t parse_uint64(std::string_view str);
} // namespace core::sse42

#endif // FASTER_PARSER_CORE_SSE42_FLOAT_PARSER_SSE42_H
