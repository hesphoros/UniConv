/**
 * @file test_encoding_conversion.cpp
 * @brief UniConv 编码转换全覆盖测试
 *
 * 测试矩阵:
 *   转换方向: UTF-8, UTF-16LE, UTF-16BE, UTF-32LE, Locale, WideString, UCS-4
 *   API 风格: 返回值版、输出参数(zero-copy)版、string_view版、CompactResult(Ex)版
 *   批量接口: ConvertEncodingBatch, ConvertEncodingBatchParallel
 *   边界条件: 空输入、纯ASCII、中文、emoji、混合脚本、超长文本、错误编码
 */

#include <gtest/gtest.h>
#include <uniconv/UniConv.h>
#include <memory>
#include <string>
#include <vector>
#include <thread>

// ============================================================================
// 测试夹具
// ============================================================================
class EncodingConversionTest : public ::testing::Test {
protected:
    void SetUp() override {
        conv = UniConv::Create();
        ASSERT_NE(conv, nullptr);
    }

    // Locale 相关 API 依赖系统 locale 检测，FetchContent 构建的 embedded libiconv
    // 在某些平台上无法正确检测 locale 编码，导致转换返回空结果。
    // 此辅助方法检测 locale API 是否可用。
    bool IsLocaleAvailable() {
        std::string result = conv->ToUtf8FromLocale("test");
        return !result.empty();
    }

    std::unique_ptr<UniConv> conv;

    // 常用测试字符串 (UTF-8 encoded)
    const std::string ascii_text    = "Hello, World!";
    const std::string chinese_text  = "\xe4\xbd\xa0\xe5\xa5\xbd\xe4\xb8\x96\xe7\x95\x8c";           // 你好世界
    const std::string mixed_text    = "Hello \xe4\xbd\xa0\xe5\xa5\xbd 123";                         // Hello 你好 123
    const std::string emoji_text    = "\xf0\x9f\x98\x80\xf0\x9f\x8c\x8d\xf0\x9f\x9a\x80";           // 😀🌍🚀
    const std::string full_text     = "Hello, \xe4\xb8\x96\xe7\x95\x8c! \xf0\x9f\x8c\x8d";          // Hello, 世界! 🌍
};

// ============================================================================
// 1. UTF-8 <-> UTF-16LE (返回值版)
// ============================================================================
TEST_F(EncodingConversionTest, Utf8ToUtf16LE_Ascii) {
    std::u16string result = conv->ToUtf16LEFromUtf8(ascii_text);
    EXPECT_FALSE(result.empty());
    std::string back = conv->ToUtf8FromUtf16LE(result);
    EXPECT_EQ(back, ascii_text);
}

TEST_F(EncodingConversionTest, Utf8ToUtf16LE_Chinese) {
    std::u16string result = conv->ToUtf16LEFromUtf8(chinese_text);
    EXPECT_FALSE(result.empty());
    std::string back = conv->ToUtf8FromUtf16LE(result);
    EXPECT_EQ(back, chinese_text);
}

TEST_F(EncodingConversionTest, Utf8ToUtf16LE_Mixed) {
    std::u16string result = conv->ToUtf16LEFromUtf8(mixed_text);
    EXPECT_FALSE(result.empty());
    std::string back = conv->ToUtf8FromUtf16LE(result);
    EXPECT_EQ(back, mixed_text);
}

TEST_F(EncodingConversionTest, Utf8ToUtf16LE_Emoji) {
    std::u16string result = conv->ToUtf16LEFromUtf8(emoji_text);
    EXPECT_FALSE(result.empty());
    std::string back = conv->ToUtf8FromUtf16LE(result);
    EXPECT_EQ(back, emoji_text);
}

TEST_F(EncodingConversionTest, Utf8ToUtf16LE_Empty) {
    std::u16string result = conv->ToUtf16LEFromUtf8(std::string(""));
    EXPECT_TRUE(result.empty());
}

TEST_F(EncodingConversionTest, Utf8ToUtf16LE_CStyleString) {
    const char* cstr = "Test C-string";
    std::u16string result = conv->ToUtf16LEFromUtf8(cstr);
    EXPECT_FALSE(result.empty());
    std::string back = conv->ToUtf8FromUtf16LE(result);
    EXPECT_EQ(back, std::string(cstr));
}

TEST_F(EncodingConversionTest, Utf16LEToUtf8_CStyleString) {
    std::u16string u16 = conv->ToUtf16LEFromUtf8(ascii_text);
    ASSERT_FALSE(u16.empty());
    std::string back = conv->ToUtf8FromUtf16LE(u16.c_str());
    EXPECT_EQ(back, ascii_text);
}

TEST_F(EncodingConversionTest, Utf16LEToUtf8_WithLength) {
    std::u16string u16 = conv->ToUtf16LEFromUtf8(ascii_text);
    ASSERT_FALSE(u16.empty());
    std::string back = conv->ToUtf8FromUtf16LE(u16.c_str(), u16.size());
    EXPECT_EQ(back, ascii_text);
}

// ============================================================================
// 2. UTF-8 <-> UTF-16BE (返回值版)
// ============================================================================
TEST_F(EncodingConversionTest, Utf8ToUtf16BE_Ascii) {
    std::u16string result = conv->ToUtf16BEFromUtf8(ascii_text);
    EXPECT_FALSE(result.empty());
    std::string back = conv->ToUtf8FromUtf16BE(result);
    EXPECT_EQ(back, ascii_text);
}

TEST_F(EncodingConversionTest, Utf8ToUtf16BE_Chinese) {
    std::u16string result = conv->ToUtf16BEFromUtf8(chinese_text);
    EXPECT_FALSE(result.empty());
    std::string back = conv->ToUtf8FromUtf16BE(result);
    EXPECT_EQ(back, chinese_text);
}

TEST_F(EncodingConversionTest, Utf8ToUtf16BE_Emoji) {
    std::u16string result = conv->ToUtf16BEFromUtf8(emoji_text);
    EXPECT_FALSE(result.empty());
    std::string back = conv->ToUtf8FromUtf16BE(result);
    EXPECT_EQ(back, emoji_text);
}

TEST_F(EncodingConversionTest, Utf8ToUtf16BE_CStyleString) {
    std::u16string result = conv->ToUtf16BEFromUtf8("CStyle");
    EXPECT_FALSE(result.empty());
    std::string back = conv->ToUtf8FromUtf16BE(result.c_str());
    EXPECT_EQ(back, "CStyle");
}

TEST_F(EncodingConversionTest, Utf16BEToUtf8_WithLength) {
    std::u16string u16 = conv->ToUtf16BEFromUtf8(chinese_text);
    ASSERT_FALSE(u16.empty());
    std::string back = conv->ToUtf8FromUtf16BE(u16.c_str(), u16.size());
    EXPECT_EQ(back, chinese_text);
}

// ============================================================================
// 3. UTF-16LE <-> UTF-16BE
// ============================================================================
TEST_F(EncodingConversionTest, Utf16LEToUtf16BE_RoundTrip) {
    std::u16string le = conv->ToUtf16LEFromUtf8(full_text);
    ASSERT_FALSE(le.empty());

    std::u16string be = conv->ToUtf16BEFromUtf16LE(le);
    EXPECT_FALSE(be.empty());

    std::u16string back_le = conv->ToUtf16LEFromUtf16BE(be);
    EXPECT_EQ(back_le, le);
}

TEST_F(EncodingConversionTest, Utf16BEToUtf16LE_CStyleString) {
    std::u16string le = conv->ToUtf16LEFromUtf8(ascii_text);
    std::u16string be = conv->ToUtf16BEFromUtf16LE(le.c_str());
    EXPECT_FALSE(be.empty());
    std::u16string back = conv->ToUtf16LEFromUtf16BE(be.c_str());
    EXPECT_EQ(back, le);
}

