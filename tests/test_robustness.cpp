/**
 * @file test_robustness.cpp
 * @brief UniConv 健壮性、线程安全与回归测试
 *
 * 测试维度:
 *   1. 畸形/非法输入数据 — 截断序列、孤立代理项、无效字节等
 *   2. 线程安全 — 多线程并发转换、共享实例、高压力
 *   3. 回归测试 — 固化已修复 bug，防止重新引入
 */

#include <gtest/gtest.h>
#include <UniConv/UniConv.h>
#include <memory>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <numeric>

// ============================================================================
// 测试夹具
// ============================================================================
class RobustnessTest : public ::testing::Test {
protected:
    void SetUp() override {
        conv = UniConv::Create();
        ASSERT_NE(conv, nullptr);
    }

    std::unique_ptr<UniConv> conv;

    const std::string ascii_text   = "Hello, World!";
    const std::string chinese_text = "\xe4\xbd\xa0\xe5\xa5\xbd\xe4\xb8\x96\xe7\x95\x8c"; // 你好世界
    const std::string emoji_text   = "\xf0\x9f\x98\x80\xf0\x9f\x8c\x8d\xf0\x9f\x9a\x80"; // 😀🌍🚀
};

// ############################################################################
//  Part 1: 畸形/非法输入数据 (Malformed Input)
// ############################################################################

// ----------------------------------------------------------------------------
// 1.1 截断的 UTF-8 多字节序列
// ----------------------------------------------------------------------------
TEST_F(RobustnessTest, Malformed_TruncatedUtf8_2ByteStart) {
    // 你 = E4 BD A0, 只取前 2 字节
    std::string truncated("\xe4\xbd", 2);
    auto result = conv->ConvertEncodingFast(truncated, "UTF-8", "UTF-16LE");
    // 应该返回失败或部分转换，不应崩溃
    (void)result;
    SUCCEED();
}

TEST_F(RobustnessTest, Malformed_TruncatedUtf8_1ByteStart) {
    // 4 字节 emoji 序列只取 1 字节
    std::string truncated("\xf0", 1);
    auto result = conv->ConvertEncodingFast(truncated, "UTF-8", "UTF-16LE");
    (void)result;
    SUCCEED();
}

TEST_F(RobustnessTest, Malformed_TruncatedUtf8_MidSequence) {
    // 合法文本 + 截断的多字节尾部
    std::string data = "Hello\xe4\xbd"; // "Hello" + 截断的 "你"
    auto result = conv->ConvertEncodingFast(data, "UTF-8", "UTF-16LE");
    (void)result;
    SUCCEED();
}

// ----------------------------------------------------------------------------
// 1.2 无效 UTF-8 字节
// ----------------------------------------------------------------------------
TEST_F(RobustnessTest, Malformed_InvalidUtf8_FE) {
    std::string invalid("\xfe\xfe\xff\xff", 4);
    auto result = conv->ConvertEncodingFast(invalid, "UTF-8", "UTF-16LE");
    EXPECT_FALSE(result.IsSuccess());
}

TEST_F(RobustnessTest, Malformed_InvalidUtf8_OverlongEncoding) {
    // C0 AF = 过长编码的 '/' (U+002F)
    std::string overlong("\xc0\xaf", 2);
    auto result = conv->ConvertEncodingFast(overlong, "UTF-8", "UTF-16LE");
    // iconv 应拒绝过长编码
    (void)result;
    SUCCEED();
}

TEST_F(RobustnessTest, Malformed_InvalidUtf8_ContinuationWithoutStart) {
    // 0x80-0xBF 是延续字节，不应出现在序列开头
    std::string bad("\x80\x81\x82", 3);
    auto result = conv->ConvertEncodingFast(bad, "UTF-8", "UTF-16LE");
    EXPECT_FALSE(result.IsSuccess());
}

// ----------------------------------------------------------------------------
// 1.3 UTF-16 奇数长度字节流
// ----------------------------------------------------------------------------
TEST_F(RobustnessTest, Malformed_OddLengthUtf16LE) {
    // 3 字节不是 UTF-16 的合法长度
    std::string odd_bytes("ABC", 3);
    auto result = conv->ConvertEncodingFast(odd_bytes, "UTF-16LE", "UTF-8");
    // 应返回失败或截断处理，不应崩溃
    (void)result;
    SUCCEED();
}

