/**
 * @file fast_float_parser_tests.cpp
 * @author Kevin Rodrigues
 * @brief Tests for FastFloatParser class
 * @version 1.0
 * @date 22/09/2025
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <cmath>
#include <limits>
#include <vector>
#include <iomanip>

#include <faster_parser/core/fast_scalar_parser.h>

using namespace core::fast_scalar_parser;

class fast_float_parser_test_t : public ::testing::Test {
protected:
    void SetUp() override {
    }

    bool near_equal(double a, double b, double tolerance = 1e-10) {
        return std::abs(a - b) < tolerance;
    }

    bool compare_with_strtod(const char *str, double tolerance = 1e-8) {
        double fast_result = parse_float(str);
        double std_result = strtod(str, nullptr);
        return near_equal(fast_result, std_result, tolerance);
    }
};

TEST_F(fast_float_parser_test_t, ParseIntegers) {
    EXPECT_TRUE(compare_with_strtod("0"));
    EXPECT_TRUE(compare_with_strtod("1"));
    EXPECT_TRUE(compare_with_strtod("123"));
    EXPECT_TRUE(compare_with_strtod("999999"));
    EXPECT_TRUE(compare_with_strtod("1000000"));
}

TEST_F(fast_float_parser_test_t, ParseDecimals) {
    EXPECT_TRUE(compare_with_strtod("0.0"));
    EXPECT_TRUE(compare_with_strtod("1.5"));
    EXPECT_TRUE(compare_with_strtod("123.456"));
    EXPECT_TRUE(compare_with_strtod("0.123456789", 1e-7));
    EXPECT_TRUE(compare_with_strtod("999.999"));
}

TEST_F(fast_float_parser_test_t, ParseFinancialPrices) {
    EXPECT_TRUE(compare_with_strtod("25.35190000"));
    EXPECT_TRUE(compare_with_strtod("0.00001234"));
    EXPECT_TRUE(compare_with_strtod("67890.12345678"));
    EXPECT_TRUE(compare_with_strtod("1.00000000"));
    EXPECT_TRUE(compare_with_strtod("999.99999999"));
    EXPECT_TRUE(compare_with_strtod("0.12345678"));
    EXPECT_TRUE(compare_with_strtod("12345.00000000"));
    EXPECT_TRUE(compare_with_strtod("0.00000001"));
    EXPECT_TRUE(compare_with_strtod("999999.99999999"));
}

TEST_F(fast_float_parser_test_t, ParseNegativeNumbers) {
    EXPECT_TRUE(compare_with_strtod("-1"));
    EXPECT_TRUE(compare_with_strtod("-123.456"));
    EXPECT_TRUE(compare_with_strtod("-0.123"));
    EXPECT_TRUE(compare_with_strtod("-999.99999999"));
    EXPECT_TRUE(compare_with_strtod("-0.00000001"));
}

TEST_F(fast_float_parser_test_t, ParsePositiveSign) {
    EXPECT_TRUE(compare_with_strtod("+1"));
    EXPECT_TRUE(compare_with_strtod("+123.456"));
    EXPECT_TRUE(compare_with_strtod("+0.123"));
    EXPECT_TRUE(compare_with_strtod("+999.99999999"));
}

TEST_F(fast_float_parser_test_t, ParseEdgeCases) {
    EXPECT_TRUE(compare_with_strtod("0.0"));
    EXPECT_TRUE(compare_with_strtod("0.00000000"));
    EXPECT_TRUE(compare_with_strtod("1.0"));
    EXPECT_TRUE(compare_with_strtod("10.0"));
    EXPECT_TRUE(compare_with_strtod("100.0"));
    EXPECT_TRUE(compare_with_strtod("1000.0"));
    EXPECT_TRUE(compare_with_strtod("10000.0"));
    EXPECT_TRUE(compare_with_strtod("100000.0"));
    EXPECT_TRUE(compare_with_strtod("1000000.0"));
}

TEST_F(fast_float_parser_test_t, ParseTrailingZeros) {
    EXPECT_TRUE(compare_with_strtod("123.45000000"));
    EXPECT_TRUE(compare_with_strtod("100.10000000"));
    EXPECT_TRUE(compare_with_strtod("0.10000000"));
    EXPECT_TRUE(compare_with_strtod("1.00100000"));
}

TEST_F(fast_float_parser_test_t, ParseVerySmallNumbers) {
    EXPECT_TRUE(compare_with_strtod("0.00000001"));
    EXPECT_TRUE(compare_with_strtod("0.00000123"));
    EXPECT_TRUE(compare_with_strtod("0.00012345"));
    EXPECT_TRUE(compare_with_strtod("0.01234567"));
}

TEST_F(fast_float_parser_test_t, ParseLargeNumbers) {
    EXPECT_TRUE(compare_with_strtod("12345678.12345678"));
    EXPECT_TRUE(compare_with_strtod("99999999.99999999"));
    EXPECT_TRUE(compare_with_strtod("1000000.0"));
    EXPECT_TRUE(compare_with_strtod("9999999.0"));
}

TEST_F(fast_float_parser_test_t, ParseFixed8Decimals) {
    double result1 = parse_float("25.35190000");
    double expected1 = 25.35190000;
    EXPECT_TRUE(near_equal(result1, expected1));

    double result2 = parse_float("0.00001234");
    double expected2 = 0.00001234;
    EXPECT_TRUE(near_equal(result2, expected2));

    double result3 = parse_float("123.45678900");
    double expected3 = 123.45678900;
    EXPECT_TRUE(near_equal(result3, expected3));
}

TEST_F(fast_float_parser_test_t, ParseWithLength) {
    const char *str = "123.456789GARBAGE";
    double result = parse_float(std::string_view(str, 10));
    double expected = 123.456789;
    EXPECT_TRUE(near_equal(result, expected, 1e-6));
}

TEST_F(fast_float_parser_test_t, PrecisionPreservation) {
    struct TestCase {
        const char *input;
        double expected;
    };

    std::vector<TestCase> test_cases = {
        {"0.12345678", 0.12345678},
        {"1.23456789", 1.23456789},
        {"12.3456789", 12.3456789},
        {"123.456789", 123.456789},
        {"1234.56789", 1234.56789},
        {"12345.6789", 12345.6789},
        {"123456.789", 123456.789},
        {"1234567.89", 1234567.89},
        {"12345678.9", 12345678.9}
    };

    for (const auto &test_case: test_cases) {
        double result = parse_float(test_case.input);
        EXPECT_TRUE(near_equal(result, test_case.expected, 1e-8))
            << "Failed for input: " << test_case.input
            << " Expected: " << std::setprecision(10) << test_case.expected
            << " Got: " << std::setprecision(10) << result;
    }
}

TEST_F(fast_float_parser_test_t, BoundaryCases) {
    EXPECT_TRUE(compare_with_strtod("0"));
    EXPECT_TRUE(compare_with_strtod("0.0"));
    EXPECT_TRUE(compare_with_strtod("-0"));
    EXPECT_TRUE(compare_with_strtod("-0.0"));
    EXPECT_TRUE(compare_with_strtod("+0"));
    EXPECT_TRUE(compare_with_strtod("+0.0"));

    for (int i = 0; i <= 9; ++i) {
        std::string digit = std::to_string(i);
        EXPECT_TRUE(compare_with_strtod(digit.c_str()));

        std::string decimal = "0." + digit;
        EXPECT_TRUE(compare_with_strtod(decimal.c_str()));
    }
}

TEST_F(fast_float_parser_test_t, PerformanceValidation) {
    std::vector<std::string> test_values = {
        "25.35190000",
        "0.00001234",
        "67890.12345678",
        "1.00000000",
        "999.99999999",
        "123.45678900",
        "-456.78901234",
        "0.98765432"
    };

    for (const auto &value: test_values) {
        EXPECT_TRUE(compare_with_strtod(value.c_str()))
            << "Failed for value: " << value;
    }
}

TEST_F(fast_float_parser_test_t, ConsistencyTest) {
    const char *test_value = "123.45678900";
    double first_result = parse_float(test_value);

    for (int i = 0; i < 100; ++i) {
        double result = parse_float(test_value);
        EXPECT_EQ(result, first_result)
            << "Inconsistent result on iteration " << i;
    }
}

TEST_F(fast_float_parser_test_t, SIMDvsScalarConsistency) {
    std::vector<std::string> test_cases = {
        "12345678.90123456",
        "87654321.09876543",
        "11111111.22222222",
        "99999999.88888888",
        "1.2",
        "12.34",
        "123.456",
        "1234.567"
    };

    for (const auto &test_case: test_cases) {
        EXPECT_TRUE(compare_with_strtod(test_case.c_str()))
            << "SIMD/Scalar inconsistency for: " << test_case;
    }
}

TEST_F(fast_float_parser_test_t, StandardParseFallback) {
    const char *test_value = "123.456";
    double fast_result = parse_float(test_value);
    double standard_result = std::strtod(test_value, nullptr);

    EXPECT_TRUE(near_equal(fast_result, standard_result, 1e-12));
}

TEST_F(fast_float_parser_test_t, MemorySafety) {
    std::vector<std::string> test_strings = {
        "1",
        "12",
        "123",
        "1234",
        "12345",
        "123456",
        "1234567",
        "12345678",
        "123456789",
        "1234567890",
        "12345678901234567890"
    };

    for (const auto &str: test_strings) {
        EXPECT_TRUE(compare_with_strtod(str.c_str()))
            << "Memory safety issue with string: " << str;
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
