/**
 * @file concepts.h
 * @author Kevin Rodrigues
 * @brief C++20 concepts for Binance market data listeners
 * @version 1.0
 * @date 16/10/2025
 */

#ifndef FASTER_PARSER_CONCEPTS_H
#define FASTER_PARSER_CONCEPTS_H

#include <concepts>

#include "faster_parser/binance/types/book_ticker.h"
#include "faster_parser/binance/types/trade.h"

namespace core::faster_parser::binance {
    /**
     * @brief Concept defining the requirements for a Binance Futures market data listener
     * @tparam T The type to be checked against the concept
     *
     * A type satisfies BinanceFutureListener if it provides callback methods for various
     * market data types. Currently supports:
     * - on_book_ticker: for book ticker updates
     * - on_trade: for aggregate trade data
     *
     * Additional callback methods can be added in the future (e.g., on_depth, on_mark_price).
     */
    template<typename T>
    concept BinanceFutureListener = requires(T &listener,
                                              const types::book_ticker_t &ticker,
                                              const types::trade_t &trade)
    {
        { listener.on_book_ticker(ticker) } -> std::same_as<void>;
        { listener.on_trade(trade) } -> std::same_as<void>;
    };
} // namespace core::faster_parser::binance

#endif //FASTER_PARSER_CONCEPTS_H
