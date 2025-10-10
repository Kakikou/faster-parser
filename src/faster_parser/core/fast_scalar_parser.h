/**
 * @file fast_scalar_parser.h
 * @author Kevin Rodrigues
 * @brief Fast scalar parser with SIMD optimizations - dispatcher interface
 * @version 1.0
 * @date 10/10/2025
 */

#ifndef FASTER_PARSER_CORE_FAST_SCALAR_PARSER_H
#define FASTER_PARSER_CORE_FAST_SCALAR_PARSER_H

#include <cstdint>
#include <string_view>

namespace core::fast_scalar_parser {
    double parse_float(std::string_view str);

    uint64_t parse_uint64(std::string_view str);
} // namespace core::fast_scalar_parser

#endif // FASTER_PARSER_CORE_FAST_SCALAR_PARSER_H
