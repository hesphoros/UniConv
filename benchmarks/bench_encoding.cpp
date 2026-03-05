/**
 * @file bench_encoding.cpp
 * @brief UniConv 性能基准测试
 *
 * 测试维度:
 *   - 不同编码对的吞吐量 (MB/s)
 *   - 不同数据量级的扩展性 (64B → 1MB)
 *   - API 风格对比 (返回值 vs 输出参数 vs Ex)
 *   - 批处理 vs 并行批处理 vs 逐条处理
 *   - 不同文本类型的影响 (ASCII vs CJK vs Emoji vs Mixed)
 */

#ifdef _WIN32
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#endif

#include <benchmark/benchmark.h>
#include <UniConv/UniConv.h>
#include <memory>
#include <string>
#include <vector>
#include <algorithm>

// ============================================================================
// 测试数据生成
// ============================================================================
namespace {

std::string GenerateAscii(size_t target_bytes) {
    const std::string base = "The quick brown fox jumps over the lazy dog. 0123456789 ";
    std::string result;
    result.reserve(target_bytes);
    while (result.size() < target_bytes) {
        result.append(base, 0, std::min(base.size(), target_bytes - result.size()));
    }
    return result;
}

std::string GenerateChinese(size_t target_bytes) {
    // 每个中文字符占 3 字节 UTF-8
    const std::string base = "\xe4\xb8\x96\xe7\x95\x8c\xe4\xbd\xa0\xe5\xa5\xbd"
                             "\xe5\xa4\xa9\xe4\xb8\x8b\xe5\xa4\xa7\xe5\x90\x8c"
                             "\xe6\x98\xa5\xe9\xa3\x8e\xe5\x8c\x96\xe9\x9b\xa8"; // 世界你好天下大同春风化雨
    std::string result;
    result.reserve(target_bytes);
    while (result.size() < target_bytes) {
        result.append(base, 0, std::min(base.size(), target_bytes - result.size()));
    }
    // 确保不截断多字节序列
    while (!result.empty() && (static_cast<unsigned char>(result.back()) & 0xC0) == 0x80) {
        result.pop_back();
    }
    if (!result.empty() && static_cast<unsigned char>(result.back()) >= 0xC0) {
        result.pop_back();
    }
    return result;
}

std::string GenerateEmoji(size_t target_bytes) {
    // 每个 emoji 占 4 字节 UTF-8
    const std::string base = "\xf0\x9f\x98\x80\xf0\x9f\x8c\x8d\xf0\x9f\x9a\x80"
                             "\xf0\x9f\x8e\x89\xf0\x9f\x92\xbb\xf0\x9f\x94\xa5"
                             "\xf0\x9f\x8c\x88\xf0\x9f\x8d\x95\xf0\x9f\x8e\xb5"; // 😀🌍🚀🎉💻🔥🌈🍕🎵
    std::string result;
    result.reserve(target_bytes);
    while (result.size() < target_bytes) {
        result.append(base, 0, std::min(base.size(), target_bytes - result.size()));
    }
    while (!result.empty() && (static_cast<unsigned char>(result.back()) & 0xC0) == 0x80) {
        result.pop_back();
    }
    if (!result.empty() && static_cast<unsigned char>(result.back()) >= 0xC0) {
        result.pop_back();
    }
    return result;
}

std::string GenerateMixed(size_t target_bytes) {
    const std::string base = "Hello\xe4\xb8\x96\xe7\x95\x8c! \xf0\x9f\x98\x80 Test123 "
                             "\xe5\xa4\xa9\xe4\xb8\x8b OK \xf0\x9f\x9a\x80 ";
    std::string result;
    result.reserve(target_bytes);
    while (result.size() < target_bytes) {
        result.append(base, 0, std::min(base.size(), target_bytes - result.size()));
    }
    while (!result.empty() && (static_cast<unsigned char>(result.back()) & 0xC0) == 0x80) {
        result.pop_back();
    }
    if (!result.empty() && static_cast<unsigned char>(result.back()) >= 0xC0) {
        result.pop_back();
    }
    return result;
}

} // namespace

// ============================================================================
// 1. 核心转换吞吐量: 不同编码对 × 不同数据量
// ============================================================================

static void BM_Utf8ToUtf16LE(benchmark::State& state) {
    auto conv = UniConv::Create();
    std::string input = GenerateChinese(static_cast<size_t>(state.range(0)));
    for (auto _ : state) {
        auto result = conv->ToUtf16LEFromUtf8(input);
        benchmark::DoNotOptimize(result);
    }
    state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * static_cast<int64_t>(input.size()));
    state.SetLabel(std::to_string(input.size()) + "B");
}
BENCHMARK(BM_Utf8ToUtf16LE)->RangeMultiplier(4)->Range(64, 1 << 20);

