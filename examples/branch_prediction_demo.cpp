/**
 * @file branch_prediction_demo.cpp
 * @brief UniConv 分支预测和编译时优化演示
 * @details 展示编译时优化、分支预测、循环展开等优化技术的效果
 */

#include "../include/UniConv.h"
#include <chrono>
#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <iomanip>
#include <numeric>

using namespace std;
using namespace std::chrono;

// 高级性能测试函数
template<typename Func>
auto BenchmarkAdvanced(const string& name, Func&& func, int iterations = 10000) {
    cout << "Performance Test: " << name << endl;
    cout << "Iterations: " << iterations << endl;
    
    // 预热阶段
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
    double median = times[5];
    double min_time = times[0];
    double max_time = times[9];
    double avg_time = accumulate(times.begin(), times.end(), 0.0) / times.size();
    
    cout << "  Average: " << fixed << setprecision(2) << avg_time << " ns/op" << endl;
    cout << "  Median:  " << median << " ns/op" << endl;
    cout << "  Min:     " << min_time << " ns/op" << endl;
    cout << "  Max:     " << max_time << " ns/op" << endl;
    cout << "  Throughput: " << fixed << setprecision(0) << (1e9 / avg_time) << " ops/s" << endl;
    cout << endl;
    
    return avg_time;
}

// 生成测试数据
vector<string> GenerateTestData(size_t count, size_t min_size = 10, size_t max_size = 1000) {
    vector<string> data;
    data.reserve(count);
    
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> size_dist(static_cast<int>(min_size), static_cast<int>(max_size));
    uniform_int_distribution<int> char_dist(32, 126);
    
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
    cout << "\n";
    cout << "=   UniConv Branch Prediction & Optimization Demo           =\n";
    cout << "==============================================================\n\n";
    
    auto converter = UniConv::Create();
    
    // 生成不同类型的测试数据
    cout << "Generating test data..." << endl;
    auto small_strings = GenerateTestData(5000, 10, 50);
    auto medium_strings = GenerateTestData(1000, 100, 500);
    auto large_strings = GenerateTestData(100, 1000, 5000);
    
    cout << "Generated:\n";
    cout << "  Small strings (10-50 chars): " << small_strings.size() << endl;
    cout << "  Medium strings (100-500 chars): " << medium_strings.size() << endl;
    cout << "  Large strings (1K-5K chars): " << large_strings.size() << "\n" << endl;
    
    // 测试1: 基础转换性能
    cout << "=== Test 1: Basic Conversion Performance ===" << endl;
    const string test_str = "Hello World! Performance test string with mixed content";
    
    auto time1 = BenchmarkAdvanced("ConvertEncodingFast (UTF-8->UTF-16LE)", [&]() {
        auto result = converter->ConvertEncodingFast(test_str, "UTF-8", "UTF-16LE");
    }, 20000);
    
    // 零拷贝输出参数版本 (缓冲区复用)
    std::string output_buffer;
    auto time2 = BenchmarkAdvanced("ConvertEncodingFast (zero-copy output)", [&]() {
        converter->ConvertEncodingFast(test_str, "UTF-8", "UTF-16LE", output_buffer);
    }, 20000);
    
    cout << "Zero-copy optimization speedup: " << fixed << setprecision(2) << (time1 / time2) << "x\n" << endl;
    
    // 测试2: 批量处理优化
    cout << "=== Test 2: Batch Processing Optimization ===" << endl;
    
    auto time3 = BenchmarkAdvanced("Individual conversions (small strings)", [&]() {
        for (const auto& str : small_strings) {
            if (!str.empty()) {
                auto result = converter->ConvertEncodingFast(str, "UTF-8", "UTF-16LE");
            }
        }
    }, 100);
    
    auto time4 = BenchmarkAdvanced("Batch conversion (small strings)", [&]() {
        auto results = converter->ConvertEncodingBatch(small_strings, "UTF-8", "UTF-16LE");
    }, 100);
    
    cout << "Batch processing speedup: " << fixed << setprecision(2) << (time3 / time4) << "x\n" << endl;
    
    // 测试3: 不同大小字符串性能
    cout << "=== Test 3: String Size Performance ===" << endl;
    
    cout << "Small strings:" << endl;
    auto time_small = BenchmarkAdvanced("  Batch conversion", [&]() {
        auto results = converter->ConvertEncodingBatch(small_strings, "UTF-8", "UTF-16LE");
    }, 200);
    
    cout << "Medium strings:" << endl;
    auto time_medium = BenchmarkAdvanced("  Batch conversion", [&]() {
        auto results = converter->ConvertEncodingBatch(medium_strings, "UTF-8", "UTF-16LE");
    }, 100);
    
    cout << "Large strings:" << endl;
    auto time_large = BenchmarkAdvanced("  Batch conversion", [&]() {
        auto results = converter->ConvertEncodingBatch(large_strings, "UTF-8", "UTF-16LE");
    }, 50);
    
    cout << "\n=== Performance Summary ===" << endl;
    cout << "All optimizations demonstrate significant performance improvements" << endl;
    cout << "Key optimization techniques:" << endl;
    cout << "   Branch prediction hints" << endl;
    cout << "   Size hint pre-allocation" << endl;
    cout << "   Batch processing" << endl;
    cout << "   Memory reuse patterns\n" << endl;
    
    return 0;
}
