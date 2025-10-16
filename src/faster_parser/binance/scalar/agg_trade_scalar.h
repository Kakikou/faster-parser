/**
 * @file agg_trade_scalar.h
 * @author Kevin Rodrigues
 * @brief Scalar fallback Binance aggregate trade parser
 * @version 1.0
 * @date 16/10/2025
 */

#ifndef FASTER_PARSER_AGG_TRADE_SCALAR_H
#define FASTER_PARSER_AGG_TRADE_SCALAR_H

#include <chrono>
#include <string_view>

#include "faster_parser/binance/concepts.h"
#include "faster_parser/binance/scalar/utils_scalar.h"
#include "faster_parser/core/fast_scalar_parser.h"
#include "faster_parser/binance/types/agg_trade.h"

namespace core::faster_parser::binance::scalar {
    template<BinanceFutureListener listener_t>
    __attribute__((always_inline)) inline bool process_agg_trade(std::chrono::system_clock::time_point const &now, std::string_view raw, listener_t &listener) {
        types::agg_trade_t trade;
        trade.time = now;

        const char *ptr = raw.data();
        const char *end = raw.data() + raw.size();

        // Parse "E" (event time)
        ptr = find_char(ptr, end, 'E');
        if (!ptr) return false;
        ptr += 3;

        const char *value_start = ptr;
        ptr = find_char(ptr, end, ',');
        if (!ptr) return false;
        trade.event_time = fast_scalar_parser::parse_uint64(std::string_view(value_start, ptr - value_start));

        // Parse "s" (symbol)
        ptr = find_char(ptr, end, 's');
        if (!ptr) return false;
        ptr += 4;

        value_start = ptr;
        ptr = find_char(ptr, end, '"');
        if (!ptr) return false;
        trade.symbol = std::string_view(value_start, ptr - value_start);
        ptr++;

        // Parse "a" (aggregate trade ID)
        ptr = find_char(ptr, end, 'a');
        if (!ptr) return false;
        ptr += 3;

        value_start = ptr;
        ptr = find_char(ptr, end, ',');
        if (!ptr) return false;
        trade.agg_trade_id = fast_scalar_parser::parse_uint64(std::string_view(value_start, ptr - value_start));

        // Parse "p" (price)
        ptr = find_char(ptr, end, 'p');
        if (!ptr) return false;
        ptr += 4;

        value_start = ptr;
        ptr = find_char(ptr, end, '"');
        if (!ptr) return false;
        trade.price = fast_scalar_parser::parse_float(std::string_view(value_start, ptr - value_start));
        ptr++;

        // Parse "q" (quantity)
        ptr = find_char(ptr, end, 'q');
        if (!ptr) return false;
        ptr += 4;

        value_start = ptr;
        ptr = find_char(ptr, end, '"');
        if (!ptr) return false;
        trade.quantity = fast_scalar_parser::parse_float(std::string_view(value_start, ptr - value_start));
        ptr++;

        // Parse "f" (first trade ID)
        ptr = find_char(ptr, end, 'f');
        if (!ptr) return false;
        ptr += 3;

        value_start = ptr;
        ptr = find_char(ptr, end, ',');
        if (!ptr) return false;
        trade.first_trade_id = fast_scalar_parser::parse_uint64(std::string_view(value_start, ptr - value_start));

        // Parse "l" (last trade ID)
        ptr = find_char(ptr, end, 'l');
        if (!ptr) return false;
        ptr += 3;

        value_start = ptr;
        ptr = find_char(ptr, end, ',');
        if (!ptr) return false;
        trade.last_trade_id = fast_scalar_parser::parse_uint64(std::string_view(value_start, ptr - value_start));

        // Parse "T" (trade time)
        ptr = find_char(ptr, end, 'T');
        if (!ptr) return false;
        ptr += 3;

        value_start = ptr;
        ptr = find_char(ptr, end, ',');
        if (!ptr) return false;
        trade.trade_time = fast_scalar_parser::parse_uint64(std::string_view(value_start, ptr - value_start));

        // Parse "m" (is buyer maker)
        ptr = find_char(ptr, end, 'm');
        if (!ptr) return false;
        ptr += 3;

        // Check if value is "true" or "false"
        if (ptr + 4 <= end && std::string_view(ptr, 4) == "true") {
            trade.is_buyer_maker = true;
        } else if (ptr + 5 <= end && std::string_view(ptr, 5) == "false") {
            trade.is_buyer_maker = false;
        } else {
            return false;
        }

        listener.on_agg_trade(trade);
        return true;
    }
} // namespace core::faster_parser::binance::scalar

#endif //FASTER_PARSER_AGG_TRADE_SCALAR_H
