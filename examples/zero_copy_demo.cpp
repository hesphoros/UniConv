/**
 * @file zero_copy_demo.cpp
 * @brief UniConv 零拷贝性能演示
 * @details 对比输出参数版本与返回值版本的性能差异，展示零拷贝优化效果
 */

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#include "UniConv.h"
#include <iostream>
#include <chrono>
#include <vector>
#include <iomanip>

using namespace std;
using namespace std::chrono;

// 性能基准测试辅助函数
template<typename Func>
double BenchmarkMs(Func&& func, int iterations) {
    auto start = high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        func();
    }
    auto end = high_resolution_clock::now();
    return duration_cast<microseconds>(end - start).count() / 1000.0;
}

void PrintHeader(const string& title) {
    cout << "\n" << string(60, '=') << "\n  " << title << "\n" << string(60, '=') << "\n";
}

void PrintResult(const string& name, double time_ms, double baseline_ms = 0) {
    cout << left << setw(40) << name << right << setw(10) << fixed << setprecision(2) << time_ms << " ms";
    if (baseline_ms > 0) {
        cout << "  (Speedup: " << fixed << setprecision(2) << (baseline_ms / time_ms) << "x)";
    }
    cout << "\n";
}

int main() {
    auto converter = UniConv::Create();
    
    cout << "\n-----------------------------------------------------------\n";
    cout << "|      UniConv Zero-Copy Performance Demo                    |\n";
    cout << "|------------------------------------------------------------|\n";
    
    const int iterations = 10000;
    const string test_text = "Performance test string for encoding conversion";
    
    // 测试1: 单次转换 - 返回值 vs 输出参数
    PrintHeader("Test 1: Single Conversion (10,000 iterations)");
    
    // 返回值版本
    double time_return = BenchmarkMs([&]() {
        string result = converter->ToUtf8FromLocale(test_text);
    }, iterations);
    PrintResult("Return value version:", time_return);
    
    // 输出参数版本（内存复用）
    string output;
    output.reserve(1024);
    double time_output = BenchmarkMs([&]() {
        converter->ToUtf8FromLocale(test_text, output);
    }, iterations);
    PrintResult("Output parameter version:", time_output, time_return);
    
    // 测试2: 循环转换 - 内存复用效果
    PrintHeader("Test 2: Loop Conversion (1,000 x 10)");
    
    vector<string> test_inputs = {
        "Short text", "Medium test string", "Long test for performance",
        "Another case", "Performance optimization", "Zero-copy semantics",
        "Thread-local cache", "High performance library", "Memory reuse", "Efficient design"
    };
    
    // 返回值版本
    double time_loop_return = BenchmarkMs([&]() {
        for (const auto& input : test_inputs) {
            string result = converter->ToUtf8FromLocale(input);
        }
    }, 1000);
    PrintResult("Return value (new allocation):", time_loop_return);
    
    // 输出参数版本
    string reused_output;
    reused_output.reserve(1024);
    double time_loop_output = BenchmarkMs([&]() {
        for (const auto& input : test_inputs) {
            converter->ToUtf8FromLocale(input, reused_output);
        }
    }, 1000);
    PrintResult("Output parameter (reuse):", time_loop_output, time_loop_return);
    
    // 总结
    PrintHeader("Performance Summary");
    double avg_speedup = ((time_return / time_output) + (time_loop_return / time_loop_output)) / 2.0;
    
    cout << "\n Average Speedup: " << fixed << setprecision(2) << avg_speedup << "x\n";
    cout << "\n Key Benefits:\n";
    cout << "   - Zero-copy semantics - reuses caller's memory\n";
    cout << "   - Eliminates move/copy overhead\n";
    cout << "   - Better cache locality in loops\n";
    cout << "   - Ideal for high-frequency conversions\n\n";
    
    return 0;
}