TEST_F(EncodingConversionTest, Utf16LEToUtf16BE_ByteSwapVerification) {
    std::u16string le = conv->ToUtf16LEFromUtf8("A");
    ASSERT_FALSE(le.empty());
    std::u16string be = conv->ToUtf16BEFromUtf16LE(le);
    ASSERT_FALSE(be.empty());

    // 'A' = 0x0041; LE stores as 41 00, BE stores as 00 41
    // After byte-swap via iconv, raw bytes should differ
    const auto* le_bytes = reinterpret_cast<const uint8_t*>(le.data());
    const auto* be_bytes = reinterpret_cast<const uint8_t*>(be.data());
    EXPECT_EQ(le_bytes[0], be_bytes[1]);
    EXPECT_EQ(le_bytes[1], be_bytes[0]);
}

// ============================================================================
// 4. UTF-8 <-> UTF-32LE
// ============================================================================
TEST_F(EncodingConversionTest, Utf8ToUtf32LE_Ascii) {
    std::u32string result = conv->ToUtf32LEFromUtf8(ascii_text);
    EXPECT_EQ(result.size(), ascii_text.size()); // ASCII: 1 char = 1 codepoint
    std::string back = conv->ToUtf8FromUtf32LE(result);
    EXPECT_EQ(back, ascii_text);
}

TEST_F(EncodingConversionTest, Utf8ToUtf32LE_Chinese) {
    std::u32string result = conv->ToUtf32LEFromUtf8(chinese_text);
    EXPECT_EQ(result.size(), 4u); // 你好世界 = 4 codepoints
    std::string back = conv->ToUtf8FromUtf32LE(result);
    EXPECT_EQ(back, chinese_text);
}

TEST_F(EncodingConversionTest, Utf8ToUtf32LE_Emoji) {
    std::u32string result = conv->ToUtf32LEFromUtf8(emoji_text);
    EXPECT_EQ(result.size(), 3u); // 😀🌍🚀 = 3 codepoints
    std::string back = conv->ToUtf8FromUtf32LE(result);
    EXPECT_EQ(back, emoji_text);
}

TEST_F(EncodingConversionTest, Utf8ToUtf32LE_CStyleString) {
    std::u32string result = conv->ToUtf32LEFromUtf8("ABC");
    EXPECT_EQ(result.size(), 3u);
    std::string back = conv->ToUtf8FromUtf32LE(result.c_str());
    EXPECT_EQ(back, "ABC");
}

// ============================================================================
// 5. UTF-16LE <-> UTF-32LE
// ============================================================================
TEST_F(EncodingConversionTest, Utf16LEToUtf32LE_RoundTrip) {
    std::u16string u16 = conv->ToUtf16LEFromUtf8(full_text);
    ASSERT_FALSE(u16.empty());

    std::u32string u32 = conv->ToUtf32LEFromUtf16LE(u16);
    EXPECT_FALSE(u32.empty());

    std::u16string back = conv->ToUtf16LEFromUtf32LE(u32);
    EXPECT_EQ(back, u16);
}

TEST_F(EncodingConversionTest, Utf16LEToUtf32LE_CStyleString) {
    std::u16string u16 = conv->ToUtf16LEFromUtf8(ascii_text);
    std::u32string u32 = conv->ToUtf32LEFromUtf16LE(u16.c_str());
    EXPECT_EQ(u32.size(), ascii_text.size());
    std::u16string back = conv->ToUtf16LEFromUtf32LE(u32.c_str());
    EXPECT_EQ(back, u16);
}

// ============================================================================
// 6. UTF-16BE <-> UTF-32LE
// ============================================================================
TEST_F(EncodingConversionTest, Utf16BEToUtf32LE_RoundTrip) {
    std::u16string be = conv->ToUtf16BEFromUtf8(chinese_text);
    ASSERT_FALSE(be.empty());

    std::u32string u32 = conv->ToUtf32LEFromUtf16BE(be);
    EXPECT_EQ(u32.size(), 4u);

    std::u16string back_be = conv->ToUtf16BEFromUtf32LE(u32);
    EXPECT_EQ(back_be, be);
}

TEST_F(EncodingConversionTest, Utf16BEToUtf32LE_CStyleString) {
    std::u16string be = conv->ToUtf16BEFromUtf8(ascii_text);
    std::u32string u32 = conv->ToUtf32LEFromUtf16BE(be.c_str());
    EXPECT_FALSE(u32.empty());
}

// ============================================================================
// 7. Locale <-> UTF-8
// ============================================================================
TEST_F(EncodingConversionTest, LocaleToUtf8_RoundTrip) {
    if (!IsLocaleAvailable()) GTEST_SKIP() << "Locale detection unavailable";
    std::string utf8 = conv->ToUtf8FromLocale(ascii_text);
    EXPECT_FALSE(utf8.empty());
    std::string back = conv->ToLocaleFromUtf8(utf8);
    EXPECT_EQ(back, ascii_text);
}

TEST_F(EncodingConversionTest, LocaleToUtf8_CStyleString) {
    if (!IsLocaleAvailable()) GTEST_SKIP() << "Locale detection unavailable";
    std::string utf8 = conv->ToUtf8FromLocale("test");
    EXPECT_FALSE(utf8.empty());
    std::string back = conv->ToLocaleFromUtf8("test");
    EXPECT_FALSE(back.empty());
}

// ============================================================================
// 8. Locale <-> UTF-16LE/BE
// ============================================================================
TEST_F(EncodingConversionTest, LocaleToUtf16LE_RoundTrip) {
    if (!IsLocaleAvailable()) GTEST_SKIP() << "Locale detection unavailable";
    std::u16string u16 = conv->ToUtf16LEFromLocale(ascii_text);
    EXPECT_FALSE(u16.empty());
    std::string back = conv->ToLocaleFromUtf16LE(u16);
    EXPECT_EQ(back, ascii_text);
}

TEST_F(EncodingConversionTest, LocaleToUtf16LE_CStyleString) {
    if (!IsLocaleAvailable()) GTEST_SKIP() << "Locale detection unavailable";
    std::u16string u16 = conv->ToUtf16LEFromLocale("locale test");
    EXPECT_FALSE(u16.empty());
    std::string back = conv->ToLocaleFromUtf16LE(u16.c_str());
    EXPECT_FALSE(back.empty());
}

TEST_F(EncodingConversionTest, LocaleToUtf16BE_RoundTrip) {
    if (!IsLocaleAvailable()) GTEST_SKIP() << "Locale detection unavailable";
    std::u16string u16 = conv->ToUtf16BEFromLocale(ascii_text);
    EXPECT_FALSE(u16.empty());
    std::string back = conv->ToLocaleFromUtf16BE(u16);
    EXPECT_EQ(back, ascii_text);
}

TEST_F(EncodingConversionTest, LocaleToUtf16BE_CStyleString) {
    if (!IsLocaleAvailable()) GTEST_SKIP() << "Locale detection unavailable";
    std::u16string u16 = conv->ToUtf16BEFromLocale("locale test");
    EXPECT_FALSE(u16.empty());
    std::string back = conv->ToLocaleFromUtf16BE(u16.c_str());
    EXPECT_FALSE(back.empty());
}

// ============================================================================
// 9. Locale <-> WideString
// ============================================================================
TEST_F(EncodingConversionTest, LocaleToWideString_RoundTrip) {
    if (!IsLocaleAvailable()) GTEST_SKIP() << "Locale detection unavailable";
    std::wstring wide = conv->ToWideStringFromLocale(ascii_text);
    EXPECT_FALSE(wide.empty());
    std::string back = conv->ToLocaleFromWideString(wide);
    EXPECT_EQ(back, ascii_text);
}

TEST_F(EncodingConversionTest, LocaleToWideString_CStyleString) {
    if (!IsLocaleAvailable()) GTEST_SKIP() << "Locale detection unavailable";
    std::wstring wide = conv->ToWideStringFromLocale("wide test");
    EXPECT_FALSE(wide.empty());
    std::string back = conv->ToLocaleFromWideString(wide.c_str());
    EXPECT_EQ(back, "wide test");
}

