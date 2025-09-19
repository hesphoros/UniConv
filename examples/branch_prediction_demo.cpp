/**
 * @file branch_prediction_demo.cpp
 * @brief 编译时优化和分支预测效果演示
 * @details 对比展示编译时优化前后的性能差异，包括分支预测、循环展开、缓存预取等优化技术
 */

#include "../include/UniConv.h"
#include <chrono>
#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <iomanip>
#include <numeric>  // for accumulate

using namespace std;
using namespace std::chrono;

// 性能测试辅助函数
template<typename Func>
auto benchmark_advanced(const std::string& name, Func&& func, int iterations = 10000) {
    cout << "Advanced Performance Test: " << name << endl;
    cout << "Iterations: " << iterations << endl;
    
    // 预热阶段，让CPU进入稳定状态
    for (int i = 0; i < iterations / 10; ++i) {
        func();
    }
    
    // 正式测试
    vector<double> times;
    times.reserve(10);
    
    for (int round = 0; round < 10; ++round) {
        auto start = high_resolution_clock::now();
        
        for (int i = 0; i < iterations / 10; ++i) {
            func();
        }
        
        auto end = high_resolution_clock::now();
        auto duration = duration_cast<nanoseconds>(end - start);
        times.push_back(duration.count() / (iterations / 10.0));
    }
    
    // 统计分析
    sort(times.begin(), times.end());
    double median = times[5];  // 中位数
    double min_time = times[0];
    double max_time = times[9];
    double avg_time = accumulate(times.begin(), times.end(), 0.0) / times.size();
    
    cout << "  Average: " << fixed << setprecision(2) << avg_time << " ns/op" << endl;
    cout << "  Median:  " << median << " ns/op" << endl;
    cout << "  Min:     " << min_time << " ns/op" << endl;
    cout << "  Max:     " << max_time << " ns/op" << endl;
    cout << "  Std Dev: " << fixed << setprecision(2) << 
             sqrt(accumulate(times.begin(), times.end(), 0.0, 
                  [avg_time](double acc, double t) { 
                      return acc + (t - avg_time) * (t - avg_time); 
                  }) / times.size()) << " ns" << endl;
    cout << "  Throughput: " << fixed << setprecision(0) << (1e9 / avg_time) << " ops/s" << endl;
    cout << endl;
    
    return avg_time;
}

// 生成测试数据的辅助函数
vector<string> generate_test_data(size_t count, size_t min_size = 10, size_t max_size = 1000) {
    vector<string> data;
    data.reserve(count);
    
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> size_dist(static_cast<int>(min_size), static_cast<int>(max_size));
    uniform_int_distribution<int> char_dist(32, 126);  // ASCII可打印字符
    
    for (size_t i = 0; i < count; ++i) {
        int str_size = size_dist(gen);
        string str;
        str.reserve(static_cast<size_t>(str_size));
        
        for (int j = 0; j < str_size; ++j) {
            str.push_back(static_cast<char>(char_dist(gen)));
        }
        
        data.push_back(move(str));
    }
    
    return data;
}

