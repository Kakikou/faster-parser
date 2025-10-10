/**
 * @file level_data.h
 * @author Kevin Rodrigues
 * @brief 
 * @version 1.0
 * @date 10/10/2025
 */

#ifndef FASTER_PARSER_LEVEL_DATA_H
#define FASTER_PARSER_LEVEL_DATA_H

#include <cstdint>

namespace core::binance::types {

    struct level_data_t {
        level_data_t() = default;

        level_data_t(uint64_t p, double v, double s) : price(p), volume(v), sequence(s) {}

        double price = 0.;
        double volume = 0.;
        uint64_t sequence = 0;
    };

} // namespace core::binance::types

#endif //FASTER_PARSER_LEVEL_DATA_H