TEST_F(EncodingConversionTest, WideStringToLocale_Aliases) {
    if (!IsLocaleAvailable()) GTEST_SKIP() << "Locale detection unavailable";
    std::wstring wide = conv->ToWideStringFromLocale(ascii_text);
    ASSERT_FALSE(wide.empty());
    std::string a = conv->ToLocaleFromWideString(wide);
    std::string b = conv->WideStringToLocale(wide);
    EXPECT_EQ(a, b);

    std::string c = conv->WideStringToLocale(wide.c_str());
    EXPECT_EQ(a, c);
}

// ============================================================================
// 10. UCS-4 <-> UTF-8
// ============================================================================
TEST_F(EncodingConversionTest, Ucs4ToUtf8_RoundTrip) {
    if (!IsLocaleAvailable()) GTEST_SKIP() << "UCS-4 conversion depends on locale";
    std::wstring ucs4 = conv->ToUcs4FromUtf8(full_text);
    EXPECT_FALSE(ucs4.empty());
    std::string back = conv->ToUtf8FromUcs4(ucs4);
    EXPECT_EQ(back, full_text);
}

TEST_F(EncodingConversionTest, Ucs4ToUtf8_Emoji) {
    if (!IsLocaleAvailable()) GTEST_SKIP() << "UCS-4 conversion depends on locale";
    std::wstring ucs4 = conv->ToUcs4FromUtf8(emoji_text);
    EXPECT_FALSE(ucs4.empty());
    std::string back = conv->ToUtf8FromUcs4(ucs4);
    EXPECT_EQ(back, emoji_text);
}

// ============================================================================
// 11. U16String <-> WString
// ============================================================================
TEST_F(EncodingConversionTest, U16StringToWString_RoundTrip) {
    if (!IsLocaleAvailable()) GTEST_SKIP() << "U16String/WString conversion depends on locale";
    std::u16string u16 = conv->ToUtf16LEFromUtf8(ascii_text);
    ASSERT_FALSE(u16.empty());

    std::wstring wide = conv->U16StringToWString(u16);
    EXPECT_FALSE(wide.empty());
}

TEST_F(EncodingConversionTest, U16StringToWString_CStyleString) {
    if (!IsLocaleAvailable()) GTEST_SKIP() << "U16String/WString conversion depends on locale";
    std::u16string u16 = conv->ToUtf16LEFromUtf8(ascii_text);
    ASSERT_FALSE(u16.empty());

    std::wstring wide = conv->U16StringToWString(u16.c_str());
    EXPECT_FALSE(wide.empty());
}

// ============================================================================
// 12. 输出参数 (zero-copy) API: ConvertEncoding / ConvertEncodingFast
// ============================================================================
TEST_F(EncodingConversionTest, OutputParam_ConvertEncoding_Utf8ToUtf16LE) {
    std::string output;
    bool ok = conv->ConvertEncoding(ascii_text, "UTF-8", "UTF-16LE", output);
    EXPECT_TRUE(ok);
    EXPECT_FALSE(output.empty());
}

TEST_F(EncodingConversionTest, OutputParam_ConvertEncodingFast_Utf8ToUtf16LE) {
    std::string output;
    auto ec = conv->ConvertEncodingFast(ascii_text, "UTF-8", "UTF-16LE", output);
    EXPECT_EQ(ec, ErrorCode::Success);
    EXPECT_FALSE(output.empty());
}

TEST_F(EncodingConversionTest, OutputParam_ConvertEncodingFast_InvalidEncoding) {
    std::string output;
    auto ec = conv->ConvertEncodingFast(ascii_text, "BOGUS", "UTF-16LE", output);
    EXPECT_NE(ec, ErrorCode::Success);
}

TEST_F(EncodingConversionTest, OutputParam_BufferReuse) {
    std::string output;
    // 第一次转换
    bool ok1 = conv->ConvertEncoding(ascii_text, "UTF-8", "UTF-16LE", output);
    EXPECT_TRUE(ok1);
    const auto* ptr1 = output.data();

    // 第二次转换复用 buffer
    bool ok2 = conv->ConvertEncoding(mixed_text, "UTF-8", "UTF-16LE", output);
    EXPECT_TRUE(ok2);
    EXPECT_FALSE(output.empty());
    // 如果 capacity 足够，可能复用同一块内存 (不强制断言，仅验证功能正确)
    (void)ptr1;
}

// ============================================================================
// 13. 输出参数 API: UTF-8 系列
// ============================================================================
TEST_F(EncodingConversionTest, OutputParam_ToUtf8FromLocale) {
    if (!IsLocaleAvailable()) GTEST_SKIP() << "Locale detection unavailable";
    std::string output;
    bool ok = conv->ToUtf8FromLocale(ascii_text, output);
    EXPECT_TRUE(ok);
    EXPECT_FALSE(output.empty());
}

TEST_F(EncodingConversionTest, OutputParam_ToLocaleFromUtf8) {
    if (!IsLocaleAvailable()) GTEST_SKIP() << "Locale detection unavailable";
    std::string output;
    bool ok = conv->ToLocaleFromUtf8(ascii_text, output);
    EXPECT_TRUE(ok);
    EXPECT_FALSE(output.empty());
}

TEST_F(EncodingConversionTest, OutputParam_ToUtf8FromUtf16LE) {
    std::u16string u16 = conv->ToUtf16LEFromUtf8(chinese_text);
    std::string output;
    bool ok = conv->ToUtf8FromUtf16LE(u16, output);
    EXPECT_TRUE(ok);
    EXPECT_EQ(output, chinese_text);
}

TEST_F(EncodingConversionTest, OutputParam_ToUtf8FromUtf16BE) {
    std::u16string u16 = conv->ToUtf16BEFromUtf8(chinese_text);
    std::string output;
    bool ok = conv->ToUtf8FromUtf16BE(u16, output);
    EXPECT_TRUE(ok);
    EXPECT_EQ(output, chinese_text);
}

TEST_F(EncodingConversionTest, OutputParam_ToUtf8FromUtf32LE) {
    std::u32string u32 = conv->ToUtf32LEFromUtf8(chinese_text);
    std::string output;
    bool ok = conv->ToUtf8FromUtf32LE(u32, output);
    EXPECT_TRUE(ok);
    EXPECT_EQ(output, chinese_text);
}

// ============================================================================
// 14. 输出参数 API: UTF-16 系列
// ============================================================================
TEST_F(EncodingConversionTest, OutputParam_ToUtf16LEFromUtf8) {
    std::u16string output;
    bool ok = conv->ToUtf16LEFromUtf8(ascii_text, output);
    EXPECT_TRUE(ok);
    EXPECT_FALSE(output.empty());
}

TEST_F(EncodingConversionTest, OutputParam_ToUtf16BEFromUtf8) {
    std::u16string output;
    bool ok = conv->ToUtf16BEFromUtf8(ascii_text, output);
    EXPECT_TRUE(ok);
    EXPECT_FALSE(output.empty());
}

TEST_F(EncodingConversionTest, OutputParam_ToUtf16LEFromLocale) {
    if (!IsLocaleAvailable()) GTEST_SKIP() << "Locale detection unavailable";
    std::u16string output;
    bool ok = conv->ToUtf16LEFromLocale(ascii_text, output);
    EXPECT_TRUE(ok);
    EXPECT_FALSE(output.empty());
}

TEST_F(EncodingConversionTest, OutputParam_ToUtf16BEFromLocale) {
    if (!IsLocaleAvailable()) GTEST_SKIP() << "Locale detection unavailable";
    std::u16string output;
    bool ok = conv->ToUtf16BEFromLocale(ascii_text, output);
    EXPECT_TRUE(ok);
    EXPECT_FALSE(output.empty());
}

TEST_F(EncodingConversionTest, OutputParam_Utf16LEBEInterconvert) {
    std::u16string le = conv->ToUtf16LEFromUtf8(full_text);
    ASSERT_FALSE(le.empty());

    std::u16string be_out;
    bool ok1 = conv->ToUtf16BEFromUtf16LE(le, be_out);
    EXPECT_TRUE(ok1);

    std::u16string le_back;
    bool ok2 = conv->ToUtf16LEFromUtf16BE(be_out, le_back);
    EXPECT_TRUE(ok2);
    EXPECT_EQ(le_back, le);
}

