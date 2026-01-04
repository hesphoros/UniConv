/**
 * @file parallel_batch_demo.cpp
 * @brief UniConv 并行批量转换演示
 * @details 展示并行批量转换的性能优势，对比串行与并行处理
 */

#pragma execution_character_set("utf-8")

#include <iostream>
#include <chrono>
#include <vector>
#include <iomanip>
#include "UniConv.h"

using namespace std;
using namespace std::chrono;

void PrintSeparator() {
    cout << string(70, '=') << endl;
}

void PrintHeader(const string& title) {
    cout << "\n";
    PrintSeparator();
    cout << "  " << title << endl;
    PrintSeparator();
}

// 演示1: 基本并行批量转换
void Demo1_BasicParallelBatch() {
    PrintHeader("Demo 1: Basic Parallel Batch Conversion");
    
    auto converter = UniConv::Create();
    
    // 准备测试数据
    vector<string> inputs;
    for (int i = 0; i < 50; ++i) {
        inputs.push_back("Message " + to_string(i) + ": UniConv parallel processing test");
    }
    
    cout << "  Input data: " << inputs.size() << " messages" << endl;
    cout << "  Conversion: UTF-8 → GBK" << endl;
    
    // 并行批量转换
    auto start = high_resolution_clock::now();
    auto results = converter->ConvertEncodingBatchParallel(inputs, "UTF-8", "GBK");
    auto end = high_resolution_clock::now();
    
    auto duration = duration_cast<microseconds>(end - start).count();
    
    cout << "\n  Completed" << endl;
    cout << "  Time: " << duration << " us" << endl;
    cout << "  Success: " << results.size() << " items" << endl;
}

// 演示2: 使用输出参数版本（零拷贝）
void Demo2_OutputParameterVersion() {
    PrintHeader("Demo 2: Output Parameter Version (Zero-Copy)");
    
    auto converter = UniConv::Create();
    
    // 准备测试数据
    vector<string> inputs;
    for (int i = 0; i < 100; ++i) {
        inputs.push_back("Test " + to_string(i) + ": UniConv parallel batch conversion");
    }
    
    cout << "  Input data: " << inputs.size() << " messages" << endl;
    cout << "  Conversion: UTF-8 → GB2312" << endl;
    
    // 预分配输出向量
    vector<string> outputs;
    
    // 并行批量转换（输出参数版本）
    auto start = high_resolution_clock::now();
    bool success = converter->ConvertEncodingBatchParallel(inputs, "UTF-8", "GB2312", outputs);
    auto end = high_resolution_clock::now();
    
    auto duration = duration_cast<microseconds>(end - start).count();
    
    cout << "\n  Completed" << endl;
    cout << "  Time: " << duration << " us" << endl;
    cout << "  Status: " << (success ? "All success" : "Partial failure") << endl;
    cout << "  Output: " << outputs.size() << " items" << endl;
}

// 演示3: 性能对比 - 串行 vs 并行
void Demo3_PerformanceComparison() {
    PrintHeader("Demo 3: Performance Comparison - Serial vs Parallel");
    
    auto converter = UniConv::Create();
    
    // 准备大批量数据
    const int batch_size = 500;
    vector<string> inputs;
    inputs.reserve(batch_size);
    
    for (int i = 0; i < batch_size; ++i) {
        inputs.push_back("Test data " + to_string(i) + ": Performance comparison test");
    }
    
    cout << "  Input data: " << batch_size << " messages" << endl;
    cout << "  Conversion: UTF-8 → GB18030" << endl;
    
    // 串行批量转换
    cout << "\n  1. Serial batch conversion..." << endl;
    auto start_serial = high_resolution_clock::now();
    auto results_serial = converter->ConvertEncodingBatch(inputs, "UTF-8", "GB18030");
    auto end_serial = high_resolution_clock::now();
    auto duration_serial = duration_cast<microseconds>(end_serial - start_serial).count();
    
    cout << "     Time: " << duration_serial << " μs" << endl;
    
    // 并行批量转换
    cout << "\n  2. Parallel batch conversion..." << endl;
    auto start_parallel = high_resolution_clock::now();
    auto results_parallel = converter->ConvertEncodingBatchParallel(inputs, "UTF-8", "GB18030");
    auto end_parallel = high_resolution_clock::now();
    auto duration_parallel = duration_cast<microseconds>(end_parallel - start_parallel).count();
    
    cout << "     Time: " << duration_parallel << " μs" << endl;
    
    // 计算加速比
    double speedup = static_cast<double>(duration_serial) / duration_parallel;
    cout << "\n  Performance improvement: " << fixed << setprecision(2) << speedup << "x" << endl;
    cout << "  Time reduction: " << fixed << setprecision(1) 
         << ((duration_serial - duration_parallel) * 100.0 / duration_serial) << "%" << endl;
}

// 演示4: 不同线程数性能测试
void Demo4_ThreadScaling() {
    PrintHeader("Demo 4: Thread Scaling Test");
    
    auto converter = UniConv::Create();
    
    // 准备测试数据
    const int batch_size = 400;
    vector<string> inputs;
    inputs.reserve(batch_size);
    
    for (int i = 0; i < batch_size; ++i) {
        inputs.push_back("Thread scaling test " + to_string(i) + ": Evaluate thread performance");
    }
    
    cout << "  Input data: " << batch_size << " messages" << endl;
    cout << "  Conversion: UTF-8 → GBK" << endl;
    
    // 测试不同线程数
    vector<int> thread_counts = {1, 2, 4, 8};
    vector<double> durations;
    
    cout << "\n  Test results:" << endl;
    
    for (int num_threads : thread_counts) {
        auto start = high_resolution_clock::now();
        auto results = converter->ConvertEncodingBatchParallel(inputs, "UTF-8", "GBK", num_threads);
        auto end = high_resolution_clock::now();
        
        auto duration = duration_cast<microseconds>(end - start).count();
        durations.push_back(duration);
        
        cout << "    " << num_threads << " thread(s): " << setw(8) << duration << " μs";
        
        if (num_threads > 1 && !durations.empty()) {
            double speedup = durations[0] / duration;
            cout << "  (Speedup: " << fixed << setprecision(2) << speedup << "x)";
        }
        cout << endl;
    }
    
    // 计算并行效率
    if (durations.size() >= 4) {
        double efficiency_4 = (durations[0] / durations[2]) / 4.0 * 100;
        double efficiency_8 = (durations[0] / durations[3]) / 8.0 * 100;
        cout << "\n  Parallel efficiency:" << endl;
        cout << "    4 threads: " << fixed << setprecision(1) << efficiency_4 << "%" << endl;
        cout << "    8 threads: " << fixed << setprecision(1) << efficiency_8 << "%" << endl;
    }
}

int main() {
    cout << "\n============================================================\n";
    cout << "=   UniConv Parallel Batch Conversion Demonstration         =\n";
    cout << "==============================================================\n";
    
    Demo1_BasicParallelBatch();
    Demo2_OutputParameterVersion();
    Demo3_PerformanceComparison();
    Demo4_ThreadScaling();
    
    cout << "\n" << string(70, '=') << endl;
    cout << "All demos completed successfully!" << endl;
    cout << string(70, '=') << "\n" << endl;
    
    return 0;
}
