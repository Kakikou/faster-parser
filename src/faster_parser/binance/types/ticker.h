/**
 * @file ticker.h
 * @author Kevin Rodrigues
 * @brief 
 * @version 1.0
 * @date 27/10/2025
 */

#ifndef FASTER_PARSER_TICKER_H
#define FASTER_PARSER_TICKER_H

#include <chrono>
#include <cstdint>
#include <string_view>

namespace core::faster_parser::binance::types {

    /**
     * @brief 24hr ticker statistics structure for Binance Futures
     * Corresponds to the "24hrTicker" event type from Binance WebSocket API
     * Contains 24-hour rolling window statistics
     */
    struct ticker_t {
        std::chrono::system_clock::time_point time;     // Reception time
        std::string_view symbol;                        // Symbol (zero-copy reference)
        uint64_t event_time;                            // Event time (E)
        double price_change;                            // Price change (p)
        double price_change_percent;                    // Price change percent (P)
        double weighted_avg_price;                      // Weighted average price (w)
        double last_price;                              // Last price (c)
        double last_quantity;                           // Last quantity (Q)
        double open_price;                              // Open price (o)
        double high_price;                              // High price (h)
        double low_price;                               // Low price (l)
        double total_traded_base_volume;                // Total traded base volume (v)
        double total_traded_quote_volume;               // Total traded quote volume (q)
        uint64_t statistics_open_time;                  // Statistics open time (O)
        uint64_t statistics_close_time;                 // Statistics close time (C)
        uint64_t first_trade_id;                        // First trade ID (F)
        uint64_t last_trade_id;                         // Last trade ID (L)
        uint64_t total_trades;                          // Total number of trades (n)
    };

}

#endif //FASTER_PARSER_TICKER_H