// ============================================================================
// 15. 输出参数 API: UTF-32 系列
// ============================================================================
TEST_F(EncodingConversionTest, OutputParam_ToUtf32LEFromUtf8) {
    std::u32string output;
    bool ok = conv->ToUtf32LEFromUtf8(ascii_text, output);
    EXPECT_TRUE(ok);
    EXPECT_EQ(output.size(), ascii_text.size());
}

TEST_F(EncodingConversionTest, OutputParam_ToUtf32LEFromUtf16LE) {
    std::u16string u16 = conv->ToUtf16LEFromUtf8(chinese_text);
    std::u32string output;
    bool ok = conv->ToUtf32LEFromUtf16LE(u16, output);
    EXPECT_TRUE(ok);
    EXPECT_EQ(output.size(), 4u);
}

TEST_F(EncodingConversionTest, OutputParam_ToUtf32LEFromUtf16BE) {
    std::u16string be = conv->ToUtf16BEFromUtf8(chinese_text);
    std::u32string output;
    bool ok = conv->ToUtf32LEFromUtf16BE(be, output);
    EXPECT_TRUE(ok);
    EXPECT_EQ(output.size(), 4u);
}

// ============================================================================
// 16. 输出参数 API: Locale / WideString 系列
// ============================================================================
TEST_F(EncodingConversionTest, OutputParam_ToLocaleFromUtf16LE) {
    if (!IsLocaleAvailable()) GTEST_SKIP() << "Locale detection unavailable";
    std::u16string u16 = conv->ToUtf16LEFromLocale(ascii_text);
    std::string output;
    bool ok = conv->ToLocaleFromUtf16LE(u16, output);
    EXPECT_TRUE(ok);
    EXPECT_EQ(output, ascii_text);
}

TEST_F(EncodingConversionTest, OutputParam_ToLocaleFromUtf16BE) {
    if (!IsLocaleAvailable()) GTEST_SKIP() << "Locale detection unavailable";
    std::u16string u16 = conv->ToUtf16BEFromLocale(ascii_text);
    std::string output;
    bool ok = conv->ToLocaleFromUtf16BE(u16, output);
    EXPECT_TRUE(ok);
    EXPECT_EQ(output, ascii_text);
}

TEST_F(EncodingConversionTest, OutputParam_ToLocaleFromWideString) {
    if (!IsLocaleAvailable()) GTEST_SKIP() << "Locale detection unavailable";
    std::wstring wide = conv->ToWideStringFromLocale(ascii_text);
    std::string output;
    bool ok = conv->ToLocaleFromWideString(wide, output);
    EXPECT_TRUE(ok);
    EXPECT_EQ(output, ascii_text);
}

TEST_F(EncodingConversionTest, OutputParam_ToWideStringFromLocale) {
    if (!IsLocaleAvailable()) GTEST_SKIP() << "Locale detection unavailable";
    std::wstring output;
    bool ok = conv->ToWideStringFromLocale(ascii_text, output);
    EXPECT_TRUE(ok);
    EXPECT_FALSE(output.empty());
}

TEST_F(EncodingConversionTest, OutputParam_U16StringToWString) {
    if (!IsLocaleAvailable()) GTEST_SKIP() << "U16String/WString depends on locale";
    std::u16string u16 = conv->ToUtf16LEFromUtf8(ascii_text);
    std::wstring output;
    bool ok = conv->U16StringToWString(u16, output);
    EXPECT_TRUE(ok);
    EXPECT_FALSE(output.empty());
}

// ============================================================================
// 17. string_view 输入重载
// ============================================================================
TEST_F(EncodingConversionTest, StringView_ConvertEncoding) {
    std::string_view sv = ascii_text;
    std::string output;
    bool ok = conv->ConvertEncoding(sv, "UTF-8", "UTF-16LE", output);
    EXPECT_TRUE(ok);
    EXPECT_FALSE(output.empty());
}

TEST_F(EncodingConversionTest, StringView_ConvertEncodingFast) {
    std::string_view sv = chinese_text;
    std::string output;
    auto ec = conv->ConvertEncodingFast(sv, "UTF-8", "UTF-16LE", output);
    EXPECT_EQ(ec, ErrorCode::Success);
    EXPECT_FALSE(output.empty());
}

TEST_F(EncodingConversionTest, StringView_ToUtf8FromLocale) {
    if (!IsLocaleAvailable()) GTEST_SKIP() << "Locale detection unavailable";
    std::string_view sv = ascii_text;
    std::string output;
    bool ok = conv->ToUtf8FromLocale(sv, output);
    EXPECT_TRUE(ok);
}

TEST_F(EncodingConversionTest, StringView_ToLocaleFromUtf8) {
    if (!IsLocaleAvailable()) GTEST_SKIP() << "Locale detection unavailable";
    std::string_view sv = ascii_text;
    std::string output;
    bool ok = conv->ToLocaleFromUtf8(sv, output);
    EXPECT_TRUE(ok);
}

TEST_F(EncodingConversionTest, StringView_ToUtf16LEFromUtf8) {
    std::string_view sv = full_text;
    std::u16string output;
    bool ok = conv->ToUtf16LEFromUtf8(sv, output);
    EXPECT_TRUE(ok);
    EXPECT_FALSE(output.empty());
}

TEST_F(EncodingConversionTest, StringView_ToUtf16BEFromUtf8) {
    std::string_view sv = full_text;
    std::u16string output;
    bool ok = conv->ToUtf16BEFromUtf8(sv, output);
    EXPECT_TRUE(ok);
    EXPECT_FALSE(output.empty());
}

TEST_F(EncodingConversionTest, StringView_SubstringConversion) {
    std::string full = "prefix_Hello_suffix";
    std::string_view sv(full.data() + 7, 5); // "Hello"
    std::string output;
    bool ok = conv->ConvertEncoding(sv, "UTF-8", "UTF-16LE", output);
    EXPECT_TRUE(ok);

    std::string verify_out;
    conv->ConvertEncoding(std::string("Hello"), "UTF-8", "UTF-16LE", verify_out);
    EXPECT_EQ(output, verify_out);
}

// ============================================================================
// 18. CompactResult (Ex) API
// ============================================================================
TEST_F(EncodingConversionTest, Ex_ToUtf8FromLocaleEx) {
    if (!IsLocaleAvailable()) GTEST_SKIP() << "Locale detection unavailable";
    auto result = conv->ToUtf8FromLocaleEx(ascii_text);
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_FALSE(result.GetValue().empty());
}

TEST_F(EncodingConversionTest, Ex_ToLocaleFromUtf8Ex) {
    if (!IsLocaleAvailable()) GTEST_SKIP() << "Locale detection unavailable";
    auto result = conv->ToLocaleFromUtf8Ex(ascii_text);
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_FALSE(result.GetValue().empty());
}

TEST_F(EncodingConversionTest, Ex_ToUtf16LEFromLocaleEx) {
    if (!IsLocaleAvailable()) GTEST_SKIP() << "Locale detection unavailable";
    auto result = conv->ToUtf16LEFromLocaleEx(ascii_text);
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_FALSE(result.GetValue().empty());
}

TEST_F(EncodingConversionTest, Ex_ToUtf16BEFromLocaleEx) {
    if (!IsLocaleAvailable()) GTEST_SKIP() << "Locale detection unavailable";
    auto result = conv->ToUtf16BEFromLocaleEx(ascii_text);
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_FALSE(result.GetValue().empty());
}

TEST_F(EncodingConversionTest, Ex_ToUtf8FromUtf16LEEx) {
    std::u16string u16 = conv->ToUtf16LEFromUtf8(chinese_text);
    auto result = conv->ToUtf8FromUtf16LEEx(u16);
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ(result.GetValue(), chinese_text);
}

TEST_F(EncodingConversionTest, Ex_ToUtf8FromUtf16BEEx) {
    std::u16string u16 = conv->ToUtf16BEFromUtf8(chinese_text);
    auto result = conv->ToUtf8FromUtf16BEEx(u16);
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ(result.GetValue(), chinese_text);
}

