/**
 * @file float_parser_scalar.h
 * @author Kevin Rodrigues
 * @brief Scalar floating-point parser implementation (fallback)
 * @version 1.0
 * @date 10/10/2025
 */

#ifndef FASTER_PARSER_CORE_SCALAR_FLOAT_PARSER_SCALAR_H
#define FASTER_PARSER_CORE_SCALAR_FLOAT_PARSER_SCALAR_H

#include <cstdint>
#include <string_view>

namespace core::scalar {
    bool all_digits(std::string_view str);

    uint64_t parse_8_digits(std::string_view str);

    double parse_float(std::string_view str);

    uint64_t parse_uint64(std::string_view str);
} // namespace core::scalar

#endif // FASTER_PARSER_CORE_SCALAR_FLOAT_PARSER_SCALAR_H
