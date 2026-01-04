/**
 * @file stage1_demo.cpp
 * @brief UniConv Stage 1 高性能特性演示
 * @details 展示 StringBufferPool、CompactResult 特化和高性能转换方法
 */

#include "../include/UniConv.h"
#include <chrono>
#include <iostream>
#include <vector>
#include <iomanip>

using namespace std;
using namespace std::chrono;

// 性能测试辅助函数
template<typename Func>
auto Benchmark(const string& name, Func&& func, int iterations = 1000) {
    cout << "Test: " << name << " (" << iterations << " iterations)" << endl;
    
    auto start = high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        func();
    }
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(end - start);
    
    cout << "Total time: " << duration.count() << " us" << endl;
    cout << "Average: " << (duration.count() / iterations) << " us/op" << endl;
    cout << "Throughput: " << (iterations * 1000000.0 / duration.count()) << " ops/s\n" << endl;
    
    return duration;
}

int main() {
    cout << "==============================================================\n";
    cout << "=   UniConv Stage 1 Performance Demo                         =\n";
    cout << "=   StringBufferPool + CompactResult                         =\n";
    cout << "==============================================================\n\n";
    
    auto converter = UniConv::Create();
    
    // 测试数据
    const string test_utf8 = "Hello World! Test string with special chars";
    const vector<string> batch_inputs = {
        "Test String 1",
        "Test String 2 with some content",
        "Test String 3",
        "Short text",
        "Very long text with lots of characters for performance testing",
        "Performance testing data",
        "Testing performance with mixed content"
    };
    
    cout << "Test data:\n";
    cout << "  UTF-8 string length: " << test_utf8.size() << " bytes\n";
    cout << "  Batch array: " << batch_inputs.size() << " strings\n" << endl;
    
    // CompactResult 特化测试
    cout << "=== CompactResult<std::string> Specialization ===" << endl;
    
    // 预留容量构造
    auto result_with_capacity = StringResult::WithReservedCapacity(1024);
    if (result_with_capacity.IsSuccess()) {
        cout << "Reserved capacity construction: " << result_with_capacity.GetCapacity() << " bytes" << endl;
    }
    
    // Emplace 构造
    auto emplaced_result = StringResult::EmplaceSuccess("Hello", " ", "World!");
    if (emplaced_result.IsSuccess()) {
        cout << "Emplace construction: \"" << emplaced_result.GetValue() << "\"" << endl;
    }
    
    // C 字符串构造
    auto c_string_result = StringResult::FromCString(test_utf8.c_str(), test_utf8.size());
    if (c_string_result.IsSuccess()) {
        cout << "C string construction: " << c_string_result.GetSize() << " bytes" << endl;
        cout << "SSO optimized: " << (c_string_result.IsSmallString() ? "Yes" : "No") << endl;
    }
    cout << endl;
    
    // StringBufferPool 测试
    cout << "=== StringBufferPool Test ===" << endl;
    auto initial_stats = converter->GetPoolStatistics();
    cout << "Initial: active_buffers=" << initial_stats.active_buffers 
         << ", total_conversions=" << initial_stats.total_conversions << "\n" << endl;
    
    // 高性能方法测试
    cout << "=== High-Performance Conversion Methods ===" << endl;
    
    // 1. ConvertEncodingFast
    cout << "1. ConvertEncodingFast:" << endl;
    auto fast_result = converter->ConvertEncodingFast(test_utf8, "UTF-8", "UTF-16LE");
    if (fast_result.IsSuccess()) {
        cout << "Success UTF-8 -> UTF-16LE\n";
        cout << "  Input: " << test_utf8.size() << " bytes\n";
        cout << "  Output: " << fast_result.GetSize() << " bytes\n" << endl;
    }
    
    // 2. ConvertEncodingFastWithHint
    cout << "2. ConvertEncodingFastWithHint:" << endl;
    auto hint_result = converter->ConvertEncodingFastWithHint(test_utf8, "UTF-8", "UTF-16LE", 1024);
    if (hint_result.IsSuccess()) {
        cout << "Success UTF-8 -> UTF-16LE (hint: 1024)\n";
        cout << "  Output: " << hint_result.GetSize() << " bytes\n";
        cout << "  Pre-allocated capacity: " << hint_result.GetCapacity() << " bytes\n";
        
        // 测试系统估算
        cout << "  [Debug] System estimate for 43 bytes UTF-8->UTF-16LE would be: ~" << (43 * 1.8 * 1.1) << " bytes" << endl;
        cout << "  [Debug] Expected final_estimate should be: min(1024, max(170, 512)) = " << (std::min)(1024, (std::max)(170, 512)) << endl;
        cout << endl;
    }
    
    // 3. ConvertEncodingBatch
    cout << "3. ConvertEncodingBatch:" << endl;
    auto batch_results = converter->ConvertEncodingBatch(batch_inputs, "UTF-8", "UTF-16LE");
    cout << "  Batch conversion results:\n";
    for (size_t i = 0; i < batch_results.size(); ++i) {
        if (batch_results[i].IsSuccess()) {
            cout << "  Success [" << i << "] " << batch_inputs[i].size() 
                 << " -> " << batch_results[i].GetSize() << " bytes" << endl;
        }
    }
    cout << endl;
    
    // 性能基准测试
    cout << "=== Performance Benchmark ===" << endl;
    
    // 传统方法
    Benchmark("Traditional ConvertEncoding", [&]() {
        auto result = converter->ConvertEncoding(test_utf8, "UTF-8", "UTF-16LE");
    }, 1000);
    
    // 高性能方法
    Benchmark("High-Performance ConvertEncodingFast", [&]() {
        auto result = converter->ConvertEncodingFast(test_utf8, "UTF-8", "UTF-16LE");
    }, 1000);
    
    // 带提示的优化方法
    Benchmark("Optimized ConvertEncodingFastWithHint", [&]() {
        auto result = converter->ConvertEncodingFastWithHint(test_utf8, "UTF-8", "UTF-16LE", 200);
    }, 1000);
    
    // 批量转换
    Benchmark("Batch ConvertEncodingBatch", [&]() {
        auto results = converter->ConvertEncodingBatch(batch_inputs, "UTF-8", "UTF-16LE");
    }, 100);
    
    // 最终统计
    cout << "=== Final Statistics ===" << endl;
    auto final_stats = converter->GetPoolStatistics();
    cout << "Total conversions: " << final_stats.total_conversions << endl;
    cout << "Cache hits: " << final_stats.cache_hits << endl;
    cout << "Cache hit rate: " << fixed << setprecision(2) << (final_stats.hit_rate * 100) << "%" << endl;
    cout << "Active buffers: " << final_stats.active_buffers << "\n" << endl;
    
    cout << "Stage 1 demo completed!" << endl;
    cout << "Key optimization points:\n";
    cout << "  • StringBufferPool: Reduces memory allocation overhead\n";
    cout << "  • CompactResult specialization: Optimizes string operations\n";
    cout << "  • Smart size estimation: Reduces buffer expansion\n";
    cout << "  • Batch processing: Improves large-scale efficiency\n" << endl;
    
    return 0;
}