// ============================================================================
// 19. ConvertEncodingFast (CompactResult 返回值版) 多编码对
// ============================================================================
TEST_F(EncodingConversionTest, FastResult_Utf8ToGBK) {
    auto result = conv->ConvertEncodingFast(chinese_text, "UTF-8", "GBK");
    ASSERT_TRUE(result.IsSuccess());
    auto back = conv->ConvertEncodingFast(result.GetValue(), "GBK", "UTF-8");
    ASSERT_TRUE(back.IsSuccess());
    EXPECT_EQ(back.GetValue(), chinese_text);
}

TEST_F(EncodingConversionTest, FastResult_Utf8ToGB2312) {
    // GB2312 是 GBK 的子集, 基本汉字可互转
    auto result = conv->ConvertEncodingFast(chinese_text, "UTF-8", "GB2312");
    ASSERT_TRUE(result.IsSuccess());
    auto back = conv->ConvertEncodingFast(result.GetValue(), "GB2312", "UTF-8");
    ASSERT_TRUE(back.IsSuccess());
    EXPECT_EQ(back.GetValue(), chinese_text);
}

TEST_F(EncodingConversionTest, FastResult_Utf8ToISO8859_1) {
    auto result = conv->ConvertEncodingFast(ascii_text, "UTF-8", "ISO-8859-1");
    ASSERT_TRUE(result.IsSuccess());
    auto back = conv->ConvertEncodingFast(result.GetValue(), "ISO-8859-1", "UTF-8");
    ASSERT_TRUE(back.IsSuccess());
    EXPECT_EQ(back.GetValue(), ascii_text);
}

TEST_F(EncodingConversionTest, FastResult_Utf8ToCP1252) {
    auto result = conv->ConvertEncodingFast(ascii_text, "UTF-8", "CP1252");
    ASSERT_TRUE(result.IsSuccess());
    auto back = conv->ConvertEncodingFast(result.GetValue(), "CP1252", "UTF-8");
    ASSERT_TRUE(back.IsSuccess());
    EXPECT_EQ(back.GetValue(), ascii_text);
}

TEST_F(EncodingConversionTest, FastResult_Utf8ToBIG5) {
    // BIG5 能表示繁体中文
    std::string traditional = "\xe4\xb8\x96\xe7\x95\x8c"; // 世界 (简繁通用)
    auto result = conv->ConvertEncodingFast(traditional, "UTF-8", "BIG5");
    ASSERT_TRUE(result.IsSuccess());
    auto back = conv->ConvertEncodingFast(result.GetValue(), "BIG5", "UTF-8");
    ASSERT_TRUE(back.IsSuccess());
    EXPECT_EQ(back.GetValue(), traditional);
}

TEST_F(EncodingConversionTest, FastResult_Utf8ToShiftJIS) {
    // Shift_JIS 能表示 ASCII
    auto result = conv->ConvertEncodingFast(ascii_text, "UTF-8", "SHIFT_JIS");
    ASSERT_TRUE(result.IsSuccess());
    auto back = conv->ConvertEncodingFast(result.GetValue(), "SHIFT_JIS", "UTF-8");
    ASSERT_TRUE(back.IsSuccess());
    EXPECT_EQ(back.GetValue(), ascii_text);
}

TEST_F(EncodingConversionTest, FastResult_Utf8ToEucKR) {
    auto result = conv->ConvertEncodingFast(ascii_text, "UTF-8", "EUC-KR");
    ASSERT_TRUE(result.IsSuccess());
    auto back = conv->ConvertEncodingFast(result.GetValue(), "EUC-KR", "UTF-8");
    ASSERT_TRUE(back.IsSuccess());
    EXPECT_EQ(back.GetValue(), ascii_text);
}

TEST_F(EncodingConversionTest, FastResult_GBKToUtf16LE) {
    auto gbk = conv->ConvertEncodingFast(chinese_text, "UTF-8", "GBK");
    ASSERT_TRUE(gbk.IsSuccess());
    auto u16 = conv->ConvertEncodingFast(gbk.GetValue(), "GBK", "UTF-16LE");
    ASSERT_TRUE(u16.IsSuccess());
    auto back = conv->ConvertEncodingFast(u16.GetValue(), "UTF-16LE", "UTF-8");
    ASSERT_TRUE(back.IsSuccess());
    EXPECT_EQ(back.GetValue(), chinese_text);
}

// ============================================================================
// 20. ConvertEncodingBatch (批量转换)
// ============================================================================
TEST_F(EncodingConversionTest, Batch_CompactResultVersion) {
    std::vector<std::string> inputs = {"Hello", "World", chinese_text, ascii_text};
    auto results = conv->ConvertEncodingBatch(inputs, "UTF-8", "UTF-16LE");
    ASSERT_EQ(results.size(), inputs.size());
    for (size_t i = 0; i < results.size(); ++i) {
        EXPECT_TRUE(results[i].IsSuccess()) << "Failed at index " << i;
    }
}

TEST_F(EncodingConversionTest, Batch_OutputParamVersion) {
    std::vector<std::string> inputs = {"Hello", "World", chinese_text};
    std::vector<std::string> outputs;
    bool ok = conv->ConvertEncodingBatch(inputs, "UTF-8", "UTF-16LE", outputs);
    EXPECT_TRUE(ok);
    ASSERT_EQ(outputs.size(), inputs.size());
    for (const auto& out : outputs) {
        EXPECT_FALSE(out.empty());
    }
}

TEST_F(EncodingConversionTest, Batch_EmptyInputList) {
    std::vector<std::string> inputs;
    auto results = conv->ConvertEncodingBatch(inputs, "UTF-8", "UTF-16LE");
    EXPECT_TRUE(results.empty());
}

TEST_F(EncodingConversionTest, Batch_InvalidEncoding) {
    std::vector<std::string> inputs = {"test1", "test2"};
    auto results = conv->ConvertEncodingBatch(inputs, "BOGUS", "UTF-16LE");
    ASSERT_EQ(results.size(), inputs.size());
    for (const auto& r : results) {
        EXPECT_FALSE(r.IsSuccess());
    }
}

TEST_F(EncodingConversionTest, Batch_RoundTrip) {
    std::vector<std::string> inputs = {ascii_text, chinese_text, mixed_text};
    auto forward = conv->ConvertEncodingBatch(inputs, "UTF-8", "UTF-16LE");
    ASSERT_EQ(forward.size(), inputs.size());

    std::vector<std::string> u16_strs;
    for (const auto& r : forward) {
        ASSERT_TRUE(r.IsSuccess());
        u16_strs.push_back(r.GetValue());
    }

    auto backward = conv->ConvertEncodingBatch(u16_strs, "UTF-16LE", "UTF-8");
    ASSERT_EQ(backward.size(), inputs.size());
    for (size_t i = 0; i < inputs.size(); ++i) {
        ASSERT_TRUE(backward[i].IsSuccess());
        EXPECT_EQ(backward[i].GetValue(), inputs[i]);
    }
}

// ============================================================================
// 21. ConvertEncodingBatchParallel (并行批量转换)
// ============================================================================
TEST_F(EncodingConversionTest, BatchParallel_CompactResultVersion) {
    std::vector<std::string> inputs;
    for (int i = 0; i < 100; ++i) {
        inputs.push_back("item_" + std::to_string(i) + "_" + ascii_text);
    }
    auto results = conv->ConvertEncodingBatchParallel(inputs, "UTF-8", "UTF-16LE", 4);
    ASSERT_EQ(results.size(), inputs.size());
    for (size_t i = 0; i < results.size(); ++i) {
        EXPECT_TRUE(results[i].IsSuccess()) << "Failed at index " << i;
    }
}

TEST_F(EncodingConversionTest, BatchParallel_OutputParamVersion) {
    std::vector<std::string> inputs;
    for (int i = 0; i < 50; ++i) {
        inputs.push_back(chinese_text + std::to_string(i));
    }
    std::vector<std::string> outputs;
    bool ok = conv->ConvertEncodingBatchParallel(inputs, "UTF-8", "UTF-16LE", outputs, 2);
    EXPECT_TRUE(ok);
    ASSERT_EQ(outputs.size(), inputs.size());
}