static void BM_Utf16LEToUtf8(benchmark::State& state) {
    auto conv = UniConv::Create();
    std::string src = GenerateChinese(static_cast<size_t>(state.range(0)));
    std::u16string input = conv->ToUtf16LEFromUtf8(src);
    for (auto _ : state) {
        auto result = conv->ToUtf8FromUtf16LE(input);
        benchmark::DoNotOptimize(result);
    }
    state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * static_cast<int64_t>(input.size()) * 2);
    state.SetLabel(std::to_string(input.size() * 2) + "B");
}
BENCHMARK(BM_Utf16LEToUtf8)->RangeMultiplier(4)->Range(64, 1 << 20);

static void BM_Utf8ToUtf16BE(benchmark::State& state) {
    auto conv = UniConv::Create();
    std::string input = GenerateChinese(static_cast<size_t>(state.range(0)));
    for (auto _ : state) {
        auto result = conv->ToUtf16BEFromUtf8(input);
        benchmark::DoNotOptimize(result);
    }
    state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * static_cast<int64_t>(input.size()));
    state.SetLabel(std::to_string(input.size()) + "B");
}
BENCHMARK(BM_Utf8ToUtf16BE)->RangeMultiplier(4)->Range(64, 1 << 20);

static void BM_Utf8ToUtf32LE(benchmark::State& state) {
    auto conv = UniConv::Create();
    std::string input = GenerateChinese(static_cast<size_t>(state.range(0)));
    for (auto _ : state) {
        auto result = conv->ToUtf32LEFromUtf8(input);
        benchmark::DoNotOptimize(result);
    }
    state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * static_cast<int64_t>(input.size()));
    state.SetLabel(std::to_string(input.size()) + "B");
}
BENCHMARK(BM_Utf8ToUtf32LE)->RangeMultiplier(4)->Range(64, 1 << 20);

static void BM_Utf8ToGBK(benchmark::State& state) {
    auto conv = UniConv::Create();
    std::string input = GenerateChinese(static_cast<size_t>(state.range(0)));
    for (auto _ : state) {
        auto result = conv->ConvertEncodingFast(input, "UTF-8", "GBK");
        benchmark::DoNotOptimize(result);
    }
    state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * static_cast<int64_t>(input.size()));
    state.SetLabel(std::to_string(input.size()) + "B");
}
BENCHMARK(BM_Utf8ToGBK)->RangeMultiplier(4)->Range(64, 1 << 20);

static void BM_Utf16LEToUtf16BE(benchmark::State& state) {
    auto conv = UniConv::Create();
    std::string src = GenerateChinese(static_cast<size_t>(state.range(0)));
    std::u16string input = conv->ToUtf16LEFromUtf8(src);
    for (auto _ : state) {
        auto result = conv->ToUtf16BEFromUtf16LE(input);
        benchmark::DoNotOptimize(result);
    }
    state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * static_cast<int64_t>(input.size()) * 2);
    state.SetLabel(std::to_string(input.size() * 2) + "B");
}
BENCHMARK(BM_Utf16LEToUtf16BE)->RangeMultiplier(4)->Range(64, 1 << 20);

// ============================================================================
// 2. 文本类型对性能的影响
// ============================================================================

static void BM_TextType_ASCII(benchmark::State& state) {
    auto conv = UniConv::Create();
    std::string input = GenerateAscii(4096);
    for (auto _ : state) {
        auto result = conv->ToUtf16LEFromUtf8(input);
        benchmark::DoNotOptimize(result);
    }
    state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * static_cast<int64_t>(input.size()));
}
BENCHMARK(BM_TextType_ASCII);

static void BM_TextType_CJK(benchmark::State& state) {
    auto conv = UniConv::Create();
    std::string input = GenerateChinese(4096);
    for (auto _ : state) {
        auto result = conv->ToUtf16LEFromUtf8(input);
        benchmark::DoNotOptimize(result);
    }
    state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * static_cast<int64_t>(input.size()));
}
BENCHMARK(BM_TextType_CJK);

static void BM_TextType_Emoji(benchmark::State& state) {
    auto conv = UniConv::Create();
    std::string input = GenerateEmoji(4096);
    for (auto _ : state) {
        auto result = conv->ToUtf16LEFromUtf8(input);
        benchmark::DoNotOptimize(result);
    }
    state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * static_cast<int64_t>(input.size()));
}
BENCHMARK(BM_TextType_Emoji);

static void BM_TextType_Mixed(benchmark::State& state) {
    auto conv = UniConv::Create();
    std::string input = GenerateMixed(4096);
    for (auto _ : state) {
        auto result = conv->ToUtf16LEFromUtf8(input);
        benchmark::DoNotOptimize(result);
    }
    state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * static_cast<int64_t>(input.size()));
}
BENCHMARK(BM_TextType_Mixed);