TEST_F(RobustnessTest, Malformed_OddLengthUtf16BE) {
    std::string odd_bytes("ABCDE", 5);
    auto result = conv->ConvertEncodingFast(odd_bytes, "UTF-16BE", "UTF-8");
    (void)result;
    SUCCEED();
}

// ----------------------------------------------------------------------------
// 1.4 孤立代理项 (Isolated Surrogates)
// ----------------------------------------------------------------------------
TEST_F(RobustnessTest, Malformed_IsolatedHighSurrogate) {
    // U+D800 in UTF-16LE: 00 D8 — 高代理项，无配对低代理
    char16_t surrogate[] = { 0x0048, 0xD800, 0x0069 }; // "H" + lone high + "i"
    std::string bytes(reinterpret_cast<const char*>(surrogate), sizeof(surrogate));
    auto result = conv->ConvertEncodingFast(bytes, "UTF-16LE", "UTF-8");
    // iconv 对孤立代理项的处理因实现而异，不应崩溃
    (void)result;
    SUCCEED();
}

TEST_F(RobustnessTest, Malformed_IsolatedLowSurrogate) {
    // U+DC00 低代理项，无配对高代理
    char16_t surrogate[] = { 0x0048, 0xDC00, 0x0069 };
    std::string bytes(reinterpret_cast<const char*>(surrogate), sizeof(surrogate));
    auto result = conv->ConvertEncodingFast(bytes, "UTF-16LE", "UTF-8");
    (void)result;
    SUCCEED();
}

// ----------------------------------------------------------------------------
// 1.5 混合有效/无效数据
// ----------------------------------------------------------------------------
TEST_F(RobustnessTest, Malformed_ValidPrefixInvalidSuffix) {
    // 合法 UTF-8 + 无效尾部
    std::string data = chinese_text + "\xff\xfe";
    auto result = conv->ConvertEncodingFast(data, "UTF-8", "UTF-16LE");
    // 至少不应崩溃
    (void)result;
    SUCCEED();
}

TEST_F(RobustnessTest, Malformed_AllZeroBytes) {
    std::string zeros(16, '\0');
    auto result = conv->ConvertEncodingFast(zeros, "UTF-8", "UTF-16LE");
    EXPECT_TRUE(result.IsSuccess());
    auto back = conv->ConvertEncodingFast(result.GetValue(), "UTF-16LE", "UTF-8");
    EXPECT_TRUE(back.IsSuccess());
    EXPECT_EQ(back.GetValue(), zeros);
}

// ----------------------------------------------------------------------------
// 1.6 输出参数版对畸形输入的处理
// ----------------------------------------------------------------------------
TEST_F(RobustnessTest, Malformed_OutputParam_TruncatedUtf8) {
    std::string truncated("\xe4\xbd", 2);
    std::string output;
    auto ec = conv->ConvertEncodingFast(truncated, "UTF-8", "UTF-16LE", output);
    // 不应崩溃，返回错误码
    (void)ec;
    SUCCEED();
}

TEST_F(RobustnessTest, Malformed_OutputParam_InvalidToUtf8FromUtf16LE) {
    // 直接传入非法的 u16string 数据（孤立代理项）
    std::u16string bad = { 0x0041, 0xD800, 0x0042 };
    std::string output;
    bool ok = conv->ToUtf8FromUtf16LE(bad, output);
    // 不应崩溃
    (void)ok;
    SUCCEED();
}

// ----------------------------------------------------------------------------
// 1.7 不可表示的字符（编码范围不匹配）
// ----------------------------------------------------------------------------
TEST_F(RobustnessTest, Malformed_ChineseToISO8859_1) {
    // 中文不可用 ISO-8859-1 表示
    auto result = conv->ConvertEncodingFast(chinese_text, "UTF-8", "ISO-8859-1");
    EXPECT_FALSE(result.IsSuccess());
}