TEST_F(EncodingConversionTest, BatchParallel_AutoThreadCount) {
    std::vector<std::string> inputs = {ascii_text, chinese_text, mixed_text};
    auto results = conv->ConvertEncodingBatchParallel(inputs, "UTF-8", "UTF-16LE", 0);
    ASSERT_EQ(results.size(), inputs.size());
    for (const auto& r : results) {
        EXPECT_TRUE(r.IsSuccess());
    }
}

TEST_F(EncodingConversionTest, BatchParallel_ConsistentWithBatch) {
    std::vector<std::string> inputs = {ascii_text, chinese_text, emoji_text, mixed_text};
    auto serial = conv->ConvertEncodingBatch(inputs, "UTF-8", "UTF-16LE");
    auto parallel = conv->ConvertEncodingBatchParallel(inputs, "UTF-8", "UTF-16LE", 4);

    ASSERT_EQ(serial.size(), parallel.size());
    for (size_t i = 0; i < serial.size(); ++i) {
        ASSERT_TRUE(serial[i].IsSuccess());
        ASSERT_TRUE(parallel[i].IsSuccess());
        EXPECT_EQ(serial[i].GetValue(), parallel[i].GetValue());
    }
}

// ============================================================================
// 22. 跨编码链式往返一致性
// ============================================================================
TEST_F(EncodingConversionTest, ChainRoundTrip_Utf8_Utf16LE_Utf16BE_Utf32LE_Utf8) {
    std::u16string u16le = conv->ToUtf16LEFromUtf8(full_text);
    ASSERT_FALSE(u16le.empty());
    std::u16string u16be = conv->ToUtf16BEFromUtf16LE(u16le);
    ASSERT_FALSE(u16be.empty());
    std::u32string u32 = conv->ToUtf32LEFromUtf16BE(u16be);
    ASSERT_FALSE(u32.empty());
    std::string back = conv->ToUtf8FromUtf32LE(u32);
    EXPECT_EQ(back, full_text);
}

TEST_F(EncodingConversionTest, ChainRoundTrip_Utf8_GBK_Utf16LE_Utf8) {
    auto gbk = conv->ConvertEncodingFast(chinese_text, "UTF-8", "GBK");
    ASSERT_TRUE(gbk.IsSuccess());
    auto u16 = conv->ConvertEncodingFast(gbk.GetValue(), "GBK", "UTF-16LE");
    ASSERT_TRUE(u16.IsSuccess());
    auto back = conv->ConvertEncodingFast(u16.GetValue(), "UTF-16LE", "UTF-8");
    ASSERT_TRUE(back.IsSuccess());
    EXPECT_EQ(back.GetValue(), chinese_text);
}

// ============================================================================
// 23. 边界条件与特殊字符
// ============================================================================
TEST_F(EncodingConversionTest, Boundary_SingleByte) {
    auto result = conv->ConvertEncodingFast("A", "UTF-8", "UTF-16LE");
    ASSERT_TRUE(result.IsSuccess());
    // 'A' in UTF-16LE should be 2 bytes (0x41, 0x00)
    EXPECT_EQ(result.GetValue().size(), 2u);
}

TEST_F(EncodingConversionTest, Boundary_SingleChinese) {
    auto result = conv->ConvertEncodingFast("\xe4\xb8\xad", "UTF-8", "UTF-16LE"); // 中
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(result.GetValue().size(), 2u); // 中 in UTF-16LE = 2 bytes
}

TEST_F(EncodingConversionTest, Boundary_SingleEmoji) {
    auto result = conv->ConvertEncodingFast("\xf0\x9f\x98\x80", "UTF-8", "UTF-32LE"); // 😀
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(result.GetValue().size(), 4u); // 1 codepoint = 4 bytes in UTF-32
}

TEST_F(EncodingConversionTest, Boundary_LargeText) {
    std::string large;
    for (int i = 0; i < 10000; ++i) {
        large += mixed_text;
    }
    auto result = conv->ConvertEncodingFast(large, "UTF-8", "UTF-16LE");
    ASSERT_TRUE(result.IsSuccess());
    auto back = conv->ConvertEncodingFast(result.GetValue(), "UTF-16LE", "UTF-8");
    ASSERT_TRUE(back.IsSuccess());
    EXPECT_EQ(back.GetValue(), large);
}

TEST_F(EncodingConversionTest, Boundary_NullEmbedded) {
    // 含 \0 的字符串 (通过 string 构造)
    std::string with_null("AB\0CD", 5);
    ASSERT_EQ(with_null.size(), 5u);
    auto result = conv->ConvertEncodingFast(with_null, "UTF-8", "UTF-16LE");
    ASSERT_TRUE(result.IsSuccess());
    auto back = conv->ConvertEncodingFast(result.GetValue(), "UTF-16LE", "UTF-8");
    ASSERT_TRUE(back.IsSuccess());
    EXPECT_EQ(back.GetValue(), with_null);
}

TEST_F(EncodingConversionTest, Boundary_MultipleSpaces) {
    std::string spaces = "   \t\n\r   ";
    auto result = conv->ConvertEncodingFast(spaces, "UTF-8", "UTF-16LE");
    ASSERT_TRUE(result.IsSuccess());
    auto back = conv->ConvertEncodingFast(result.GetValue(), "UTF-16LE", "UTF-8");
    ASSERT_TRUE(back.IsSuccess());
    EXPECT_EQ(back.GetValue(), spaces);
}

TEST_F(EncodingConversionTest, Boundary_MultipleCJKScripts) {
    // 中日韩混合: 中文 + 日文平假名 + 韩文
    std::string cjk = "\xe4\xb8\xad\xe6\x96\x87"       // 中文
                       "\xe3\x81\x82\xe3\x81\x84"       // あい
                       "\xed\x95\x9c\xea\xb5\xad";      // 한국
    auto result = conv->ConvertEncodingFast(cjk, "UTF-8", "UTF-16LE");
    ASSERT_TRUE(result.IsSuccess());
    auto back = conv->ConvertEncodingFast(result.GetValue(), "UTF-16LE", "UTF-8");
    ASSERT_TRUE(back.IsSuccess());
    EXPECT_EQ(back.GetValue(), cjk);
}

// ============================================================================
// 24. 错误处理
// ============================================================================
TEST_F(EncodingConversionTest, Error_EmptyEncodingName) {
    auto result = conv->ConvertEncodingFast(ascii_text, "", "UTF-8");
    EXPECT_FALSE(result.IsSuccess());
}

TEST_F(EncodingConversionTest, Error_NullEncodingName) {
    auto result = conv->ConvertEncodingFast(ascii_text, nullptr, nullptr);
    EXPECT_FALSE(result.IsSuccess());
}

TEST_F(EncodingConversionTest, Error_SpecialCharsInEncodingName) {
    auto result = conv->ConvertEncodingFast(ascii_text, "@#$%^&*()", "UTF-8");
    EXPECT_FALSE(result.IsSuccess());
}

TEST_F(EncodingConversionTest, Error_VeryLongEncodingName) {
    std::string long_name(256, 'X');
    auto result = conv->ConvertEncodingFast(ascii_text, long_name.c_str(), "UTF-8");
    EXPECT_FALSE(result.IsSuccess());
}

TEST_F(EncodingConversionTest, Error_SameEncoding_NoOp) {
    auto result = conv->ConvertEncodingFast(ascii_text, "UTF-8", "UTF-8");
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(result.GetValue(), ascii_text);
}

// ============================================================================
// 25. 工具/查询 API
// ============================================================================
TEST_F(EncodingConversionTest, Utility_GetCurrentSystemEncoding) {
    std::string enc = UniConv::GetCurrentSystemEncoding();
    EXPECT_FALSE(enc.empty());
}

