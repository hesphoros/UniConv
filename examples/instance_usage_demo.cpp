/**
 * @file instance_usage_demo.cpp
 * @brief Demonstrates different ways to use UniConv instances
 * @details Shows factory pattern (Create) and direct construction
 * 
 * This example demonstrates:
 * 1. Creating independent instances with Create() - recommended way
 * 2. Using stack-allocated instances with direct construction
 * 3. Benefits of using independent instances for better testability
 * 4. Thread-safety with independent instances
 * 
 * @author UniConv Team
 * @date 2025-12-29
 */

#include <iostream>
#include <memory>
#include <vector>
#include <thread>
#include "../include/UniConv.h"

using namespace std;

/**
 * @brief Example 1: Using factory method to create independent instances (Recommended)
 */
void FactoryUsageExample() {
    cout << "\n=== Example 1: Factory Pattern (Recommended) ===" << endl;
    
    // Create new independent instances using Create()
    auto converter1 = UniConv::Create();
    auto converter2 = UniConv::Create();
    
    string utf8_text = "Hello, 世界! Independent instance";
    
    // Each instance has its own cache and state
    auto result1 = converter1->ConvertEncodingFast(utf8_text, "UTF-8", "UTF-16LE");
    auto result2 = converter2->ConvertEncodingFast(utf8_text, "UTF-8", "GBK");
    
    if (result1.IsSuccess() && result2.IsSuccess()) {
        cout << "Both instances work independently!" << endl;
        cout << "  Instance 1 (UTF-16LE): " << result1.GetValue().size() << " bytes" << endl;
        cout << "  Instance 2 (GBK): " << result2.GetValue().size() << " bytes" << endl;
    }
    
    cout << "  Different instances? " << (converter1.get() != converter2.get() ? "Yes" : "No") << endl;
    cout << "  Automatic cleanup via unique_ptr: Yes" << endl;
}

/**
 * @brief Example 2: Using stack-allocated instance
 */
void StackInstanceExample() {
    cout << "\n=== Example 2: Stack-Allocated Instance ===" << endl;
    
    // Create instance on stack (direct construction)
    UniConv converter;
    
    string utf8_text = "Hello, 世界! Stack instance";
    auto result = converter.ConvertEncodingFast(utf8_text, "UTF-8", "UTF-16LE");
    
    if (result.IsSuccess()) {
        cout << "Stack instance works!" << endl;
        cout << "  Output size: " << result.GetValue().size() << " bytes" << endl;
    }
    
    cout << "  Automatic cleanup when out of scope: Yes" << endl;
    cout << "  Note: Good for RAII and local usage" << endl;
}

/**
 * @brief Example 3: Thread-safety with independent instances
 */
void ThreadSafetyExample() {
    cout << "\n=== Example 3: Thread Safety with Independent Instances ===" << endl;
    
    vector<thread> threads;
    const int num_threads = 4;
    
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([i]() {
            // Each thread gets its own instance
            auto converter = UniConv::Create();
            
            string text = "Thread " + to_string(i) + ": Hello, 世界!";
            auto result = converter->ConvertEncodingFast(text, "UTF-8", "UTF-16LE");
            
            if (result.IsSuccess()) {
                cout << "  Thread " << i << " completed: " 
                     << result.GetValue().size() << " bytes" << endl;
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    cout << "  All threads completed successfully with independent instances!" << endl;
}

/**
 * @brief Example 4: Comparing factory vs stack allocation performance
 */
void PerformanceComparison() {
    cout << "\n=== Example 4: Performance Comparison ===" << endl;
    
    string utf8_text = "Performance test string 性能测试字符串";
    const int iterations = 1000;
    
    // Test with factory pattern (heap allocation)
    auto start = chrono::high_resolution_clock::now();
    {
        auto converter = UniConv::Create();
        for (int i = 0; i < iterations; ++i) {
            auto result = converter->ConvertEncodingFast(utf8_text, "UTF-8", "UTF-16LE");
        }
    }
    auto factory_time = chrono::duration_cast<chrono::microseconds>(
        chrono::high_resolution_clock::now() - start).count();
    
    // Test with stack allocation
    start = chrono::high_resolution_clock::now();
    {
        UniConv converter;
        for (int i = 0; i < iterations; ++i) {
            auto result = converter.ConvertEncodingFast(utf8_text, "UTF-8", "UTF-16LE");
        }
    }
    auto stack_time = chrono::duration_cast<chrono::microseconds>(
        chrono::high_resolution_clock::now() - start).count();
    
    cout << "  Factory pattern (heap): " << factory_time << " μs" << endl;
    cout << "  Stack allocation: " << stack_time << " μs" << endl;
    cout << "  Performance difference: " << abs(factory_time - stack_time) << " μs ("
         << (abs(factory_time - stack_time) * 100.0 / max(factory_time, stack_time)) << "%)" << endl;
}

/**
 * @brief Main entry point
 */
int main() {
    cout << "========================================" << endl;
    cout << "  UniConv Instance Usage Demonstration" << endl;
    cout << "========================================" << endl;
    
    try {
        // Run all examples
        FactoryUsageExample();
        StackInstanceExample();
        ThreadSafetyExample();
        PerformanceComparison();
        
        cout << "\n========================================" << endl;
        cout << "  All examples completed successfully!" << endl;
        cout << "========================================" << endl;
        
        cout << "\nKey Takeaways:" << endl;
        cout << "  ✓ Create() - Recommended for heap allocation with automatic cleanup" << endl;
        cout << "  ✓ Direct construction - Use for stack allocation and RAII" << endl;
        cout << "  ✓ Independent instances - Better for multi-threading" << endl;
        cout << "  ✓ No singleton - Each instance is independent" << endl;
        cout << "  ✓ Performance - Similar between factory and stack allocation" << endl;
        
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}
