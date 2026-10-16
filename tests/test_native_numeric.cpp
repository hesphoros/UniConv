/**
 * @file test_native_numeric.cpp
 * @brief Tests for UniConv's native numeric parsing and formatting APIs.
 */

#include <gtest/gtest.h>
#include <UniConv/UniConv.h>

#include <cstdint>
#include <cmath>
#include <limits>
#include <string>
#include <string_view>

namespace {

template <typename Result>
void ExpectNumericFailure(const Result& result, ErrorCode expected) {
    ASSERT_FALSE(result.IsSuccess());
    EXPECT_EQ(result.GetErrorCode(), expected);
}

TEST(NativeNumericParseTest, ParsesDecimalAndAutoHexForInt32) {
    const auto decimal = UniConv::ParseInt32(std::string_view{"12345"});
    ASSERT_TRUE(decimal.IsSuccess());
    EXPECT_EQ(decimal.GetValue(), 12345);

    const auto negative = UniConv::ParseInt32("-12345");
    ASSERT_TRUE(negative.IsSuccess());
    EXPECT_EQ(negative.GetValue(), -12345);

    const auto hexadecimal = UniConv::ParseInt32("0x7F");
    ASSERT_TRUE(hexadecimal.IsSuccess());
    EXPECT_EQ(hexadecimal.GetValue(), 127);

    const auto leading_zero = UniConv::ParseInt32("007");
    ASSERT_TRUE(leading_zero.IsSuccess());
    EXPECT_EQ(leading_zero.GetValue(), 7);
}

TEST(NativeNumericParseTest, HonorsExplicitIntegerBaseOptions) {
    const IntegerParseOptions decimal_options{IntegerBase::Decimal};
    const auto decimal = UniConv::ParseInt32("123", decimal_options);
    ASSERT_TRUE(decimal.IsSuccess());
    EXPECT_EQ(decimal.GetValue(), 123);

    const auto decimal_prefix = UniConv::ParseInt32("0x7B", decimal_options);
    ExpectNumericFailure(decimal_prefix, ErrorCode::InvalidNumber);

    const IntegerParseOptions hexadecimal_options{IntegerBase::Hexadecimal};
    const auto hexadecimal = UniConv::ParseInt32("7B", hexadecimal_options);
    ASSERT_TRUE(hexadecimal.IsSuccess());
    EXPECT_EQ(hexadecimal.GetValue(), 123);
}

TEST(NativeNumericParseTest, ParsesInt32Boundaries) {
    const auto minimum = UniConv::ParseInt32(std::to_string(std::numeric_limits<std::int32_t>::min()));
    ASSERT_TRUE(minimum.IsSuccess());
    EXPECT_EQ(minimum.GetValue(), std::numeric_limits<std::int32_t>::min());

    const auto maximum = UniConv::ParseInt32(std::to_string(std::numeric_limits<std::int32_t>::max()));
    ASSERT_TRUE(maximum.IsSuccess());
    EXPECT_EQ(maximum.GetValue(), std::numeric_limits<std::int32_t>::max());
}

TEST(NativeNumericParseTest, ParsesInt64BoundariesAndExplicitLength) {
    const auto minimum_text = std::to_string(std::numeric_limits<std::int64_t>::min());
    const auto maximum_text = std::to_string(std::numeric_limits<std::int64_t>::max());

    const auto minimum = UniConv::ParseInt64(minimum_text.data(), minimum_text.size());
    ASSERT_TRUE(minimum.IsSuccess());
    EXPECT_EQ(minimum.GetValue(), std::numeric_limits<std::int64_t>::min());

    const auto maximum = UniConv::ParseInt64(maximum_text.data(), maximum_text.size());
    ASSERT_TRUE(maximum.IsSuccess());
    EXPECT_EQ(maximum.GetValue(), std::numeric_limits<std::int64_t>::max());

    const char embedded_nul[] = {'1', '2', '\0', '3'};
    const auto bounded = UniConv::ParseInt64(embedded_nul, sizeof(embedded_nul));
    ExpectNumericFailure(bounded, ErrorCode::InvalidNumber);
}

TEST(NativeNumericParseTest, SupportsNativeCharacterViews) {
    const auto utf16 = UniConv::ParseInt32(std::u16string_view{u"-42"});
    ASSERT_TRUE(utf16.IsSuccess());
    EXPECT_EQ(utf16.GetValue(), -42);

    const auto utf32 = UniConv::ParseInt64(std::u32string_view{U"0x2A"});
    ASSERT_TRUE(utf32.IsSuccess());
    EXPECT_EQ(utf32.GetValue(), 42);

    const std::wstring wide_text = L"314159";
    const auto wide = UniConv::ParseInt32(std::wstring_view{wide_text});
    ASSERT_TRUE(wide.IsSuccess());
    EXPECT_EQ(wide.GetValue(), 314159);
}

TEST(NativeNumericParseTest, RejectsMalformedAndOutOfRangeInput) {
    ExpectNumericFailure(UniConv::ParseInt32(std::string_view{}),
                         ErrorCode::InvalidNumber);
    ExpectNumericFailure(UniConv::ParseInt32("12 "), ErrorCode::InvalidNumber);
    ExpectNumericFailure(UniConv::ParseInt32("0b101"), ErrorCode::InvalidNumber);
    ExpectNumericFailure(UniConv::ParseInt32("0x"), ErrorCode::InvalidNumber);
    ExpectNumericFailure(UniConv::ParseInt32("2147483648"), ErrorCode::NumericOverflow);
    ExpectNumericFailure(UniConv::ParseInt64("9223372036854775808"),
                         ErrorCode::NumericOverflow);

    const auto null_input = UniConv::ParseInt64(static_cast<const char*>(nullptr));
    ExpectNumericFailure(null_input, ErrorCode::InvalidParameter);
}

TEST(NativeNumericFormatTest, FormatsInt32IntoAllNativeStringTypes) {
    const auto narrow = UniConv::ToStringFromInt32(-123456);
    ASSERT_TRUE(narrow.IsSuccess());
    EXPECT_EQ(narrow.GetValue(), "-123456");

    const auto utf16 = UniConv::ToUtf16StringFromInt32(-123456);
    ASSERT_TRUE(utf16.IsSuccess());
    EXPECT_EQ(utf16.GetValue(), u"-123456");

    const auto utf32 = UniConv::ToUtf32StringFromInt32(-123456);
    ASSERT_TRUE(utf32.IsSuccess());
    EXPECT_EQ(utf32.GetValue(), U"-123456");

    const auto wide = UniConv::ToWideStringFromInt32(-123456);
    ASSERT_TRUE(wide.IsSuccess());
    EXPECT_EQ(wide.GetValue(), L"-123456");
}

TEST(NativeNumericFormatTest, FormatsInt64Boundaries) {
    const auto minimum = UniConv::ToStringFromInt64(std::numeric_limits<std::int64_t>::min());
    ASSERT_TRUE(minimum.IsSuccess());
    EXPECT_EQ(minimum.GetValue(), "-9223372036854775808");

    const auto maximum = UniConv::ToStringFromInt64(std::numeric_limits<std::int64_t>::max());
    ASSERT_TRUE(maximum.IsSuccess());
    EXPECT_EQ(maximum.GetValue(), "9223372036854775807");

    const auto zero = UniConv::ToUtf16StringFromInt64(0);
    ASSERT_TRUE(zero.IsSuccess());
    EXPECT_EQ(zero.GetValue(), u"0");
}

TEST(NativeNumericFormatTest, OutputParameterMatchesResultValueAndReusesStorage) {
    std::string narrow = "stale";
    EXPECT_EQ(UniConv::ToStringFromInt64(-9876543210LL, narrow), ErrorCode::Success);
    EXPECT_EQ(narrow, "-9876543210");

    std::u16string utf16 = u"stale";
    EXPECT_EQ(UniConv::ToUtf16StringFromInt64(-9876543210LL, utf16), ErrorCode::Success);
    EXPECT_EQ(utf16, u"-9876543210");

    std::u32string utf32 = U"stale";
    EXPECT_EQ(UniConv::ToUtf32StringFromInt32(456789, utf32), ErrorCode::Success);
    EXPECT_EQ(utf32, U"456789");

    std::wstring wide = L"stale";
    EXPECT_EQ(UniConv::ToWideStringFromInt32(456789, wide), ErrorCode::Success);
    EXPECT_EQ(wide, L"456789");

    const auto result = UniConv::ToStringFromInt64(-9876543210LL);
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(narrow, result.GetValue());
}

TEST(NativeFloatingParseTest, ParsesDecimalScientificAndNativeViews) {
    const auto decimal = UniConv::ParseDouble("-1.25");
    ASSERT_TRUE(decimal.IsSuccess());
    EXPECT_DOUBLE_EQ(decimal.GetValue(), -1.25);

    const auto scientific = UniConv::ParseDouble("6.02e23");
    ASSERT_TRUE(scientific.IsSuccess());
    EXPECT_DOUBLE_EQ(scientific.GetValue(), 6.02e23);

    const auto utf16 = UniConv::ParseFloat(std::u16string_view{u"0.5"});
    ASSERT_TRUE(utf16.IsSuccess());
    EXPECT_FLOAT_EQ(utf16.GetValue(), 0.5f);

    const auto wide = UniConv::ParseDouble(std::wstring_view{L"+3.125"});
    ASSERT_TRUE(wide.IsSuccess());
    EXPECT_DOUBLE_EQ(wide.GetValue(), 3.125);
}

TEST(NativeFloatingParseTest, RejectsMalformedWhitespaceAndOverflow) {
    ExpectNumericFailure(UniConv::ParseDouble(std::string_view{}),
                         ErrorCode::InvalidNumber);
    ExpectNumericFailure(UniConv::ParseDouble(" 1.0"), ErrorCode::InvalidNumber);
    ExpectNumericFailure(UniConv::ParseDouble("1.0 "), ErrorCode::InvalidNumber);
    ExpectNumericFailure(UniConv::ParseDouble("0x1.0"), ErrorCode::InvalidNumber);
    ExpectNumericFailure(UniConv::ParseDouble("1e"), ErrorCode::InvalidNumber);
    ExpectNumericFailure(UniConv::ParseDouble("1.0tail"), ErrorCode::InvalidNumber);
    ExpectNumericFailure(UniConv::ParseDouble("1e9999"), ErrorCode::NumericOverflow);
    ExpectNumericFailure(UniConv::ParseDouble("nan"), ErrorCode::InvalidNumber);

    FloatingParseOptions allow_non_finite{};
    allow_non_finite.allow_non_finite = true;
    const auto infinity = UniConv::ParseDouble("-INF", allow_non_finite);
    ASSERT_TRUE(infinity.IsSuccess());
    EXPECT_TRUE(std::isinf(infinity.GetValue()));
    EXPECT_LT(infinity.GetValue(), 0.0);
}

TEST(NativeFloatingFormatTest, FormatsAllNativeStringTypesAndPrecision) {
    const auto narrow = UniConv::ToStringFromDouble(1.25);
    ASSERT_TRUE(narrow.IsSuccess());
    EXPECT_EQ(narrow.GetValue(), "1.25");

    const auto utf16 = UniConv::ToUtf16StringFromFloat(0.5f);
    ASSERT_TRUE(utf16.IsSuccess());
    EXPECT_EQ(utf16.GetValue(), u"0.5");

    const auto utf32 = UniConv::ToUtf32StringFromDouble(12.5);
    ASSERT_TRUE(utf32.IsSuccess());
    EXPECT_EQ(utf32.GetValue(), U"12.5");

    const auto wide = UniConv::ToWideStringFromDouble(12.5);
    ASSERT_TRUE(wide.IsSuccess());
    EXPECT_EQ(wide.GetValue(), L"12.5");

    FloatingFormatOptions fixed{};
    fixed.format = std::chars_format::fixed;
    fixed.precision = 3;
    const auto precise = UniConv::ToStringFromDouble(1.25, fixed);
    ASSERT_TRUE(precise.IsSuccess());
    EXPECT_EQ(precise.GetValue(), "1.250");
}

TEST(NativeFloatingFormatTest, HandlesNonFinitePolicyAndOutputParameters) {
    FloatingFormatOptions reject{};
    reject.allow_non_finite = false;
    ExpectNumericFailure(UniConv::ToStringFromDouble(std::numeric_limits<double>::infinity(), reject),
                         ErrorCode::InvalidParameter);

    const auto nan = UniConv::ToStringFromDouble(std::numeric_limits<double>::quiet_NaN());
    ASSERT_TRUE(nan.IsSuccess());
    EXPECT_EQ(nan.GetValue(), "nan");

    std::u16string output = u"stale";
    EXPECT_EQ(UniConv::ToUtf16StringFromDouble(2.5, output), ErrorCode::Success);
    EXPECT_EQ(output, u"2.5");
}

TEST(NativeUtfFastTest, ConvertsAsciiUnicodeAndEmbeddedNul) {
    const std::string input("A\0\xF0\x9F\x98\x80", 6);
    const auto utf16 = UniConv::ToUtf16FromUtf8Fast(std::string_view{input});
    ASSERT_TRUE(utf16.IsSuccess());
    ASSERT_EQ(utf16.GetValue().size(), 4u);
    EXPECT_EQ(utf16.GetValue()[0], u'A');
    EXPECT_EQ(utf16.GetValue()[1], u'\0');
    EXPECT_EQ(utf16.GetValue()[2], static_cast<char16_t>(0xD83D));
    EXPECT_EQ(utf16.GetValue()[3], static_cast<char16_t>(0xDE00));

    const auto utf8 = UniConv::ToUtf8FromUtf16Fast(std::u16string_view{utf16.GetValue()});
    ASSERT_TRUE(utf8.IsSuccess());
    EXPECT_EQ(utf8.GetValue(), input);
}

TEST(NativeUtfFastTest, RejectsInvalidUtf8AndUnpairedUtf16) {
    const std::string invalid_utf8("\xF0\x28\x8C\xBC", 4);
    const auto utf16 = UniConv::ToUtf16FromUtf8Fast(std::string_view{invalid_utf8});
    ExpectNumericFailure(utf16, ErrorCode::InvalidSequence);

    const std::u16string high_only(1, static_cast<char16_t>(0xD800));
    const auto utf8 = UniConv::ToUtf8FromUtf16Fast(std::u16string_view{high_only});
    ExpectNumericFailure(utf8, ErrorCode::InvalidSequence);

    const char bounded[] = {'A', 'B', 'C', '\0', 'D'};
    const auto bounded_result = UniConv::ToUtf16FromUtf8Fast(bounded, 3);
    ASSERT_TRUE(bounded_result.IsSuccess());
    EXPECT_EQ(bounded_result.GetValue(), u"ABC");
}

TEST(NativeUtfFastTest, OutputParameterErrorsClearOutput) {
    std::u16string utf16 = u"stale";
    const std::string invalid("\xC0\x80", 2);
    EXPECT_EQ(UniConv::ToUtf16FromUtf8Fast(invalid, utf16), ErrorCode::InvalidSequence);
    EXPECT_TRUE(utf16.empty());

    std::string utf8 = "stale";
    const std::u16string invalid_utf16(1, static_cast<char16_t>(0xDC00));
    EXPECT_EQ(UniConv::ToUtf8FromUtf16Fast(invalid_utf16, utf8), ErrorCode::InvalidSequence);
    EXPECT_TRUE(utf8.empty());
}

TEST(NativeUtfFastTest, HandlesLargeInputForConfiguredBackend) {
    std::string input;
    input.reserve(1024);
    for (int i = 0; i < 128; ++i) {
        input += "ASCII-";
        input += "\xE4\xB8\xAD";
        input += "\xF0\x9F\x98\x80";
    }

    std::u16string utf16;
    ASSERT_EQ(UniConv::ToUtf16FromUtf8Fast(input, utf16), ErrorCode::Success);
    ASSERT_FALSE(utf16.empty());

    std::string round_trip;
    ASSERT_EQ(UniConv::ToUtf8FromUtf16Fast(utf16, round_trip), ErrorCode::Success);
    EXPECT_EQ(round_trip, input);
}

}  // namespace