TEST_F(RobustnessTest, Malformed_EmojiToGBK) {
    // Emoji 不在 GBK 编码范围内
    auto result = conv->ConvertEncodingFast(emoji_text, "UTF-8", "GBK");
    EXPECT_FALSE(result.IsSuccess());
}

// ############################################################################
//  Part 2: 线程安全 (Thread Safety)
// ############################################################################

// ----------------------------------------------------------------------------
// 2.1 多线程同编码对转换
// ----------------------------------------------------------------------------
TEST_F(RobustnessTest, ThreadSafety_SameEncodingPair) {
    constexpr int kThreads = 8;
    constexpr int kIterations = 500;
    std::atomic<int> failures{0};

    auto worker = [&](int id) {
        for (int i = 0; i < kIterations; ++i) {
            std::string input = "Thread" + std::to_string(id) + "_iter" + std::to_string(i) + "_" + chinese_text;
            auto result = conv->ConvertEncodingFast(input, "UTF-8", "UTF-16LE");
            if (!result.IsSuccess()) {
                failures.fetch_add(1);
                continue;
            }
            auto back = conv->ConvertEncodingFast(result.GetValue(), "UTF-16LE", "UTF-8");
            if (!back.IsSuccess() || back.GetValue() != input) {
                failures.fetch_add(1);
            }
        }
    };

    std::vector<std::thread> threads;
    threads.reserve(kThreads);
    for (int i = 0; i < kThreads; ++i) {
        threads.emplace_back(worker, i);
    }
    for (auto& t : threads) {
        t.join();
    }

    EXPECT_EQ(failures.load(), 0) << "Thread safety violation: " << failures.load()
                                   << " failures in " << kThreads * kIterations << " operations";
}

// ----------------------------------------------------------------------------
// 2.2 多线程不同编码对转换
// ----------------------------------------------------------------------------
TEST_F(RobustnessTest, ThreadSafety_DifferentEncodingPairs) {
    constexpr int kIterations = 200;
    std::atomic<int> failures{0};

    struct EncodingPair { const char* from; const char* to; std::string input; };
    std::vector<EncodingPair> pairs = {
        {"UTF-8",    "UTF-16LE", chinese_text},
        {"UTF-8",    "UTF-16BE", chinese_text},
        {"UTF-8",    "UTF-32LE", ascii_text},
        {"UTF-8",    "GBK",      chinese_text},
        {"UTF-8",    "SHIFT_JIS", ascii_text},
        {"UTF-8",    "EUC-KR",   ascii_text},
        {"UTF-8",    "ISO-8859-1", ascii_text},
        {"UTF-8",    "BIG5",     "\xe4\xb8\x96\xe7\x95\x8c"}, // 世界
    };

    auto worker = [&](const EncodingPair& pair) {
        for (int i = 0; i < kIterations; ++i) {
            auto fwd = conv->ConvertEncodingFast(pair.input, pair.from, pair.to);
            if (!fwd.IsSuccess()) { failures.fetch_add(1); continue; }
            auto back = conv->ConvertEncodingFast(fwd.GetValue(), pair.to, pair.from);
            if (!back.IsSuccess() || back.GetValue() != pair.input) {
                failures.fetch_add(1);
            }
        }
    };

    std::vector<std::thread> threads;
    threads.reserve(pairs.size());
    for (const auto& p : pairs) {
        threads.emplace_back(worker, std::cref(p));
    }
    for (auto& t : threads) {
        t.join();
    }

    EXPECT_EQ(failures.load(), 0) << failures.load() << " failures across "
                                   << pairs.size() << " encoding pairs";
}

// ----------------------------------------------------------------------------
// 2.3 多线程使用各自独立实例
// ----------------------------------------------------------------------------
TEST_F(RobustnessTest, ThreadSafety_IndependentInstances) {
    constexpr int kThreads = 8;
    constexpr int kIterations = 300;
    std::atomic<int> failures{0};

    auto worker = [&]() {
        auto local_conv = UniConv::Create();
        if (!local_conv) { failures.fetch_add(kIterations); return; }

        for (int i = 0; i < kIterations; ++i) {
            auto result = local_conv->ConvertEncodingFast(chinese_text, "UTF-8", "UTF-16LE");
            if (!result.IsSuccess()) { failures.fetch_add(1); continue; }
            auto back = local_conv->ConvertEncodingFast(result.GetValue(), "UTF-16LE", "UTF-8");
            if (!back.IsSuccess() || back.GetValue() != chinese_text) {
                failures.fetch_add(1);
            }
        }
    };

    std::vector<std::thread> threads;
    threads.reserve(kThreads);
    for (int i = 0; i < kThreads; ++i) {
        threads.emplace_back(worker);
    }
    for (auto& t : threads) {
        t.join();
    }

    EXPECT_EQ(failures.load(), 0);
}

