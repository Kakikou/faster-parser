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

namespace core::faster_parser::binance {
    /**
     * @brief Concept defining the requirements for a Binance market data listener
     * @tparam T The type to be checked against the concept
     *
     * A type satisfies BinanceListener if it provides an on_book_ticker method
     * that accepts a book_ticker_t parameter (by value or reference).
     * Additional callback methods can be added in the future (e.g., on_trade, on_depth).
     */
    template<typename T>
    concept BinanceFutureListener = requires(T &listener, const types::book_ticker_t &ticker)
    {
        { listener.on_book_ticker(ticker) } -> std::same_as<void>;
    };
} // namespace core::faster_parser::binance

#endif //FASTER_PARSER_CONCEPTS_H
