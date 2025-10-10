/**
 * @file float_parser_neon.h
 * @author Kevin Rodrigues
 * @brief NEON-optimized floating-point parser implementation
 * @version 1.0
 * @date 10/10/2025
 */

#ifndef FASTER_PARSER_CORE_NEON_FLOAT_PARSER_NEON_H
#define FASTER_PARSER_CORE_NEON_FLOAT_PARSER_NEON_H

#include <cstdint>
#include <string_view>

#ifdef __aarch64__
#include <arm_neon.h>
#endif

namespace core::neon {
    bool all_digits(std::string_view str);

    uint64_t parse_8_digits(std::string_view str);

    double parse_float(std::string_view str);

    uint64_t parse_uint64(std::string_view str);
} // namespace core::neon

#endif // FASTER_PARSER_CORE_NEON_FLOAT_PARSER_NEON_H