// ============================================================================
// 3. API 风格对比: 返回值 vs 输出参数 vs CompactResult(Ex)
// ============================================================================

static void BM_ApiStyle_ReturnValue(benchmark::State& state) {
    auto conv = UniConv::Create();
    std::string input = GenerateChinese(4096);
    for (auto _ : state) {
        std::u16string result = conv->ToUtf16LEFromUtf8(input);
        benchmark::DoNotOptimize(result);
    }
    state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * static_cast<int64_t>(input.size()));
}
BENCHMARK(BM_ApiStyle_ReturnValue);

static void BM_ApiStyle_OutputParam(benchmark::State& state) {
    auto conv = UniConv::Create();
    std::string input = GenerateChinese(4096);
    std::u16string output;
    for (auto _ : state) {
        conv->ToUtf16LEFromUtf8(input, output);
        benchmark::DoNotOptimize(output);
    }
    state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * static_cast<int64_t>(input.size()));
}
BENCHMARK(BM_ApiStyle_OutputParam);

static void BM_ApiStyle_CompactResult(benchmark::State& state) {
    auto conv = UniConv::Create();
    std::string input = GenerateChinese(4096);
    for (auto _ : state) {
        auto result = conv->ConvertEncodingFast(input, "UTF-8", "UTF-16LE");
        benchmark::DoNotOptimize(result);
    }
    state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * static_cast<int64_t>(input.size()));
}
BENCHMARK(BM_ApiStyle_CompactResult);

static void BM_ApiStyle_OutputParam_ErrorCode(benchmark::State& state) {
    auto conv = UniConv::Create();
    std::string input = GenerateChinese(4096);
    std::string output;
    for (auto _ : state) {
        conv->ConvertEncodingFast(input, "UTF-8", "UTF-16LE", output);
        benchmark::DoNotOptimize(output);
    }
    state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * static_cast<int64_t>(input.size()));
}
BENCHMARK(BM_ApiStyle_OutputParam_ErrorCode);

// ============================================================================
// 4. 缓冲区复用效果: 输出参数预分配 vs 每次新建
// ============================================================================

static void BM_BufferReuse_NoReuse(benchmark::State& state) {
    auto conv = UniConv::Create();
    std::string input = GenerateChinese(4096);
    for (auto _ : state) {
        std::string output;
        conv->ConvertEncodingFast(input, "UTF-8", "UTF-16LE", output);
        benchmark::DoNotOptimize(output);
    }
    state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * static_cast<int64_t>(input.size()));
}
BENCHMARK(BM_BufferReuse_NoReuse);

static void BM_BufferReuse_WithReuse(benchmark::State& state) {
    auto conv = UniConv::Create();
    std::string input = GenerateChinese(4096);
    std::string output;
    output.reserve(input.size() * 2);
    for (auto _ : state) {
        conv->ConvertEncodingFast(input, "UTF-8", "UTF-16LE", output);
        benchmark::DoNotOptimize(output);
    }
    state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * static_cast<int64_t>(input.size()));
}
BENCHMARK(BM_BufferReuse_WithReuse);

// ============================================================================
// 5. 批处理: 串行 vs 并行 vs 逐条
// ============================================================================

static void BM_Batch_OneByOne(benchmark::State& state) {
    auto conv = UniConv::Create();
    const int count = static_cast<int>(state.range(0));
    std::vector<std::string> inputs(count, GenerateChinese(256));
    int64_t total_bytes = 0;
    for (const auto& s : inputs) total_bytes += static_cast<int64_t>(s.size());

    for (auto _ : state) {
        for (const auto& input : inputs) {
            auto result = conv->ConvertEncodingFast(input, "UTF-8", "UTF-16LE");
            benchmark::DoNotOptimize(result);
        }
    }
    state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * total_bytes);
    state.SetLabel(std::to_string(count) + " items");
}
BENCHMARK(BM_Batch_OneByOne)->RangeMultiplier(4)->Range(16, 4096);

static void BM_Batch_Serial(benchmark::State& state) {
    auto conv = UniConv::Create();
    const int count = static_cast<int>(state.range(0));
    std::vector<std::string> inputs(count, GenerateChinese(256));
    int64_t total_bytes = 0;
    for (const auto& s : inputs) total_bytes += static_cast<int64_t>(s.size());

    for (auto _ : state) {
        auto results = conv->ConvertEncodingBatch(inputs, "UTF-8", "UTF-16LE");
        benchmark::DoNotOptimize(results);
    }
    state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * total_bytes);
    state.SetLabel(std::to_string(count) + " items");
}
BENCHMARK(BM_Batch_Serial)->RangeMultiplier(4)->Range(16, 4096);

