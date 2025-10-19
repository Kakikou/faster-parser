/**
 * @file future.h
 * @author Kevin Rodrigues
 * @brief SIMD-optimized parser for Binance Futures market data messages
 * @version 1.0
 * @date 10/10/2025
 */

#ifndef FASTER_PARSER_FUTURE_H
#define FASTER_PARSER_FUTURE_H

#include <chrono>
#include <string_view>

#include "faster_parser/binance/concepts.h"
#include "faster_parser/core/fast_scalar_parser.h"

#if defined(__AVX512F__)
#include "avx512/utils_avx512.h"
namespace impl = core::faster_parser::binance::avx512;
#elif defined(__AVX2__)
#include "avx2/utils_avx2.h"
namespace impl = core::faster_parser::binance::avx2;
#elif defined(__aarch64__) || defined(__ARM_NEON)
#include "neon/utils_neon.h"
namespace impl = core::faster_parser::binance::neon;
#else
#include "scalar/utils_scalar.h"
namespace impl = core::faster_parser::binance::scalar;
#endif

namespace core::faster_parser::binance {
    class binance_future_parser_t {
    public:
        template<BinanceFutureListener listener_t>
        static __attribute__((always_inline)) bool parse(std::chrono::system_clock::time_point const &now, std::string_view raw, listener_t &listener) {
            if (raw.size() < 20) {
                return false;
            }

            // Check message type
            if (impl::match_string(raw.data(), R"({"e":"bookTicker)", 16)) {
                return process_book_ticker(now, raw, listener);
            } else if (impl::match_string(raw.data(), R"({"e":"aggTrade)", 14)) {
                return process_agg_trade(now, raw, listener);
            }

            return false;
        }

        template<BinanceFutureListener listener_t>
        static __attribute__((always_inline)) bool process_book_ticker(std::chrono::system_clock::time_point const &now, std::string_view raw, listener_t &listener) {
            // Message example: {"e":"bookTicker","u":8822354685185,"s":"ASTERUSDT","b":"1.5822000","B":"457","a":"1.5823000","A":"112","T":1760083106579,"E":1760083106579}
            types::book_ticker_t ticker;
            ticker.time = now;

            const char *ptr = raw.data();
            const char *end = raw.data() + raw.size();

            ptr = impl::find_char(ptr, end, 'u');
            if (!ptr) return false;
            ptr += 3;

            const char *value_start = ptr;
            ptr = impl::find_char(ptr, end, ',');
            if (!ptr) return false;
            uint64_t update_id = fast_scalar_parser::parse_uint64(std::string_view(value_start, ptr));

            ptr = impl::find_char(ptr, end, 's');
            if (!ptr) return false;
            ptr += 4;

            value_start = ptr;
            ptr = impl::find_char(ptr, end, '"');
            if (!ptr) return false;
            ticker.symbol = std::string_view(value_start, ptr - value_start);
            ptr++;

            ptr = impl::find_char(ptr, end, 'b');
            if (!ptr) return false;
            ptr += 4;

            value_start = ptr;
            ptr = impl::find_char(ptr, end, '"');
            if (!ptr) return false;
            ticker.bid.price = fast_scalar_parser::parse_float(std::string_view(value_start, ptr));
            ptr++;

            ptr = impl::find_char(ptr, end, 'B');
            if (!ptr) return false;
            ptr += 4;

            value_start = ptr;
            ptr = impl::find_char(ptr, end, '"');
            if (!ptr) return false;
            ticker.bid.volume = fast_scalar_parser::parse_float(std::string_view(value_start, ptr));
            ptr++;

            ptr = impl::find_char(ptr, end, 'a');
            if (!ptr) return false;
            ptr += 4;

            value_start = ptr;
            ptr = impl::find_char(ptr, end, '"');
            if (!ptr) return false;
            ticker.ask.price = fast_scalar_parser::parse_float(std::string_view(value_start, ptr));
            ptr++;

            ptr = impl::find_char(ptr, end, 'A');
            if (!ptr) return false;
            ptr += 4;

            value_start = ptr;
            ptr = impl::find_char(ptr, end, '"');
            if (!ptr) return false;
            ticker.ask.volume = fast_scalar_parser::parse_float(std::string_view(value_start, ptr));
            ptr++;

            ptr = impl::find_char(ptr, end, 'T');
            if (!ptr) return false;
            ptr = impl::find_char(ptr, end, ',');
            if (!ptr) return false;
            ptr++;

            ptr = impl::find_char(ptr, end, 'E');
            if (!ptr) return false;
            ptr += 3;

            value_start = ptr;
            ptr = impl::find_char(ptr, end, '}');
            if (!ptr) return false;
            ticker.exchange_timestamp = fast_scalar_parser::parse_uint64(std::string_view(value_start, ptr));

            ticker.bid.sequence = update_id;
            ticker.ask.sequence = update_id;

            listener.on_book_ticker(ticker);
            return true;
        }