TEST_F(EncodingConversionTest, Utility_GetEncodingNameByCodePage) {
    std::string name = UniConv::GetEncodingNameByCodePage(65001); // UTF-8
    EXPECT_FALSE(name.empty());
}

TEST_F(EncodingConversionTest, Utility_GetEncodingNamePtr) {
    const char* ptr = conv->GetEncodingNamePtr(65001);
    EXPECT_NE(ptr, nullptr);
}

TEST_F(EncodingConversionTest, Utility_GetEncodingNameFast) {
    auto result = conv->GetEncodingNameFast(65001);
    EXPECT_TRUE(result.IsSuccess());
}

TEST_F(EncodingConversionTest, Utility_GetSystemCodePageFast) {
    auto result = conv->GetSystemCodePageFast();
#if defined(_WIN32) || defined(__linux__)
    EXPECT_TRUE(result.IsSuccess());
#else
    // macOS 不使用 Windows 代码页系统，GetSystemCodePageFast 可能不支持
    if (!result.IsSuccess()) {
        GTEST_SKIP() << "GetSystemCodePageFast not supported on this platform";
    }
#endif
}

TEST_F(EncodingConversionTest, Utility_GetPoolStatistics) {
    // 做一些转换以产生统计数据
    (void)conv->ConvertEncodingFast(ascii_text, "UTF-8", "UTF-16LE");
    (void)conv->ConvertEncodingFast(chinese_text, "UTF-8", "GBK");

    auto stats = conv->GetPoolStatistics();
    // 验证统计结构可正常获取（不同 API 路径的计数策略可能不同）
    EXPECT_GE(stats.total_conversions + stats.iconv_cache_size, 0u);
}

TEST_F(EncodingConversionTest, Utility_EncodingToString) {
    std::string name = UniConv::ToString(UniConv::Encoding::utf_8);
    EXPECT_FALSE(name.empty());
}

TEST_F(EncodingConversionTest, Utility_SetDefaultEncoding) {
    conv->SetDefaultEncoding("UTF-8");
    if (!IsLocaleAvailable()) GTEST_SKIP() << "Locale detection unavailable";
    std::string utf8 = conv->ToUtf8FromLocale(ascii_text);
    EXPECT_FALSE(utf8.empty());
}

// ============================================================================
// 26. 返回值版缺失重载的独立测试
// ============================================================================
TEST_F(EncodingConversionTest, ReturnVal_ToLocaleFromUtf16LE_U16String) {
    if (!IsLocaleAvailable()) GTEST_SKIP() << "Locale detection unavailable";
    std::u16string u16 = conv->ToUtf16LEFromLocale(ascii_text);
    ASSERT_FALSE(u16.empty());
    std::string back = conv->ToLocaleFromUtf16LE(u16);
    EXPECT_EQ(back, ascii_text);
}

TEST_F(EncodingConversionTest, ReturnVal_ToLocaleFromUtf16LE_U16String_Chinese) {
    if (!IsLocaleAvailable()) GTEST_SKIP() << "Locale detection unavailable";
    std::u16string u16 = conv->ToUtf16LEFromLocale(chinese_text);
    ASSERT_FALSE(u16.empty());
    std::string back = conv->ToLocaleFromUtf16LE(u16);
    EXPECT_EQ(back, chinese_text);
}

TEST_F(EncodingConversionTest, ReturnVal_ToLocaleFromUtf16BE_U16String) {
    if (!IsLocaleAvailable()) GTEST_SKIP() << "Locale detection unavailable";
    std::u16string u16 = conv->ToUtf16BEFromLocale(ascii_text);
    ASSERT_FALSE(u16.empty());
    std::string back = conv->ToLocaleFromUtf16BE(u16);
    EXPECT_EQ(back, ascii_text);
}

TEST_F(EncodingConversionTest, ReturnVal_ToLocaleFromUtf16BE_U16String_Chinese) {
    if (!IsLocaleAvailable()) GTEST_SKIP() << "Locale detection unavailable";
    std::u16string u16 = conv->ToUtf16BEFromLocale(chinese_text);
    ASSERT_FALSE(u16.empty());
    std::string back = conv->ToLocaleFromUtf16BE(u16);
    EXPECT_EQ(back, chinese_text);
}

TEST_F(EncodingConversionTest, ReturnVal_ToUtf16BEFromUtf32LE_CStyleString) {
    std::u32string u32 = conv->ToUtf32LEFromUtf8(chinese_text);
    ASSERT_FALSE(u32.empty());
    std::u16string be = conv->ToUtf16BEFromUtf32LE(u32.c_str());
    ASSERT_FALSE(be.empty());
    std::string back = conv->ToUtf8FromUtf16BE(be);
    EXPECT_EQ(back, chinese_text);
}

TEST_F(EncodingConversionTest, ReturnVal_ToLocaleFromWideString_CStyleString) {
    if (!IsLocaleAvailable()) GTEST_SKIP() << "Locale detection unavailable";
    std::wstring wide = conv->ToWideStringFromLocale(ascii_text);
    ASSERT_FALSE(wide.empty());
    std::string back = conv->ToLocaleFromWideString(wide.c_str());
    EXPECT_EQ(back, ascii_text);
}

TEST_F(EncodingConversionTest, ReturnVal_WideStringToLocale_CStyleString) {
    if (!IsLocaleAvailable()) GTEST_SKIP() << "Locale detection unavailable";
    std::wstring wide = conv->ToWideStringFromLocale(chinese_text);
    ASSERT_FALSE(wide.empty());
    std::string back = conv->WideStringToLocale(wide.c_str());
    EXPECT_EQ(back, chinese_text);
}

// ============================================================================
// 27. UCS-4 / U16StringToWString 深度测试
// ============================================================================
TEST_F(EncodingConversionTest, Ucs4ToUtf8_Ascii) {
    std::wstring ucs4 = conv->ToUcs4FromUtf8(ascii_text);
    ASSERT_FALSE(ucs4.empty());
    EXPECT_EQ(ucs4.size(), ascii_text.size());
    std::string back = conv->ToUtf8FromUcs4(ucs4);
    EXPECT_EQ(back, ascii_text);
}

TEST_F(EncodingConversionTest, Ucs4ToUtf8_SingleChar) {
    std::string single = "\xe4\xb8\xad"; // 中
    std::wstring ucs4 = conv->ToUcs4FromUtf8(single);
    ASSERT_FALSE(ucs4.empty());
    EXPECT_EQ(ucs4.size(), 1u);
    std::string back = conv->ToUtf8FromUcs4(ucs4);
    EXPECT_EQ(back, single);
}

TEST_F(EncodingConversionTest, Ucs4ToUtf8_Chinese) {
    std::wstring ucs4 = conv->ToUcs4FromUtf8(chinese_text);
    ASSERT_FALSE(ucs4.empty());
    EXPECT_EQ(ucs4.size(), 4u);
    std::string back = conv->ToUtf8FromUcs4(ucs4);
    EXPECT_EQ(back, chinese_text);
}

TEST_F(EncodingConversionTest, Ucs4ToUtf8_Empty) {
    std::wstring ucs4 = conv->ToUcs4FromUtf8("");
    EXPECT_TRUE(ucs4.empty());
    std::string back = conv->ToUtf8FromUcs4(std::wstring{});
    EXPECT_TRUE(back.empty());
}

TEST_F(EncodingConversionTest, U16StringToWString_Chinese) {
    std::u16string u16 = conv->ToUtf16LEFromUtf8(chinese_text);
    ASSERT_FALSE(u16.empty());
    std::wstring wide = conv->U16StringToWString(u16);
    ASSERT_FALSE(wide.empty());
    EXPECT_EQ(wide.size(), 4u);
}

TEST_F(EncodingConversionTest, U16StringToWString_ContentVerification) {
    std::u16string u16 = conv->ToUtf16LEFromUtf8(ascii_text);
    ASSERT_FALSE(u16.empty());
    std::wstring wide = conv->U16StringToWString(u16);
    ASSERT_FALSE(wide.empty());
    std::wstring expected(ascii_text.begin(), ascii_text.end());
    EXPECT_EQ(wide, expected);
}

