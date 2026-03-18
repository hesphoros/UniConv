/**
 * @file test_integer_conversion.cpp
 * @brief UniConv integer conversion tests - validates string <-> integer conversion functions
 */

#include <gtest/gtest.h>
#include <UniConv/UniConv.h>
#include <memory>
#include <string>
#include <limits>

// ============================================================================
// Test Fixture: Provides shared UniConv instance
// ============================================================================
class IntegerConversionTest : public ::testing::Test {
protected:
    void SetUp() override {
        converter = UniConv::Create();
        ASSERT_NE(converter, nullptr);
    }

    std::unique_ptr<UniConv> converter;
};

// ============================================================================
// Int32 to String Conversion Tests
// ============================================================================
TEST_F(IntegerConversionTest, Int32ToStringPositive) {
    EXPECT_EQ(converter->ToStringFromInt32(42), "42");
    EXPECT_EQ(converter->ToStringFromInt32(12345), "12345");
}

TEST_F(IntegerConversionTest, Int32ToStringNegative) {
    EXPECT_EQ(converter->ToStringFromInt32(-42), "-42");
    EXPECT_EQ(converter->ToStringFromInt32(-12345), "-12345");
}

TEST_F(IntegerConversionTest, Int32ToStringZero) {
    EXPECT_EQ(converter->ToStringFromInt32(0), "0");
}

TEST_F(IntegerConversionTest, Int32ToStringMinMax) {
    EXPECT_EQ(converter->ToStringFromInt32(std::numeric_limits<int32_t>::min()),
              std::to_string(std::numeric_limits<int32_t>::min()));
    EXPECT_EQ(converter->ToStringFromInt32(std::numeric_limits<int32_t>::max()),
              std::to_string(std::numeric_limits<int32_t>::max()));
}

// ============================================================================
// Int64 to String Conversion Tests
// ============================================================================
TEST_F(IntegerConversionTest, Int64ToStringPositive) {
    EXPECT_EQ(converter->ToStringFromInt64(42LL), "42");
    EXPECT_EQ(converter->ToStringFromInt64(1234567890123LL), "1234567890123");
}

TEST_F(IntegerConversionTest, Int64ToStringNegative) {
    EXPECT_EQ(converter->ToStringFromInt64(-42LL), "-42");
    EXPECT_EQ(converter->ToStringFromInt64(-1234567890123LL), "-1234567890123");
}

TEST_F(IntegerConversionTest, Int64ToStringZero) {
    EXPECT_EQ(converter->ToStringFromInt64(0LL), "0");
}

TEST_F(IntegerConversionTest, Int64ToStringMinMax) {
    EXPECT_EQ(converter->ToStringFromInt64(std::numeric_limits<int64_t>::min()),
              std::to_string(std::numeric_limits<int64_t>::min()));
    EXPECT_EQ(converter->ToStringFromInt64(std::numeric_limits<int64_t>::max()),
              std::to_string(std::numeric_limits<int64_t>::max()));
}

// ============================================================================
// Int32 to U16String Conversion Tests
// ============================================================================
TEST_F(IntegerConversionTest, Int32ToU16StringPositive) {
    auto result = converter->ToU16StringFromInt32(42);
    EXPECT_EQ(result.size(), 2u);
    EXPECT_EQ(result[0], u'4');
    EXPECT_EQ(result[1], u'2');
}

TEST_F(IntegerConversionTest, Int32ToU16StringNegative) {
    auto result = converter->ToU16StringFromInt32(-123);
    EXPECT_EQ(result.size(), 4u);
    EXPECT_EQ(result[0], u'-');
    EXPECT_EQ(result[1], u'1');
    EXPECT_EQ(result[2], u'2');
    EXPECT_EQ(result[3], u'3');
}

TEST_F(IntegerConversionTest, Int32ToU16StringZero) {
    auto result = converter->ToU16StringFromInt32(0);
    EXPECT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0], u'0');
}