// ----------------------------------------------------------------------------
// 2.4 多线程混合 API 风格
// ----------------------------------------------------------------------------
TEST_F(RobustnessTest, ThreadSafety_MixedApiStyles) {
    constexpr int kIterations = 200;
    std::atomic<int> failures{0};

    // 返回值版
    auto worker_return = [&]() {
        for (int i = 0; i < kIterations; ++i) {
            std::u16string u16 = conv->ToUtf16LEFromUtf8(chinese_text);
            std::string back = conv->ToUtf8FromUtf16LE(u16);
            if (back != chinese_text) failures.fetch_add(1);
        }
    };

    // 输出参数版
    auto worker_outparam = [&]() {
        for (int i = 0; i < kIterations; ++i) {
            std::u16string u16;
            conv->ToUtf16LEFromUtf8(chinese_text, u16);
            std::string back;
            conv->ToUtf8FromUtf16LE(u16, back);
            if (back != chinese_text) failures.fetch_add(1);
        }
    };

    // Ex 版
    auto worker_ex = [&]() {
        for (int i = 0; i < kIterations; ++i) {
            auto fwd = conv->ConvertEncodingFast(chinese_text, "UTF-8", "GBK");
            if (!fwd.IsSuccess()) { failures.fetch_add(1); continue; }
            auto back = conv->ConvertEncodingFast(fwd.GetValue(), "GBK", "UTF-8");
            if (!back.IsSuccess() || back.GetValue() != chinese_text) {
                failures.fetch_add(1);
            }
        }
    };

    std::vector<std::thread> threads;
    threads.emplace_back(worker_return);
    threads.emplace_back(worker_return);
    threads.emplace_back(worker_outparam);
    threads.emplace_back(worker_outparam);
    threads.emplace_back(worker_ex);
    threads.emplace_back(worker_ex);

    for (auto& t : threads) {
        t.join();
    }

    EXPECT_EQ(failures.load(), 0);
}

// ----------------------------------------------------------------------------
// 2.5 高并发压力测试
// ----------------------------------------------------------------------------
TEST_F(RobustnessTest, ThreadSafety_HighConcurrencyStress) {
    constexpr int kThreads = 16;
    constexpr int kIterations = 100;
    std::atomic<int> total_ops{0};
    std::atomic<int> failures{0};

    auto worker = [&](int id) {
        for (int i = 0; i < kIterations; ++i) {
            std::string input = std::to_string(id) + "_" + std::to_string(i) + "_" + emoji_text;

            // UTF-8 → UTF-16LE → UTF-16BE → UTF-32LE → UTF-8 链式往返
            std::u16string u16le = conv->ToUtf16LEFromUtf8(input);
            if (u16le.empty()) { failures.fetch_add(1); continue; }

            std::u16string u16be = conv->ToUtf16BEFromUtf16LE(u16le);
            if (u16be.empty()) { failures.fetch_add(1); continue; }

            std::u32string u32 = conv->ToUtf32LEFromUtf16BE(u16be);
            if (u32.empty()) { failures.fetch_add(1); continue; }

            std::string back = conv->ToUtf8FromUtf32LE(u32);
            if (back != input) { failures.fetch_add(1); continue; }

            total_ops.fetch_add(1);
        }
    };

    std::vector<std::thread> threads;
    threads.reserve(kThreads);
    for (int i = 0; i < kThreads; ++i) {
        threads.emplace_back(worker, i);
    }
    for (auto& t : threads) {
        t.join();
    }

    EXPECT_EQ(failures.load(), 0);
    EXPECT_EQ(total_ops.load(), kThreads * kIterations);
}

