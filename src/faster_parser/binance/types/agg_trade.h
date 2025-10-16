/**
 * @file agg_trade.h
 * @author Kevin Rodrigues
 * @brief Aggregate trade data structure for Binance Futures
 * @version 1.0
 * @date 16/10/2025
 */

#ifndef FASTER_PARSER_AGG_TRADE_H
#define FASTER_PARSER_AGG_TRADE_H

#include <chrono>
#include <string_view>

namespace core::faster_parser::binance::types {

    struct agg_trade_t {
        agg_trade_t() = default;
        agg_trade_t(agg_trade_t const &) = default;
        agg_trade_t& operator=(agg_trade_t const &) = default;

        std::chrono::system_clock::time_point time;  // Reception time
        std::string_view symbol;                      // Symbol (zero-copy reference)
        uint64_t event_time;                          // Event time (E)
        uint64_t agg_trade_id;                        // Aggregate trade ID (a)
        double price;                                 // Price (p)
        double quantity;                              // Quantity (q)
        uint64_t first_trade_id;                      // First trade ID (f)
        uint64_t last_trade_id;                       // Last trade ID (l)
        uint64_t trade_time;                          // Trade time (T)
        bool is_buyer_maker;                          // Is buyer the market maker (m)
    };

} // namespace core::faster_parser::binance::types

#endif //FASTER_PARSER_AGG_TRADE_H
