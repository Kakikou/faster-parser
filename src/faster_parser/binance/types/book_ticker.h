/**
 * @file book_ticker.h
 * @author Kevin Rodrigues
 * @brief 
 * @version 1.0
 * @date 10/10/2025
 */

#ifndef FASTER_PARSER_BOOK_TICKER_H
#define FASTER_PARSER_BOOK_TICKER_H

#include <chrono>
#include <string_view>
#include "level_data.h"

namespace core::faster_parser::binance::types {

    struct book_ticker_t {
        book_ticker_t() = default;
        book_ticker_t(book_ticker_t const &) = default;
        book_ticker_t& operator=(book_ticker_t const &) = default;

        std::chrono::system_clock::time_point time;
        std::string_view symbol;  // Zero-copy reference to original JSON
        uint64_t exchange_timestamp;
        level_data_t bid;
        level_data_t ask;
    };

} // namespace core::faster_parser::binance::types

#endif //FASTER_PARSER_BOOK_TICKER_H