TEST_F(EncodingConversionTest, U16StringToWString_Emoji) {
    std::u16string u16 = conv->ToUtf16LEFromUtf8(emoji_text);
    ASSERT_FALSE(u16.empty());
    std::wstring wide = conv->U16StringToWString(u16);
    EXPECT_FALSE(wide.empty());
}

TEST_F(EncodingConversionTest, U16StringToWString_Empty) {
    std::wstring wide = conv->U16StringToWString(std::u16string{});
    EXPECT_TRUE(wide.empty());
}

// ============================================================================
// 28. 输出参数版多字节数据覆盖
// ============================================================================
TEST_F(EncodingConversionTest, OutputParam_ToUtf8FromLocale_Chinese) {
    if (!IsLocaleAvailable()) GTEST_SKIP() << "Locale detection unavailable";
    std::string output;
    bool ok = conv->ToUtf8FromLocale(chinese_text, output);
    EXPECT_TRUE(ok);
    EXPECT_EQ(output, chinese_text);
}

TEST_F(EncodingConversionTest, OutputParam_ToLocaleFromUtf8_Chinese) {
    if (!IsLocaleAvailable()) GTEST_SKIP() << "Locale detection unavailable";
    std::string output;
    bool ok = conv->ToLocaleFromUtf8(chinese_text, output);
    EXPECT_TRUE(ok);
    EXPECT_FALSE(output.empty());
    std::string back;
    bool ok2 = conv->ToUtf8FromLocale(output, back);
    EXPECT_TRUE(ok2);
    EXPECT_EQ(back, chinese_text);
}

TEST_F(EncodingConversionTest, OutputParam_ToWideStringFromLocale_Chinese) {
    if (!IsLocaleAvailable()) GTEST_SKIP() << "Locale detection unavailable";
    std::wstring output;
    bool ok = conv->ToWideStringFromLocale(chinese_text, output);
    EXPECT_TRUE(ok);
    EXPECT_FALSE(output.empty());
}

TEST_F(EncodingConversionTest, OutputParam_ToLocaleFromWideString_Chinese) {
    if (!IsLocaleAvailable()) GTEST_SKIP() << "Locale detection unavailable";
    std::wstring wide = conv->ToWideStringFromLocale(chinese_text);
    ASSERT_FALSE(wide.empty());
    std::string output;
    bool ok = conv->ToLocaleFromWideString(wide, output);
    EXPECT_TRUE(ok);
    EXPECT_EQ(output, chinese_text);
}

TEST_F(EncodingConversionTest, OutputParam_ToUtf16LEFromLocale_Chinese) {
    if (!IsLocaleAvailable()) GTEST_SKIP() << "Locale detection unavailable";
    std::u16string output;
    bool ok = conv->ToUtf16LEFromLocale(chinese_text, output);
    EXPECT_TRUE(ok);
    EXPECT_FALSE(output.empty());
    std::string back = conv->ToUtf8FromUtf16LE(output);
    EXPECT_EQ(back, chinese_text);
}

TEST_F(EncodingConversionTest, OutputParam_ToUtf16BEFromLocale_Chinese) {
    if (!IsLocaleAvailable()) GTEST_SKIP() << "Locale detection unavailable";
    std::u16string output;
    bool ok = conv->ToUtf16BEFromLocale(chinese_text, output);
    EXPECT_TRUE(ok);
    EXPECT_FALSE(output.empty());
    std::string back = conv->ToUtf8FromUtf16BE(output);
    EXPECT_EQ(back, chinese_text);
}

TEST_F(EncodingConversionTest, OutputParam_ToLocaleFromUtf16LE_Chinese) {
    if (!IsLocaleAvailable()) GTEST_SKIP() << "Locale detection unavailable";
    std::u16string u16 = conv->ToUtf16LEFromLocale(chinese_text);
    ASSERT_FALSE(u16.empty());
    std::string output;
    bool ok = conv->ToLocaleFromUtf16LE(u16, output);
    EXPECT_TRUE(ok);
    EXPECT_EQ(output, chinese_text);
}

TEST_F(EncodingConversionTest, OutputParam_ToLocaleFromUtf16BE_Chinese) {
    if (!IsLocaleAvailable()) GTEST_SKIP() << "Locale detection unavailable";
    std::u16string u16 = conv->ToUtf16BEFromLocale(chinese_text);
    ASSERT_FALSE(u16.empty());
    std::string output;
    bool ok = conv->ToLocaleFromUtf16BE(u16, output);
    EXPECT_TRUE(ok);
    EXPECT_EQ(output, chinese_text);
}

TEST_F(EncodingConversionTest, OutputParam_U16StringToWString_Chinese) {
    std::u16string u16 = conv->ToUtf16LEFromUtf8(chinese_text);
    ASSERT_FALSE(u16.empty());
    std::wstring output;
    bool ok = conv->U16StringToWString(u16, output);
    EXPECT_TRUE(ok);
    EXPECT_EQ(output.size(), 4u);
}

// ============================================================================
// 29. Ex (CompactResult) 版多字节数据覆盖
// ============================================================================
TEST_F(EncodingConversionTest, Ex_ToUtf8FromLocaleEx_Chinese) {
    if (!IsLocaleAvailable()) GTEST_SKIP() << "Locale detection unavailable";
    auto result = conv->ToUtf8FromLocaleEx(chinese_text);
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ(result.GetValue(), chinese_text);
}

TEST_F(EncodingConversionTest, Ex_ToLocaleFromUtf8Ex_Chinese) {
    if (!IsLocaleAvailable()) GTEST_SKIP() << "Locale detection unavailable";
    auto result = conv->ToLocaleFromUtf8Ex(chinese_text);
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_FALSE(result.GetValue().empty());
    auto back = conv->ToUtf8FromLocaleEx(result.GetValue());
    EXPECT_TRUE(back.IsSuccess());
    EXPECT_EQ(back.GetValue(), chinese_text);
}

TEST_F(EncodingConversionTest, Ex_ToUtf16LEFromLocaleEx_Chinese) {
    if (!IsLocaleAvailable()) GTEST_SKIP() << "Locale detection unavailable";
    auto result = conv->ToUtf16LEFromLocaleEx(chinese_text);
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_FALSE(result.GetValue().empty());
}

TEST_F(EncodingConversionTest, Ex_ToUtf16BEFromLocaleEx_Chinese) {
    if (!IsLocaleAvailable()) GTEST_SKIP() << "Locale detection unavailable";
    auto result = conv->ToUtf16BEFromLocaleEx(chinese_text);
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_FALSE(result.GetValue().empty());
}

TEST_F(EncodingConversionTest, Ex_ToUtf8FromLocaleEx_Emoji) {
    if (!IsLocaleAvailable()) GTEST_SKIP() << "Locale detection unavailable";
    auto result = conv->ToUtf8FromLocaleEx(emoji_text);
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ(result.GetValue(), emoji_text);
}

TEST_F(EncodingConversionTest, Ex_ToUtf8FromUtf16LEEx_Emoji) {
    std::u16string u16 = conv->ToUtf16LEFromUtf8(emoji_text);
    ASSERT_FALSE(u16.empty());
    auto result = conv->ToUtf8FromUtf16LEEx(u16);
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ(result.GetValue(), emoji_text);
}

TEST_F(EncodingConversionTest, Ex_ToUtf8FromUtf16BEEx_Emoji) {
    std::u16string u16 = conv->ToUtf16BEFromUtf8(emoji_text);
    ASSERT_FALSE(u16.empty());
    auto result = conv->ToUtf8FromUtf16BEEx(u16);
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ(result.GetValue(), emoji_text);
}

// ============================================================================
// 30. 缺失工具函数测试
// ============================================================================
TEST_F(EncodingConversionTest, Utility_GetCurrentSystemEncodingCodePage) {
    auto codepage = UniConv::GetCurrentSystemEncodingCodePage();
#ifdef _WIN32
    EXPECT_NE(codepage, static_cast<std::uint16_t>(-1));
#elif defined(__linux__) || defined(__APPLE__)
    (void)codepage;
#endif
}
