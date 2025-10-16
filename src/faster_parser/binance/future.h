/**
 * @file future.h
 * @author Kevin Rodrigues
 * @brief SIMD-optimized parser for Binance Futures book ticker messages
 * @version 1.0
 * @date 10/10/2025
 */

#ifndef FASTER_PARSER_FUTURE_H
#define FASTER_PARSER_FUTURE_H

#include <chrono>
#include <string_view>

#include "faster_parser/binance/concepts.h"

#if defined(__AVX512F__)
#include "avx512/utils_avx512.h"
#include "avx512/book_ticker_avx512.h"
namespace impl = core::faster_parser::binance::avx512;
#elif defined(__AVX2__)
#include "avx2/utils_avx2.h"
#include "avx2/book_ticker_avx2.h"
namespace impl = core::faster_parser::binance::avx2;
#elif defined(__aarch64__) || defined(__ARM_NEON)
#include "neon/utils_neon.h"
#include "neon/book_ticker_neon.h"
namespace impl = core::faster_parser::binance::neon;
#else
#include "scalar/utils_scalar.h"
#include "scalar/book_ticker_scalar.h"
namespace impl = core::faster_parser::binance::scalar;
#endif

namespace core::faster_parser::binance {
    class binance_future_parser_t {
    public:
        template<BinanceFutureListener listener_t>
        static __attribute__((always_inline)) inline bool parse(std::chrono::system_clock::time_point const &now, std::string_view raw, listener_t &listener) {
            if (raw.size() < 20) {
                return false;
            }
            if (!impl::match_string(raw.data(), R"({"e":"bookTicker)", 16)) {
                return false;
            }
            process_book_ticker(now, raw, listener);
            return true;
        }

        template<BinanceFutureListener listener_t>
        static __attribute__((always_inline)) inline bool process_book_ticker(std::chrono::system_clock::time_point const &now, std::string_view raw, listener_t &listener) {
            // Message example: {"e":"bookTicker","u":8822354685185,"s":"ASTERUSDT","b":"1.5822000","B":"457","a":"1.5823000","A":"112","T":1760083106579,"E":1760083106579}
            return impl::process_book_ticker(now, raw, listener);
        }
    };
} // core::faster_parser::binance

#endif //FASTER_PARSER_FUTURE_H