// ############################################################################
//  Part 3: 回归测试 (Regression)
// ############################################################################

// ----------------------------------------------------------------------------
// 3.1 UTF-16BE 输出参数版字节序一致性
//     (回归: 之前手动 (ch>>8)|ch 构造 char16_t 导致双重字节交换)
// ----------------------------------------------------------------------------
TEST_F(RobustnessTest, Regression_Utf16BE_OutputParam_ByteOrder) {
    // 返回值版 vs 输出参数版应产生完全一致的数据
    std::u16string ret_val = conv->ToUtf16BEFromUtf8(chinese_text);
    ASSERT_FALSE(ret_val.empty());

    std::u16string out_param;
    bool ok = conv->ToUtf16BEFromUtf8(chinese_text, out_param);
    ASSERT_TRUE(ok);

    EXPECT_EQ(ret_val, out_param) << "Return-value and output-param versions must produce identical bytes";
}

TEST_F(RobustnessTest, Regression_Utf16BE_OutputParam_RoundTrip_Chinese) {
    std::u16string u16;
    bool ok1 = conv->ToUtf16BEFromUtf8(chinese_text, u16);
    ASSERT_TRUE(ok1);

    std::string back;
    bool ok2 = conv->ToUtf8FromUtf16BE(u16, back);
    ASSERT_TRUE(ok2);
    EXPECT_EQ(back, chinese_text);
}

TEST_F(RobustnessTest, Regression_Utf16BE_Ex_RoundTrip_Chinese) {
    auto fwd = conv->ToUtf8FromUtf16BEEx(conv->ToUtf16BEFromUtf8(chinese_text));
    ASSERT_TRUE(fwd.IsSuccess());
    EXPECT_EQ(fwd.GetValue(), chinese_text);
}

// ----------------------------------------------------------------------------
// 3.2 ToLocaleFromUtf16BE 输出参数版字节序
//     (回归: 同类手动字节提取 bug)
// ----------------------------------------------------------------------------
TEST_F(RobustnessTest, Regression_ToLocaleFromUtf16BE_OutputParam) {
    std::u16string u16 = conv->ToUtf16BEFromUtf8(chinese_text);
    ASSERT_FALSE(u16.empty());

    // 返回值版
    std::string ret_val = conv->ToLocaleFromUtf16BE(u16);
    // 输出参数版
    std::string out_param;
    bool ok = conv->ToLocaleFromUtf16BE(u16, out_param);
    ASSERT_TRUE(ok);

    EXPECT_EQ(ret_val, out_param);
}

// ----------------------------------------------------------------------------
// 3.3 macOS locale 检测
//     (回归: GetCurrentSystemEncoding 在 macOS 上返回 "Encoding not found.")
// ----------------------------------------------------------------------------
TEST_F(RobustnessTest, Regression_LocaleDetection_NotPlaceholder) {
    std::string enc = UniConv::GetCurrentSystemEncoding();
    EXPECT_FALSE(enc.empty());
    EXPECT_NE(enc, "Encoding not found.") << "macOS locale detection regression";
}

TEST_F(RobustnessTest, Regression_LocaleConversion_NotEmpty) {
    std::string result = conv->ToUtf8FromLocale(ascii_text);
    EXPECT_FALSE(result.empty()) << "Locale conversion should work after locale detection fix";
    EXPECT_EQ(result, ascii_text);
}

// ----------------------------------------------------------------------------
// 3.4 wchar_t 编码映射
//     (回归: ToString(wchar_t_encoding) 返回不受支持的 "WCHAR_T")
// ----------------------------------------------------------------------------
TEST_F(RobustnessTest, Regression_WcharEncoding_PlatformCorrect) {
    std::string wchar_enc = UniConv::ToString(UniConv::Encoding::wchar_t_encoding);
    EXPECT_FALSE(wchar_enc.empty());
    EXPECT_NE(wchar_enc, "WCHAR_T") << "Should not be the unsupported raw WCHAR_T string";

    if constexpr (sizeof(wchar_t) == 2) {
        EXPECT_EQ(wchar_enc, "UTF-16LE");
    } else if constexpr (sizeof(wchar_t) == 4) {
        EXPECT_EQ(wchar_enc, "UTF-32LE");
    }
}

