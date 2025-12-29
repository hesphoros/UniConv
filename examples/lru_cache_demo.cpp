/**
 * @file lru_cache_demo.cpp
 * @brief UniConv LRU 缓存性能演示
 * @details 演示 iconv 描述符 LRU 缓存机制的性能提升效果
 */

#include <iostream>
#include <chrono>
#include <vector>
#include <iomanip>
#include "../include/UniConv.h"

using namespace std;
using namespace std::chrono;

// 性能计时器类
class PerformanceTimer {
public:
    PerformanceTimer() : start_time(high_resolution_clock::now()) {}
    
    double ElapsedMilliseconds() const {
        auto end_time = high_resolution_clock::now();
        return duration<double, milli>(end_time - start_time).count();
    }
    
    void Reset() {
        start_time = high_resolution_clock::now();
    }

private:
    high_resolution_clock::time_point start_time;
};

// 打印统计信息
void PrintStatistics(const UniConv::PoolStats& stats, const string& test_name) {
    cout << "\n=== " << test_name << " ===" << endl;
    cout << "String Buffer Pool Active: " << stats.active_buffers << endl;
    cout << "Total Conversions: " << stats.total_conversions << endl;
    cout << "Pool Cache Hits: " << stats.cache_hits << endl;
    cout << "Pool Hit Rate: " << fixed << setprecision(2) 
         << (stats.hit_rate * 100) << "%" << endl;
    
    cout << "\niconv Descriptor Cache:" << endl;
    cout << "  Cache Size: " << stats.iconv_cache_size << endl;
    cout << "  Cache Hits: " << stats.iconv_cache_hits << endl;
    cout << "  Cache Misses: " << stats.iconv_cache_misses << endl;
    cout << "  Cache Evictions: " << stats.iconv_cache_evictions << endl;
    cout << "  iconv Hit Rate: " << fixed << setprecision(2) 
         << (stats.iconv_cache_hit_rate * 100) << "%" << endl;
    cout << "  Average Hit Count: " << fixed << setprecision(2) 
         << stats.iconv_avg_hit_count << endl;
}

// 基础缓存性能测试
void TestBasicCachePerformance() {
    cout << "\nBasic cache performance test..." << endl;
    
    auto converter = UniConv::Create();
    const string test_data = "Hello World! Performance test data";
    
    // 测试数据 - 使用已验证的编码转换对
    vector<pair<string, string>> encoding_pairs = {
        {"UTF-8", "UTF-16LE"},
        {"UTF-8", "UTF-16BE"},
        {"UTF-8", "GBK"},
        {"UTF-8", "GB2312"}
    };
    
    PerformanceTimer timer;
    
    // 第一轮：建立缓存
    for (int round = 0; round < 3; ++round) {
        for (const auto& [from, to] : encoding_pairs) {
            auto result = converter->ConvertEncodingFast(test_data, from.c_str(), to.c_str());
        }
    }
    
    double first_phase_time = timer.ElapsedMilliseconds();
    auto stats_after_warmup = converter->GetPoolStatistics();
    
    timer.Reset();
    
    // 第二轮：高频访问利用缓存
    for (int round = 0; round < 100; ++round) {
        for (const auto& [from, to] : encoding_pairs) {
            auto result = converter->ConvertEncodingFast(test_data, from.c_str(), to.c_str());
        }
    }
    
    double second_phase_time = timer.ElapsedMilliseconds();
    auto final_stats = converter->GetPoolStatistics();
    
    cout << "Warmup phase: " << fixed << setprecision(2) << first_phase_time << " ms" << endl;
    cout << "High-frequency phase: " << fixed << setprecision(2) << second_phase_time << " ms" << endl;
    cout << "Average per conversion: " << fixed << setprecision(4) 
         << (second_phase_time / (100 * encoding_pairs.size())) << " ms" << endl;
    
    PrintStatistics(final_stats, "Basic Cache Performance");
}

// 多线程缓存测试
void TestMultithreadedCache() {
    cout << "\nMultithreaded cache test..." << endl;
    
    auto converter = UniConv::Create();
    const string test_data = "Multithreaded cache test data";
    
    // 使用并行批量转换测试多线程缓存
    vector<string> inputs;
    for (int i = 0; i < 500; ++i) {
        inputs.push_back(test_data + to_string(i));
    }
    
    PerformanceTimer timer;
    
    // 并行批量转换会触发多线程缓存访问
    auto results = converter->ConvertEncodingBatchParallel(inputs, "UTF-8", "UTF-16LE");
    
    double elapsed = timer.ElapsedMilliseconds();
    auto stats = converter->GetPoolStatistics();
    
    cout << "Parallel conversion time: " << fixed << setprecision(2) << elapsed << " ms" << endl;
    cout << "Conversions completed: " << results.size() << endl;
    
    PrintStatistics(stats, "Multithreaded Cache Test");
}

int main() {
    cout << "==============================================================\n";
    cout << "=   UniConv LRU Cache Performance Demonstration              =\n";
    cout << "==============================================================\n";
    
    TestBasicCachePerformance();
    TestMultithreadedCache();
    
    cout << "\n=== All Tests Completed ===" << endl;
    
    return 0;
}