// ============================================================================
// Int64 to U16String Conversion Tests
// ============================================================================
TEST_F(IntegerConversionTest, Int64ToU16StringPositive) {
    auto result = converter->ToU16StringFromInt64(9876543210LL);
    EXPECT_EQ(result.size(), 10u);
    EXPECT_EQ(result[0], u'9');
    EXPECT_EQ(result[9], u'0');
}

TEST_F(IntegerConversionTest, Int64ToU16StringNegative) {
    auto result = converter->ToU16StringFromInt64(-999LL);
    EXPECT_EQ(result.size(), 4u);
    EXPECT_EQ(result[0], u'-');
    EXPECT_EQ(result[1], u'9');
}

// ============================================================================
// Int32 to WString Conversion Tests
// ============================================================================
TEST_F(IntegerConversionTest, Int32ToWStringPositive) {
    auto result = converter->ToWStringFromInt32(789);
    EXPECT_EQ(result.size(), 3u);
    EXPECT_EQ(result[0], L'7');
    EXPECT_EQ(result[1], L'8');
    EXPECT_EQ(result[2], L'9');
}

TEST_F(IntegerConversionTest, Int32ToWStringNegative) {
    auto result = converter->ToWStringFromInt32(-456);
    EXPECT_EQ(result.size(), 4u);
    EXPECT_EQ(result[0], L'-');
    EXPECT_EQ(result[1], L'4');
}

TEST_F(IntegerConversionTest, Int32ToWStringZero) {
    auto result = converter->ToWStringFromInt32(0);
    EXPECT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0], L'0');
}

// ============================================================================
// Int64 to WString Conversion Tests
// ============================================================================
TEST_F(IntegerConversionTest, Int64ToWStringPositive) {
    auto result = converter->ToWStringFromInt64(1234567890LL);
    EXPECT_EQ(result.size(), 10u);
    EXPECT_EQ(result[0], L'1');
}

TEST_F(IntegerConversionTest, Int64ToWStringNegative) {
    auto result = converter->ToWStringFromInt64(-88888LL);
    EXPECT_EQ(result.size(), 6u);
    EXPECT_EQ(result[0], L'-');
}

// ============================================================================
// String to Int32 Conversion Tests
// ============================================================================
TEST_F(IntegerConversionTest, StringToInt32Positive) {
    auto result = converter->ToInt32FromString("42");
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(result.GetValue(), 42);
}

TEST_F(IntegerConversionTest, StringToInt32Negative) {
    auto result = converter->ToInt32FromString("-12345");
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(result.GetValue(), -12345);
}

TEST_F(IntegerConversionTest, StringToInt32Zero) {
    auto result = converter->ToInt32FromString("0");
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(result.GetValue(), 0);
}

TEST_F(IntegerConversionTest, StringToInt32MinMax) {
    auto minResult = converter->ToInt32FromString(std::to_string(std::numeric_limits<int32_t>::min()));
    ASSERT_TRUE(minResult.IsSuccess());
    EXPECT_EQ(minResult.GetValue(), std::numeric_limits<int32_t>::min());

    auto maxResult = converter->ToInt32FromString(std::to_string(std::numeric_limits<int32_t>::max()));
    ASSERT_TRUE(maxResult.IsSuccess());
    EXPECT_EQ(maxResult.GetValue(), std::numeric_limits<int32_t>::max());
}

TEST_F(IntegerConversionTest, StringToInt32EmptyString) {
    auto result = converter->ToInt32FromString("");
    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(result.GetErrorCode(), ErrorCode::InvalidParameter);
}

TEST_F(IntegerConversionTest, StringToInt32InvalidFormat) {
    auto result = converter->ToInt32FromString("abc");
    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(result.GetErrorCode(), ErrorCode::InvalidSequence);
}

TEST_F(IntegerConversionTest, StringToInt32PartialNumber) {
    auto result = converter->ToInt32FromString("123abc");
    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(result.GetErrorCode(), ErrorCode::InvalidSequence);
}

