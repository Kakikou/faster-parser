/**
 * @file parsers.h
 * @author Kevin Rodrigues
 * @brief Fast floating-point parser with SIMD optimizations
 * @version 1.0
 * @date 11/09/2025
 */

#ifndef FASTER_PARSER_PARSERS_H
#define FASTER_PARSER_PARSERS_H

#include <cstdint>
#include <string_view>

#ifdef __x86_64__
#include <immintrin.h>
#ifdef __AVX2__
#endif
#ifdef __AVX512F__
#endif
#elif defined(__aarch64__) || defined(__ARM_NEON)
#include <arm_neon.h>
#endif

namespace core::fast_float_parser {

    /**
     * @brief High-performance floating-point parser with SIMD optimizations
     * @param str String view containing the number to parse
     * @return Parsed floating-point number
     */
    double parse_float(std::string_view str);

}

#endif // FASTER_PARSER_PARSERS_H