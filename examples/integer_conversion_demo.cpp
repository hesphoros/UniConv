/**
 * @file integer_conversion_demo.cpp
 * @brief Demonstrates integer to string and string to integer conversion functionality
 */

#include <UniConv/UniConv.h>
#include <iostream>
#include <limits>

int main() {
    auto& conv = UniConv::ThreadLocal();

    std::cout << "=== Integer to String Conversions ===" << std::endl;

    // Int32 to string conversions
    int32_t num32 = -12345;
    std::cout << "int32_t " << num32 << " to:" << std::endl;
    std::cout << "  string: " << conv.ToStringFromInt32(num32) << std::endl;

    auto u16str = conv.ToU16StringFromInt32(num32);
    std::cout << "  u16string length: " << u16str.size() << std::endl;

    auto wstr = conv.ToWStringFromInt32(num32);
    std::cout << "  wstring length: " << wstr.size() << std::endl;

    // Int64 to string conversions
    int64_t num64 = 9876543210123LL;
    std::cout << "\nint64_t " << num64 << " to:" << std::endl;
    std::cout << "  string: " << conv.ToStringFromInt64(num64) << std::endl;
    std::cout << "  u16string length: " << conv.ToU16StringFromInt64(num64).size() << std::endl;
    std::cout << "  wstring length: " << conv.ToWStringFromInt64(num64).size() << std::endl;

    std::cout << "\n=== String to Integer Conversions ===" << std::endl;

    // String to int32
    std::string str32 = "42";
    auto result32 = conv.ToInt32FromString(str32);
    if (result32.IsSuccess()) {
        std::cout << "String \"" << str32 << "\" to int32_t: " << result32.GetValue() << std::endl;
    }

    // String to int64
    std::string str64 = "-1234567890123";
    auto result64 = conv.ToInt64FromString(str64);
    if (result64.IsSuccess()) {
        std::cout << "String \"" << str64 << "\" to int64_t: " << result64.GetValue() << std::endl;
    }

    // U16String to int32
    std::u16string u16input = u"999";
    auto u16result = conv.ToInt32FromU16String(u16input);
    if (u16result.IsSuccess()) {
        std::cout << "u16string to int32_t: " << u16result.GetValue() << std::endl;
    }

    // WString to int64
    std::wstring winput = L"-777";
    auto wresult = conv.ToInt64FromWString(winput);
    if (wresult.IsSuccess()) {
        std::cout << "wstring to int64_t: " << wresult.GetValue() << std::endl;
    }

    std::cout << "\n=== Error Handling ===" << std::endl;

    // Invalid string
    auto invalid = conv.ToInt32FromString("abc");
    if (!invalid.IsSuccess()) {
        std::cout << "Invalid string \"abc\" correctly rejected: ErrorCode "
                  << static_cast<int>(invalid.GetErrorCode()) << std::endl;
    }

    // Overflow
    auto overflow = conv.ToInt32FromString("99999999999999999999");
    if (!overflow.IsSuccess()) {
        std::cout << "Overflow value correctly rejected: ErrorCode "
                  << static_cast<int>(overflow.GetErrorCode()) << std::endl;
    }

    // Empty string
    auto empty = conv.ToInt32FromString("");
    if (!empty.IsSuccess()) {
        std::cout << "Empty string correctly rejected: ErrorCode "
                  << static_cast<int>(empty.GetErrorCode()) << std::endl;
    }

    std::cout << "\n=== Round-trip Conversions ===" << std::endl;

    // Round-trip int32
    int32_t original32 = -54321;
    std::string str_rt = conv.ToStringFromInt32(original32);
    auto rt_result32 = conv.ToInt32FromString(str_rt);
    std::cout << "Round-trip int32: " << original32 << " -> \"" << str_rt
              << "\" -> " << rt_result32.GetValue()
              << " (match: " << (rt_result32.GetValue() == original32 ? "yes" : "no") << ")" << std::endl;

    // Round-trip int64 with u16string
    int64_t original64 = 1234567890123LL;
    std::u16string u16str_rt = conv.ToU16StringFromInt64(original64);
    auto rt_result64 = conv.ToInt64FromU16String(u16str_rt);
    std::cout << "Round-trip int64 (via u16string): " << original64
              << " -> " << rt_result64.GetValue()
              << " (match: " << (rt_result64.GetValue() == original64 ? "yes" : "no") << ")" << std::endl;

    std::cout << "\n=== Min/Max Values ===" << std::endl;

    // Test with extreme values
    int32_t min32 = std::numeric_limits<int32_t>::min();
    int32_t max32 = std::numeric_limits<int32_t>::max();

    std::cout << "int32_t min: " << conv.ToStringFromInt32(min32) << std::endl;
    std::cout << "int32_t max: " << conv.ToStringFromInt32(max32) << std::endl;

    int64_t min64 = std::numeric_limits<int64_t>::min();
    int64_t max64 = std::numeric_limits<int64_t>::max();

    std::cout << "int64_t min: " << conv.ToStringFromInt64(min64) << std::endl;
    std::cout << "int64_t max: " << conv.ToStringFromInt64(max64) << std::endl;

    return 0;
}
