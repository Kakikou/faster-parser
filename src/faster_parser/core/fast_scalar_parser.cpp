/**
 * @file fast_scalar_parser.cpp
 * @author Kevin Rodrigues
 * @brief Fast scalar parser - dispatcher implementation
 * @version 1.0
 * @date 10/10/2025
 */

#include "fast_scalar_parser.h"

#if defined(__AVX512F__)
    #include "avx512/float_parser_avx512.h"
    namespace impl = core::avx512;
#elif defined(__AVX2__)
    #include "avx2/float_parser_avx2.h"
    namespace impl = core::avx2;
#elif defined(__SSE4_2__)
    #include "sse42/float_parser_sse42.h"
    namespace impl = core::sse42;
#elif defined(__aarch64__) || defined(__ARM_NEON)
    #include "neon/float_parser_neon.h"
    namespace impl = core::neon;
#else
    #include "scalar/float_parser_scalar.h"
    namespace impl = core::scalar;
#endif

namespace core::fast_scalar_parser {

    double parse_float(std::string_view str) {
        return impl::parse_float(str);
    }

    uint64_t parse_uint64(std::string_view str) {
        return impl::parse_uint64(str);
    }

} // namespace core::fast_scalar_parser