static void BM_Batch_Parallel(benchmark::State& state) {
    auto conv = UniConv::Create();
    const int count = static_cast<int>(state.range(0));
    std::vector<std::string> inputs(count, GenerateChinese(256));
    int64_t total_bytes = 0;
    for (const auto& s : inputs) total_bytes += static_cast<int64_t>(s.size());

    for (auto _ : state) {
        auto results = conv->ConvertEncodingBatchParallel(inputs, "UTF-8", "UTF-16LE");
        benchmark::DoNotOptimize(results);
    }
    state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * total_bytes);
    state.SetLabel(std::to_string(count) + " items");
}
BENCHMARK(BM_Batch_Parallel)->RangeMultiplier(4)->Range(16, 4096);

// ============================================================================
// 6. 往返转换 (Round-trip) 开销
// ============================================================================

static void BM_RoundTrip_Utf8_Utf16LE(benchmark::State& state) {
    auto conv = UniConv::Create();
    std::string input = GenerateChinese(4096);
    for (auto _ : state) {
        std::u16string u16 = conv->ToUtf16LEFromUtf8(input);
        std::string back = conv->ToUtf8FromUtf16LE(u16);
        benchmark::DoNotOptimize(back);
    }
    state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * static_cast<int64_t>(input.size()) * 2);
}
BENCHMARK(BM_RoundTrip_Utf8_Utf16LE);

static void BM_RoundTrip_Utf8_GBK(benchmark::State& state) {
    auto conv = UniConv::Create();
    std::string input = GenerateChinese(4096);
    for (auto _ : state) {
        auto gbk = conv->ConvertEncodingFast(input, "UTF-8", "GBK");
        if (gbk.IsSuccess()) {
            auto back = conv->ConvertEncodingFast(gbk.GetValue(), "GBK", "UTF-8");
            benchmark::DoNotOptimize(back);
        }
    }
    state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * static_cast<int64_t>(input.size()) * 2);
}
BENCHMARK(BM_RoundTrip_Utf8_GBK);

static void BM_RoundTrip_Chain_Utf8_16LE_16BE_32LE_Utf8(benchmark::State& state) {
    auto conv = UniConv::Create();
    std::string input = GenerateChinese(4096);
    for (auto _ : state) {
        std::u16string u16le = conv->ToUtf16LEFromUtf8(input);
        std::u16string u16be = conv->ToUtf16BEFromUtf16LE(u16le);
        std::u32string u32   = conv->ToUtf32LEFromUtf16BE(u16be);
        std::string    back  = conv->ToUtf8FromUtf32LE(u32);
        benchmark::DoNotOptimize(back);
    }
    state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * static_cast<int64_t>(input.size()));
}
BENCHMARK(BM_RoundTrip_Chain_Utf8_16LE_16BE_32LE_Utf8);

// ============================================================================
// 7. 实例创建开销
// ============================================================================

static void BM_CreateInstance(benchmark::State& state) {
    for (auto _ : state) {
        auto conv = UniConv::Create();
        benchmark::DoNotOptimize(conv);
    }
}
BENCHMARK(BM_CreateInstance);

static void BM_ThreadLocalInstance(benchmark::State& state) {
    for (auto _ : state) {
        auto& conv = UniConv::ThreadLocal();
        benchmark::DoNotOptimize(&conv);
    }
}
BENCHMARK(BM_ThreadLocalInstance);

// ============================================================================
// 8. string_view 零拷贝 vs std::string 拷贝
// ============================================================================

static void BM_StringViewZeroCopy(benchmark::State& state) {
    auto& conv = UniConv::ThreadLocal();
    std::string input = GenerateChinese(static_cast<size_t>(state.range(0)));
    std::string_view sv(input);
    std::string output;
    for (auto _ : state) {
        conv.ConvertEncodingFast(sv, "UTF-8", "UTF-16LE", output);
        benchmark::DoNotOptimize(output);
    }
    state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * static_cast<int64_t>(input.size()));
}
BENCHMARK(BM_StringViewZeroCopy)->RangeMultiplier(16)->Range(64, 1 << 20);

static void BM_StringCopy(benchmark::State& state) {
    auto& conv = UniConv::ThreadLocal();
    std::string input = GenerateChinese(static_cast<size_t>(state.range(0)));
    std::string output;
    for (auto _ : state) {
        conv.ConvertEncodingFast(input, "UTF-8", "UTF-16LE", output);
        benchmark::DoNotOptimize(output);
    }
    state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * static_cast<int64_t>(input.size()));
}
BENCHMARK(BM_StringCopy)->RangeMultiplier(16)->Range(64, 1 << 20);