int main() {
    cout << "============================================================" << endl;
    cout << "UniConv 编译时优化和分支预测效果演示" << endl;
    cout << "Branch Prediction & Compile-Time Optimization Demo" << endl;
    cout << "============================================================" << endl << endl;
    
    auto uniconv = UniConv::GetInstance();
    
    // 生成不同类型的测试数据
    cout << "Generating test data..." << endl;
    auto small_strings = generate_test_data(5000, 10, 50);      // 小字符串
    auto medium_strings = generate_test_data(1000, 100, 500);   // 中等字符串
    auto large_strings = generate_test_data(100, 1000, 5000);   // 大字符串
    auto mixed_strings = generate_test_data(2000, 1, 2000);     // 混合大小
    
    cout << "Generated:" << endl;
    cout << "  Small strings (10-50 chars): " << small_strings.size() << endl;
    cout << "  Medium strings (100-500 chars): " << medium_strings.size() << endl;
    cout << "  Large strings (1K-5K chars): " << large_strings.size() << endl;
    cout << "  Mixed strings (1-2K chars): " << mixed_strings.size() << endl << endl;
    
    // 1. 基础转换性能测试
    cout << "=== 1. Basic Conversion Performance ===" << endl;
    const string test_str = "Hello World! This is a performance test string with mixed content.";
    
    auto time1 = benchmark_advanced("ConvertEncodingFast (UTF-8->UTF-16LE)", [&]() {
        auto result = uniconv->ConvertEncodingFast(test_str, "UTF-8", "UTF-16LE");
    }, 20000);
    
    auto time2 = benchmark_advanced("ConvertEncodingFastWithHint (optimized)", [&]() {
        auto result = uniconv->ConvertEncodingFastWithHint(test_str, "UTF-8", "UTF-16LE", test_str.size() * 2);
    }, 20000);
    
    cout << "Hint optimization speedup: " << fixed << setprecision(2) << 
             (time1 / time2) << "x faster" << endl << endl;
    
    // 2. 批量处理优化测试
    cout << "=== 2. Batch Processing Optimization ===" << endl;
    
    auto time3 = benchmark_advanced("Individual conversions (small strings)", [&]() {
        for (const auto& str : small_strings) {
            if (!str.empty()) {
                auto result = uniconv->ConvertEncodingFast(str, "UTF-8", "UTF-16LE");
            }
        }
    }, 100);
    
    auto time4 = benchmark_advanced("Batch conversion (small strings)", [&]() {
        auto results = uniconv->ConvertEncodingBatch(small_strings, "UTF-8", "UTF-16LE");
    }, 100);
    
    cout << "Batch processing speedup: " << fixed << setprecision(2) << 
             (time3 / time4) << "x faster" << endl << endl;
    
    // 3. 缓存预取效果测试
    cout << "=== 3. Memory Access Pattern Optimization ===" << endl;
    
    // 随机访问模式（对缓存不友好）
    auto shuffled_medium = medium_strings;
    random_device rd;
    mt19937 g(rd());
    shuffle(shuffled_medium.begin(), shuffled_medium.end(), g);
    
    auto time5 = benchmark_advanced("Sequential access pattern", [&]() {
        for (const auto& str : medium_strings) {
            auto result = uniconv->ConvertEncodingFast(str, "UTF-8", "UTF-16LE");
        }
    }, 500);
    
    auto time6 = benchmark_advanced("Random access pattern", [&]() {
        for (const auto& str : shuffled_medium) {
            auto result = uniconv->ConvertEncodingFast(str, "UTF-8", "UTF-16LE");
        }
    }, 500);
    
    cout << "Cache friendliness impact: " << fixed << setprecision(2) << 
             (time6 / time5) << "x slower for random access" << endl << endl;
    
    // 4. 分支预测效果测试
    cout << "=== 4. Branch Prediction Effectiveness ===" << endl;
    
    // 创建一个包含很多空字符串的测试集（触发分支预测）
    vector<string> mostly_empty;
    mostly_empty.reserve(1000);
    for (int i = 0; i < 950; ++i) {
        mostly_empty.push_back("");  // 95%空字符串
    }
    for (int i = 0; i < 50; ++i) {
        mostly_empty.push_back("Non-empty string for testing");  // 5%非空字符串
    }
    
    // 打乱顺序，增加分支预测难度
    shuffle(mostly_empty.begin(), mostly_empty.end(), g);
    
    auto time7 = benchmark_advanced("Mostly empty strings (predictable)", [&]() {
        for (const auto& str : mostly_empty) {
            auto result = uniconv->ConvertEncodingFast(str, "UTF-8", "UTF-16LE");
        }
    }, 1000);
    
    // 创建50-50分布的测试集（分支预测最困难）
    vector<string> mixed_empty;
    mixed_empty.reserve(1000);
    for (int i = 0; i < 500; ++i) {
        mixed_empty.push_back(i % 2 == 0 ? "" : "Alternating pattern test string");
    }
    
    auto time8 = benchmark_advanced("50-50 empty/non-empty (unpredictable)", [&]() {
        for (const auto& str : mixed_empty) {
            auto result = uniconv->ConvertEncodingFast(str, "UTF-8", "UTF-16LE");
        }
    }, 1000);
    
    cout << "Branch prediction impact: " << fixed << setprecision(2) << 
             (time8 / time7) << "x slower for unpredictable branches" << endl << endl;
    
    // 5. 不同编码转换的性能对比
    cout << "=== 5. Different Encoding Performance ===" << endl;
    
    const string utf8_test = "Performance test with UTF-8 encoding and special chars: áéíóúñç";
    
    vector<pair<string, string>> encoding_pairs = {
        {"UTF-8", "UTF-16LE"},
        {"UTF-8", "UTF-16BE"},
        {"UTF-8", "UTF-32LE"},
        {"UTF-8", "ISO-8859-1"},
        {"UTF-8", "GBK"}
    };
    
    vector<double> encoding_times;
    for (const auto& [from, to] : encoding_pairs) {
        cout << "Testing " << from << " -> " << to << ":" << endl;
        auto time = benchmark_advanced(string("  ") + from + " to " + to, [&]() {
            auto result = uniconv->ConvertEncodingFast(utf8_test, from.c_str(), to.c_str());
        }, 15000);
        encoding_times.push_back(time);
    }
    
    // 找出最快的转换
    auto min_it = min_element(encoding_times.begin(), encoding_times.end());
    size_t fastest_idx = distance(encoding_times.begin(), min_it);
    
    cout << "Fastest conversion: " << encoding_pairs[fastest_idx].first << 
             " -> " << encoding_pairs[fastest_idx].second << endl;
    cout << "Relative performance:" << endl;
    for (size_t i = 0; i < encoding_times.size(); ++i) {
        cout << "  " << encoding_pairs[i].first << " -> " << encoding_pairs[i].second << 
                ": " << fixed << setprecision(2) << (encoding_times[i] / *min_it) << "x" << endl;
    }
    cout << endl;
    
    // 6. StringBufferPool 效果测试
    cout << "=== 6. StringBufferPool Effectiveness ===" << endl;
    
    auto pool_stats_before = uniconv->GetPoolStatistics();
    cout << "Pool statistics before intensive testing:" << endl;
    cout << "  Active buffers: " << pool_stats_before.active_buffers << endl;
    cout << "  Total conversions: " << pool_stats_before.total_conversions << endl;
    cout << "  Cache hits: " << pool_stats_before.cache_hits << endl;
    cout << "  Hit rate: " << fixed << setprecision(2) << (pool_stats_before.hit_rate * 100) << "%" << endl;
    
    // 进行大量转换以测试缓冲池效果
    auto time9 = benchmark_advanced("Intensive conversions (testing buffer pool)", [&]() {
        for (const auto& str : mixed_strings) {
            auto result = uniconv->ConvertEncodingFastWithHint(str, "UTF-8", "UTF-16LE", str.size() * 2);
        }
    }, 50);
    
    auto pool_stats_after = uniconv->GetPoolStatistics();
    cout << "Pool statistics after intensive testing:" << endl;
    cout << "  Active buffers: " << pool_stats_after.active_buffers << endl;
    cout << "  Total conversions: " << pool_stats_after.total_conversions << endl;
    cout << "  Cache hits: " << pool_stats_after.cache_hits << endl;
    cout << "  Hit rate: " << fixed << setprecision(2) << (pool_stats_after.hit_rate * 100) << "%" << endl;
    
    cout << "Conversions processed: " << (pool_stats_after.total_conversions - pool_stats_before.total_conversions) << endl;
    
    cout << endl << "=== Summary ===" << endl;
    cout << "Compilation optimizations applied:" << endl;
    cout << "  ✓ Branch prediction hints (LIKELY/UNLIKELY)" << endl;
    cout << "  ✓ Function attribute optimizations (HOT/COLD/PURE)" << endl;
    cout << "  ✓ Loop unrolling hints" << endl;
    cout << "  ✓ Memory prefetch instructions" << endl;
    cout << "  ✓ Cache-aligned data structures" << endl;
    cout << "  ✓ Constexpr compile-time computations" << endl;
    cout << "  ✓ Template specialization optimizations" << endl;
    
    cout << endl << "Performance improvements observed:" << endl;
    cout << "  • Hint-based allocation: ~" << fixed << setprecision(1) << (time1/time2) << "x speedup" << endl;
    cout << "  • Batch processing: ~" << fixed << setprecision(1) << (time3/time4) << "x speedup" << endl;
    cout << "  • Cache-friendly access: ~" << fixed << setprecision(1) << (time6/time5) << "x penalty for random access" << endl;
    cout << "  • Branch prediction: ~" << fixed << setprecision(1) << (time8/time7) << "x penalty for unpredictable branches" << endl;
    
    return 0;
}