/**
 * @file book_ticker.h
 * @author Kevin Rodrigues
 * @brief Book ticker data structure for Binance Futures
 * @version 1.0
 * @date 10/10/2025
 */

#ifndef FASTER_PARSER_BOOK_TICKER_H
#define FASTER_PARSER_BOOK_TICKER_H

#include <chrono>
#include <string_view>

namespace core::faster_parser::binance::types {

    /**
     * @brief Order book level data (bid or ask)
     * Represents a single price level in the order book
     */
    struct level_data_t {
        level_data_t() = default;

        level_data_t(uint64_t p, double v, double s) : price(p), volume(v), sequence(s) {}

        double price = 0.;      // Price level
        double volume = 0.;     // Volume available at this price level
        uint64_t sequence = 0;  // Update sequence number
    };

    /**
     * @brief Book ticker structure for Binance Futures
     * Corresponds to the "bookTicker" event type from Binance WebSocket API
     * Contains best bid/ask prices and volumes
     */
    struct book_ticker_t {
        book_ticker_t() = default;
        book_ticker_t(book_ticker_t const &) = default;
        book_ticker_t& operator=(book_ticker_t const &) = default;

        std::chrono::system_clock::time_point time;     // Reception time
        std::string_view symbol;                        // Symbol (zero-copy reference)
        uint64_t exchange_timestamp;                    // Exchange timestamp (E)
        level_data_t bid;                               // Best bid level
        level_data_t ask;                               // Best ask level
    };

} // namespace core::faster_parser::binance::types

#endif //FASTER_PARSER_BOOK_TICKER_H