// ----------------------------------------------------------------------------
// 3.5 返回值版与输出参数版一致性（全面检查）
//     (回归: reinterpret_cast vs 手动字节构造不一致)
// ----------------------------------------------------------------------------
TEST_F(RobustnessTest, Regression_Consistency_Utf16LE_ReturnVsOutputParam) {
    std::u16string ret_val = conv->ToUtf16LEFromUtf8(chinese_text);
    std::u16string out_param;
    conv->ToUtf16LEFromUtf8(chinese_text, out_param);
    EXPECT_EQ(ret_val, out_param);
}

TEST_F(RobustnessTest, Regression_Consistency_Utf16BE_ReturnVsOutputParam) {
    std::u16string ret_val = conv->ToUtf16BEFromUtf8(chinese_text);
    std::u16string out_param;
    conv->ToUtf16BEFromUtf8(chinese_text, out_param);
    EXPECT_EQ(ret_val, out_param);
}

TEST_F(RobustnessTest, Regression_Consistency_Utf32LE_ReturnVsOutputParam) {
    std::u32string ret_val = conv->ToUtf32LEFromUtf8(chinese_text);
    std::u32string out_param;
    conv->ToUtf32LEFromUtf8(chinese_text, out_param);
    EXPECT_EQ(ret_val, out_param);
}

TEST_F(RobustnessTest, Regression_Consistency_Utf16LEFromLocale_ReturnVsOutputParam) {
    std::u16string ret_val = conv->ToUtf16LEFromLocale(chinese_text);
    std::u16string out_param;
    conv->ToUtf16LEFromLocale(chinese_text, out_param);
    EXPECT_EQ(ret_val, out_param);
}

TEST_F(RobustnessTest, Regression_Consistency_Utf16BEFromLocale_ReturnVsOutputParam) {
    std::u16string ret_val = conv->ToUtf16BEFromLocale(chinese_text);
    std::u16string out_param;
    conv->ToUtf16BEFromLocale(chinese_text, out_param);
    EXPECT_EQ(ret_val, out_param);
}

TEST_F(RobustnessTest, Regression_Consistency_WideStringFromLocale_ReturnVsOutputParam) {
    std::wstring ret_val = conv->ToWideStringFromLocale(chinese_text);
    std::wstring out_param;
    conv->ToWideStringFromLocale(chinese_text, out_param);
    EXPECT_EQ(ret_val, out_param);
}

TEST_F(RobustnessTest, Regression_Consistency_Utf16LEBEInterconvert) {
    std::u16string le = conv->ToUtf16LEFromUtf8(chinese_text);

    std::u16string be_ret = conv->ToUtf16BEFromUtf16LE(le);
    std::u16string be_out;
    conv->ToUtf16BEFromUtf16LE(le, be_out);
    EXPECT_EQ(be_ret, be_out);

    std::u16string le_ret = conv->ToUtf16LEFromUtf16BE(be_ret);
    std::u16string le_out;
    conv->ToUtf16LEFromUtf16BE(be_ret, le_out);
    EXPECT_EQ(le_ret, le_out);
    EXPECT_EQ(le_ret, le);
}

TEST_F(RobustnessTest, Regression_Consistency_Utf32LEFromUtf16LE) {
    std::u16string u16 = conv->ToUtf16LEFromUtf8(chinese_text);
    std::u32string ret_val = conv->ToUtf32LEFromUtf16LE(u16);
    std::u32string out_param;
    conv->ToUtf32LEFromUtf16LE(u16, out_param);
    EXPECT_EQ(ret_val, out_param);
}

TEST_F(RobustnessTest, Regression_Consistency_Utf32LEFromUtf16BE) {
    std::u16string be = conv->ToUtf16BEFromUtf8(chinese_text);
    std::u32string ret_val = conv->ToUtf32LEFromUtf16BE(be);
    std::u32string out_param;
    conv->ToUtf32LEFromUtf16BE(be, out_param);
    EXPECT_EQ(ret_val, out_param);
}
