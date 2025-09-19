/**
 * @file stage1_demo.cpp
 * @brief Stage 1 High-Performance StringBufferPool and CompactResult Demo
 * @details Demonstrates StringBufferPool, optimized CompactResult specialization and high-performance conversion methods
 */

#include "../include/UniConv.h"
#include <chrono>
#include <iostream>
#include <vector>
#include <iomanip>

using namespace std;
using namespace std::chrono;

// Performance testing helper function
template<typename Func>
auto benchmark(const std::string& name, Func&& func, int iterations = 1000) {
    cout << "Starting test: " << name << " (" << iterations << " iterations)" << endl;
    
    auto start = high_resolution_clock::now();
    
    for (int i = 0; i < iterations; ++i) {
        func();
    }
    
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(end - start);
    
    cout << "Total time: " << duration.count() << " microseconds" << endl;
    cout << "Average time: " << (duration.count() / iterations) << " microseconds/op" << endl;
    cout << "Throughput: " << (iterations * 1000000.0 / duration.count()) << " ops/s" << endl;
    cout << endl;
    
    return duration;
}

int main() {
    cout << "=====================================" << endl;
    cout << "UniConv Stage 1 Performance Demo" << endl;
    cout << "StringBufferPool + CompactResult" << endl;
    cout << "=====================================" << endl << endl;
    
    auto uniconv = UniConv::GetInstance();
    
    // Test data
    const std::string test_utf8 = "Hello World! This is a test string with special chars: abcdefghijk";
    const std::vector<std::string> batch_inputs = {
        "Test String 1",
        "Test String 2 with some content",
        "Test String 3",
        "Short text",
        "Very long text with lots of characters for performance testing purpose including various data patterns",
        "Performance testing data mixed content",
        "Testing performance with mixed content ASCII and special chars"
    };
    
    cout << "Test data:" << endl;
    cout << "   - UTF-8 string length: " << test_utf8.size() << " bytes" << endl;
    cout << "   - Batch test array: " << batch_inputs.size() << " strings" << endl;
    cout << "   - Total characters: " << [&](){
        size_t total = 0;
        for(const auto& s : batch_inputs) total += s.size();
        return total;
    }() << " bytes" << endl << endl;
    
    // ========== CompactResult specialization test ==========
    cout << "CompactResult<std::string> specialization test:" << endl;
    
    // Test reserved capacity construction
    auto result_with_capacity = StringResult::WithReservedCapacity(1024);
    if (result_with_capacity.IsSuccess()) {
        cout << "Reserved capacity construction success, capacity: " << result_with_capacity.GetCapacity() << " bytes" << endl;
    }
    
    // Test emplace construction
    auto emplaced_result = StringResult::EmplaceSuccess("Hello", " ", "World!");
    if (emplaced_result.IsSuccess()) {
        cout << "Emplace construction success: \"" << emplaced_result.GetValue() << "\"" << endl;
    }
    
    // Test C string construction
    auto c_string_result = StringResult::FromCString(test_utf8.c_str(), test_utf8.size());
    if (c_string_result.IsSuccess()) {
        cout << "C string construction success, length: " << c_string_result.GetSize() << " bytes" << endl;
        cout << "   SSO optimization: " << (c_string_result.IsSmallString() ? "Yes" : "No") << endl;
    }
    cout << endl;
    
    // ========== StringBufferPool test ==========
    cout << "StringBufferPool functionality test:" << endl;
    
    // Get initial statistics
    auto initial_stats = uniconv->GetPoolStatistics();
    cout << "Initial stats: active_buffers=" << initial_stats.active_buffers << ", total_conversions=" << initial_stats.total_conversions << endl;
    
    // ========== High-performance methods test ==========
    cout << endl << "High-performance conversion methods test:" << endl;
    
    // 1. ConvertEncodingFast test
    cout << "1. ConvertEncodingFast test:" << endl;
    auto fast_result = uniconv->ConvertEncodingFast(test_utf8, "UTF-8", "UTF-16LE");
    if (fast_result.IsSuccess()) {
        cout << "Success UTF-8 -> UTF-16LE conversion" << endl;
        cout << "   Input: " << test_utf8.size() << " bytes" << endl;
        cout << "   Output: " << fast_result.GetSize() << " bytes" << endl;
    } else {
        cout << "Failed UTF-8 -> UTF-16LE conversion: " << fast_result.GetErrorMessage() << endl;
    }
    
    // 2. ConvertEncodingFastWithHint test
    cout << endl << "2. ConvertEncodingFastWithHint test:" << endl;
    auto hint_result = uniconv->ConvertEncodingFastWithHint(test_utf8, "UTF-8", "UTF-16LE", 1024);
    if (hint_result.IsSuccess()) {
        cout << "Success UTF-8 -> UTF-16LE conversion (hint: 1024 bytes)" << endl;
        cout << "   Input: " << test_utf8.size() << " bytes" << endl;
        cout << "   Output: " << hint_result.GetSize() << " bytes" << endl;
        cout << "   Pre-allocated capacity: " << hint_result.GetCapacity() << " bytes" << endl;
    } else {
        cout << "Failed UTF-8 -> UTF-16LE conversion: " << hint_result.GetErrorMessage() << endl;
    }
    
    // 3. ConvertEncodingBatch batch test
    cout << endl << "3. ConvertEncodingBatch batch test:" << endl;
    auto batch_results = uniconv->ConvertEncodingBatch(batch_inputs, "UTF-8", "UTF-16LE");
    
    cout << "   Batch conversion results:" << endl;
    for (size_t i = 0; i < batch_results.size(); ++i) {
        if (batch_results[i].IsSuccess()) {
            cout << "   Success [" << i << "] " << batch_inputs[i].size() << " -> " << batch_results[i].GetSize() << " bytes" << endl;
        } else {
            cout << "   Failed [" << i << "] error: " << batch_results[i].GetErrorMessage() << endl;
        }
    }
    
    // ========== Performance benchmark ==========
    cout << endl << "Performance benchmark test:" << endl;
    
    // Traditional method performance test
    benchmark("Traditional ConvertEncoding", [&]() {
        auto result = uniconv->ConvertEncoding(test_utf8, "UTF-8", "UTF-16LE");
    }, 1000);
    
    // High-performance method test
    benchmark("High-Performance ConvertEncodingFast", [&]() {
        auto result = uniconv->ConvertEncodingFast(test_utf8, "UTF-8", "UTF-16LE");
    }, 1000);
    
    // High-performance method with hint test
    benchmark("Optimized ConvertEncodingFastWithHint", [&]() {
        auto result = uniconv->ConvertEncodingFastWithHint(test_utf8, "UTF-8", "UTF-16LE", 200);
    }, 1000);
    
    // Batch conversion performance test
    benchmark("Batch ConvertEncodingBatch", [&]() {
        auto results = uniconv->ConvertEncodingBatch(batch_inputs, "UTF-8", "UTF-16LE");
    }, 100);  // Reduce iterations for batch test
    
    // ========== Final statistics ==========
    cout << "Final statistics:" << endl;
    auto final_stats = uniconv->GetPoolStatistics();
    cout << "   Total conversions: " << final_stats.total_conversions << endl;
    cout << "   Cache hits: " << final_stats.cache_hits << endl;
    cout << "   Cache hit rate: " << fixed << setprecision(2) << (final_stats.hit_rate * 100) << "%" << endl;
    cout << "   Active buffers: " << final_stats.active_buffers << endl;
    
    cout << endl << "Stage 1 demo completed!" << endl;
    cout << "Key optimization points:" << endl;
    cout << "   - StringBufferPool: Reduces memory allocation overhead" << endl;
    cout << "   - CompactResult specialization: Optimizes string operations" << endl;
    cout << "   - Smart size estimation: Reduces buffer expansion" << endl;
    cout << "   - Batch processing: Improves large-scale conversion efficiency" << endl;
    
    return 0;
}