        template<BinanceFutureListener listener_t>
        static __attribute__((always_inline)) bool process_agg_trade(std::chrono::system_clock::time_point const &now, std::string_view raw, listener_t &listener) {
            // Message example: {"e":"aggTrade","E":123456789,"s":"BTCUSDT","a":5933014,"p":"0.001","q":"100","f":100,"l":105,"T":123456785,"m":true}
            types::agg_trade_t trade;
            trade.time = now;

            const char *ptr = raw.data();
            const char *end = raw.data() + raw.size();

            ptr = impl::find_char(ptr, end, 'E');
            if (!ptr) return false;
            ptr += 3;

            const char *value_start = ptr;
            ptr = impl::find_char(ptr, end, ',');
            if (!ptr) return false;
            trade.event_time = fast_scalar_parser::parse_uint64(std::string_view(value_start, ptr - value_start));

            ptr = impl::find_char(ptr, end, 's');
            if (!ptr) return false;
            ptr += 4;

            value_start = ptr;
            ptr = impl::find_char(ptr, end, '"');
            if (!ptr) return false;
            trade.symbol = std::string_view(value_start, ptr - value_start);
            ptr++;

            ptr = impl::find_char(ptr, end, 'a');
            if (!ptr) return false;
            ptr += 3;

            value_start = ptr;
            ptr = impl::find_char(ptr, end, ',');
            if (!ptr) return false;
            trade.agg_trade_id = fast_scalar_parser::parse_uint64(std::string_view(value_start, ptr - value_start));

            ptr = impl::find_char(ptr, end, 'p');
            if (!ptr) return false;
            ptr += 4;

            value_start = ptr;
            ptr = impl::find_char(ptr, end, '"');
            if (!ptr) return false;
            trade.price = fast_scalar_parser::parse_float(std::string_view(value_start, ptr - value_start));
            ptr++;

            ptr = impl::find_char(ptr, end, 'q');
            if (!ptr) return false;
            ptr += 4;

            value_start = ptr;
            ptr = impl::find_char(ptr, end, '"');
            if (!ptr) return false;
            trade.quantity = fast_scalar_parser::parse_float(std::string_view(value_start, ptr - value_start));
            ptr++;

            ptr = impl::find_char(ptr, end, 'f');
            if (!ptr) return false;
            ptr += 3;

            value_start = ptr;
            ptr = impl::find_char(ptr, end, ',');
            if (!ptr) return false;
            trade.first_trade_id = fast_scalar_parser::parse_uint64(std::string_view(value_start, ptr - value_start));

            ptr = impl::find_char(ptr, end, 'l');
            if (!ptr) return false;
            ptr += 3;

            value_start = ptr;
            ptr = impl::find_char(ptr, end, ',');
            if (!ptr) return false;
            trade.last_trade_id = fast_scalar_parser::parse_uint64(std::string_view(value_start, ptr - value_start));

            ptr = impl::find_char(ptr, end, 'T');
            if (!ptr) return false;
            ptr += 3;

            value_start = ptr;
            ptr = impl::find_char(ptr, end, ',');
            if (!ptr) return false;
            trade.trade_time = fast_scalar_parser::parse_uint64(std::string_view(value_start, ptr - value_start));

            ptr = impl::find_char(ptr, end, 'm');
            if (!ptr) return false;
            ptr += 3;

            trade.is_buyer_maker = (*ptr == 't');
            // if (ptr + 4 <= end && std::string_view(ptr, 4) == "true") {
                // trade.is_buyer_maker = true;
            // } else if (ptr + 5 <= end && std::string_view(ptr, 5) == "false") {
                // trade.is_buyer_maker = false;
            // } else {
                // return false;
            // }

            listener.on_agg_trade(trade);
            return true;
        }
    };
} // core::faster_parser::binance

#endif //FASTER_PARSER_FUTURE_H