TEST_F(IntegerConversionTest, StringToInt32Overflow) {
    auto result = converter->ToInt32FromString("99999999999999999999");
    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(result.GetErrorCode(), ErrorCode::ConversionFailed);
}

TEST_F(IntegerConversionTest, StringToInt32WithWhitespace) {
    auto result = converter->ToInt32FromString(" 42 ");
    EXPECT_FALSE(result.IsSuccess());
}

// ============================================================================
// String to Int64 Conversion Tests
// ============================================================================
TEST_F(IntegerConversionTest, StringToInt64Positive) {
    auto result = converter->ToInt64FromString("1234567890123");
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(result.GetValue(), 1234567890123LL);
}

TEST_F(IntegerConversionTest, StringToInt64Negative) {
    auto result = converter->ToInt64FromString("-9876543210987");
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(result.GetValue(), -9876543210987LL);
}

TEST_F(IntegerConversionTest, StringToInt64Zero) {
    auto result = converter->ToInt64FromString("0");
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(result.GetValue(), 0LL);
}

TEST_F(IntegerConversionTest, StringToInt64MinMax) {
    auto minResult = converter->ToInt64FromString(std::to_string(std::numeric_limits<int64_t>::min()));
    ASSERT_TRUE(minResult.IsSuccess());
    EXPECT_EQ(minResult.GetValue(), std::numeric_limits<int64_t>::min());

    auto maxResult = converter->ToInt64FromString(std::to_string(std::numeric_limits<int64_t>::max()));
    ASSERT_TRUE(maxResult.IsSuccess());
    EXPECT_EQ(maxResult.GetValue(), std::numeric_limits<int64_t>::max());
}

TEST_F(IntegerConversionTest, StringToInt64EmptyString) {
    auto result = converter->ToInt64FromString("");
    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(result.GetErrorCode(), ErrorCode::InvalidParameter);
}

TEST_F(IntegerConversionTest, StringToInt64InvalidFormat) {
    auto result = converter->ToInt64FromString("xyz");
    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(result.GetErrorCode(), ErrorCode::InvalidSequence);
}

// ============================================================================
// U16String to Int32 Conversion Tests
// ============================================================================
TEST_F(IntegerConversionTest, U16StringToInt32Positive) {
    std::u16string input = u"42";
    auto result = converter->ToInt32FromU16String(input);
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(result.GetValue(), 42);
}

TEST_F(IntegerConversionTest, U16StringToInt32Negative) {
    std::u16string input = u"-999";
    auto result = converter->ToInt32FromU16String(input);
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(result.GetValue(), -999);
}

TEST_F(IntegerConversionTest, U16StringToInt32Zero) {
    std::u16string input = u"0";
    auto result = converter->ToInt32FromU16String(input);
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(result.GetValue(), 0);
}

TEST_F(IntegerConversionTest, U16StringToInt32EmptyString) {
    std::u16string input = u"";
    auto result = converter->ToInt32FromU16String(input);
    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(result.GetErrorCode(), ErrorCode::InvalidParameter);
}

TEST_F(IntegerConversionTest, U16StringToInt32NonAscii) {
    std::u16string input = u"１２３"; // Full-width digits
    auto result = converter->ToInt32FromU16String(input);
    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(result.GetErrorCode(), ErrorCode::InvalidSequence);
}

// ============================================================================
// U16String to Int64 Conversion Tests
// ============================================================================
TEST_F(IntegerConversionTest, U16StringToInt64Positive) {
    std::u16string input = u"9876543210";
    auto result = converter->ToInt64FromU16String(input);
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(result.GetValue(), 9876543210LL);
}

TEST_F(IntegerConversionTest, U16StringToInt64Negative) {
    std::u16string input = u"-123456789012";
    auto result = converter->ToInt64FromU16String(input);
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(result.GetValue(), -123456789012LL);
}

// ============================================================================
// WString to Int32 Conversion Tests
// ============================================================================
TEST_F(IntegerConversionTest, WStringToInt32Positive) {
    std::wstring input = L"789";
    auto result = converter->ToInt32FromWString(input);
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(result.GetValue(), 789);
}

TEST_F(IntegerConversionTest, WStringToInt32Negative) {
    std::wstring input = L"-456";
    auto result = converter->ToInt32FromWString(input);
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(result.GetValue(), -456);
}

TEST_F(IntegerConversionTest, WStringToInt32Zero) {
    std::wstring input = L"0";
    auto result = converter->ToInt32FromWString(input);
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(result.GetValue(), 0);
}

TEST_F(IntegerConversionTest, WStringToInt32EmptyString) {
    std::wstring input = L"";
    auto result = converter->ToInt32FromWString(input);
    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(result.GetErrorCode(), ErrorCode::InvalidParameter);
}

// ============================================================================
// WString to Int64 Conversion Tests
// ============================================================================
TEST_F(IntegerConversionTest, WStringToInt64Positive) {
    std::wstring input = L"1234567890";
    auto result = converter->ToInt64FromWString(input);
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(result.GetValue(), 1234567890LL);
}

TEST_F(IntegerConversionTest, WStringToInt64Negative) {
    std::wstring input = L"-987654321";
    auto result = converter->ToInt64FromWString(input);
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(result.GetValue(), -987654321LL);
}

// ============================================================================
// Round-trip Conversion Tests
// ============================================================================
TEST_F(IntegerConversionTest, RoundTripInt32String) {
    int32_t original = 12345;
    std::string str = converter->ToStringFromInt32(original);
    auto result = converter->ToInt32FromString(str);
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(result.GetValue(), original);
}

TEST_F(IntegerConversionTest, RoundTripInt64String) {
    int64_t original = 9876543210123LL;
    std::string str = converter->ToStringFromInt64(original);
    auto result = converter->ToInt64FromString(str);
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(result.GetValue(), original);
}

TEST_F(IntegerConversionTest, RoundTripInt32U16String) {
    int32_t original = -54321;
    std::u16string u16str = converter->ToU16StringFromInt32(original);
    auto result = converter->ToInt32FromU16String(u16str);
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(result.GetValue(), original);
}

TEST_F(IntegerConversionTest, RoundTripInt64U16String) {
    int64_t original = -1234567890123LL;
    std::u16string u16str = converter->ToU16StringFromInt64(original);
    auto result = converter->ToInt64FromU16String(u16str);
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(result.GetValue(), original);
}

TEST_F(IntegerConversionTest, RoundTripInt32WString) {
    int32_t original = 98765;
    std::wstring wstr = converter->ToWStringFromInt32(original);
    auto result = converter->ToInt32FromWString(wstr);
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(result.GetValue(), original);
}

TEST_F(IntegerConversionTest, RoundTripInt64WString) {
    int64_t original = 5555555555LL;
    std::wstring wstr = converter->ToWStringFromInt64(original);
    auto result = converter->ToInt64FromWString(wstr);
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(result.GetValue(), original);
}

// ============================================================================
// Edge Cases and Special Values
// ============================================================================
TEST_F(IntegerConversionTest, LeadingZeros) {
    auto result = converter->ToInt32FromString("00042");
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(result.GetValue(), 42);
}

TEST_F(IntegerConversionTest, PlusSign) {
    auto result = converter->ToInt32FromString("+123");
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(result.GetValue(), 123);
}

TEST_F(IntegerConversionTest, OnlyMinusSign) {
    auto result = converter->ToInt32FromString("-");
    EXPECT_FALSE(result.IsSuccess());
}

TEST_F(IntegerConversionTest, OnlyPlusSign) {
    auto result = converter->ToInt32FromString("+");
    EXPECT_FALSE(result.IsSuccess());
}

TEST_F(IntegerConversionTest, DoubleNegative) {
    auto result = converter->ToInt32FromString("--123");
    EXPECT_FALSE(result.IsSuccess());
}
