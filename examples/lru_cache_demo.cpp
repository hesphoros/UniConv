/**
 * @file lru_cache_demo.cpp
 * @brief LRU Cache Performance Testing and Validation Program
 * @author UniConv Team
 * @date 2024
 * 
 * Demonstrates and tests the performance improvements of iconv descriptor LRU cache mechanism
 */

#include <iostream>
#include <chrono>
#include <vector>
#include <random>
#include <thread>
#include <iomanip>
#include "../include/UniConv.h"

/**
 * @brief Performance timer class
 */
class PerformanceTimer {
public:
    PerformanceTimer() : start_time(std::chrono::high_resolution_clock::now()) {}
    
    double ElapsedMilliseconds() const {
        auto end_time = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double, std::milli>(end_time - start_time).count();
    }
    
    void Reset() {
        start_time = std::chrono::high_resolution_clock::now();
    }

private:
    std::chrono::high_resolution_clock::time_point start_time;
};

/**
 * @brief Print statistics information
 */
void PrintStatistics(const UniConv::PoolStats& stats, const std::string& test_name) {
    std::cout << "\n=== " << test_name << " Statistics ===" << std::endl;
    std::cout << "String Buffer Pool Active Buffers: " << stats.active_buffers << std::endl;
    std::cout << "Total Conversions: " << stats.total_conversions << std::endl;
    std::cout << "Pool Cache Hits: " << stats.cache_hits << std::endl;
    std::cout << "Pool Hit Rate: " << std::fixed << std::setprecision(2) 
              << (stats.hit_rate * 100) << "%" << std::endl;
    
    std::cout << "\n--- iconv Descriptor Cache Statistics ---" << std::endl;
    std::cout << "Cache Size: " << stats.iconv_cache_size << std::endl;
    std::cout << "Cache Hits: " << stats.iconv_cache_hits << std::endl;
    std::cout << "Cache Misses: " << stats.iconv_cache_misses << std::endl;
    std::cout << "Cache Evictions: " << stats.iconv_cache_evictions << std::endl;
    std::cout << "iconv Hit Rate: " << std::fixed << std::setprecision(2) 
              << (stats.iconv_cache_hit_rate * 100) << "%" << std::endl;
    std::cout << "Average Hit Count: " << std::fixed << std::setprecision(2) 
              << stats.iconv_avg_hit_count << std::endl;
}

/**
 * @brief Basic cache performance test
 */
void TestBasicCachePerformance() {
    std::cout << "\nBasic Cache Performance Test..." << std::endl;
    
    auto converter = UniConv::GetInstance();
    const std::string test_data = "Hello World! Test Data!";
    
    // Test data - 使用经过验证的编码转换对
    std::vector<std::pair<std::string, std::string>> encoding_pairs = {
        {"UTF-8", "UTF-16LE"},
        {"UTF-8", "UTF-16BE"},
        {"UTF-8", "GBK"},
        {"UTF-8", "GB2312"}
    };
    
    PerformanceTimer timer;
    
    // First round: Establish cache
    for (int round = 0; round < 3; ++round) {
        for (const auto& [from, to] : encoding_pairs) {
            auto result = converter->ConvertEncodingFast(test_data, from.c_str(), to.c_str());
            if (!result.IsSuccess()) {
                std::cout << "Conversion failed: " << from << " -> " << to << std::endl;
            }
        }
    }
    
    double first_phase_time = timer.ElapsedMilliseconds();
    auto stats_after_warmup = converter->GetPoolStatistics();
    
    timer.Reset();
    
    // Second round: Utilize cache for high-frequency access
    for (int round = 0; round < 100; ++round) {
        for (const auto& [from, to] : encoding_pairs) {
            auto result = converter->ConvertEncodingFast(test_data, from.c_str(), to.c_str());
            if (!result.IsSuccess()) {
                std::cout << "Conversion failed: " << from << " -> " << to << std::endl;
            }
        }
    }
    
    double second_phase_time = timer.ElapsedMilliseconds();
    auto final_stats = converter->GetPoolStatistics();
    
    std::cout << "Warmup phase time: " << std::fixed << std::setprecision(2) 
              << first_phase_time << " ms" << std::endl;
    std::cout << "High-frequency access phase time: " << std::fixed << std::setprecision(2) 
              << second_phase_time << " ms" << std::endl;
    std::cout << "Average time per conversion: " << std::fixed << std::setprecision(4) 
              << (second_phase_time / (100 * encoding_pairs.size())) << " ms" << std::endl;
    
    PrintStatistics(final_stats, "Basic Cache Performance Test");
}

/**
 * @brief LRU cache stress test
 */
void TestLRUCacheStress() {
    std::cout << "\nLRU Cache Stress Test..." << std::endl;
    
    auto converter = UniConv::GetInstance();
    converter->SetDefaultEncoding("UTF-8");
    const std::string test_data = "Test Data for Stress Testing";
    
    // Generate many different encoding pairs to test LRU mechanism
    std::vector<std::pair<std::string, std::string>> large_encoding_pairs;
    
    // Basic encodings
    std::vector<std::string> encodings = {
        "UTF-8", "UTF-16LE", "UTF-16BE", "UTF-32LE", "UTF-32BE", "ASCII", "ISO-8859-1"
    };
    
    // Generate all possible encoding pairs
    for (const auto& from : encodings) {
        for (const auto& to : encodings) {
            if (from != to) {
                large_encoding_pairs.emplace_back(from, to);
            }
        }
    }
    
    std::cout << "Total encoding pairs: " << large_encoding_pairs.size() << std::endl;
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, static_cast<int>(large_encoding_pairs.size() - 1));
    
    PerformanceTimer timer;
    
    // Random access test to trigger LRU cleanup
    for (int i = 0; i < 2000; ++i) {
        int idx = dis(gen);
        const auto& [from, to] = large_encoding_pairs[idx];
        
        auto result = converter->ConvertEncodingFast(test_data, from.c_str(), to.c_str());
        // Ignore unsupported encoding conversions
        
        // Output progress every 500 operations
        if ((i + 1) % 500 == 0) {
            auto intermediate_stats = converter->GetPoolStatistics();
            std::cout << "Progress " << (i + 1) << "/2000, Current hit rate: " 
                      << std::fixed << std::setprecision(1) 
                      << (intermediate_stats.iconv_cache_hit_rate * 100) << "%" << std::endl;
        }
    }
    
    double total_time = timer.ElapsedMilliseconds();
    auto final_stats = converter->GetPoolStatistics();
    
    std::cout << "Stress test total time: " << std::fixed << std::setprecision(2) 
              << total_time << " ms" << std::endl;
    std::cout << "Average time per conversion: " << std::fixed << std::setprecision(4) 
              << (total_time / 2000) << " ms" << std::endl;
    
    PrintStatistics(final_stats, "LRU Cache Stress Test");
}

/**
 * @brief Multi-thread cache contention test
 */
void TestMultiThreadCacheContention() {
    std::cout << "\nMulti-thread Cache Contention Test..." << std::endl;
    
    auto converter = UniConv::GetInstance();
    const std::string test_data = "MultiThread Test Data";
    constexpr int NUM_THREADS = 4;
    constexpr int OPERATIONS_PER_THREAD = 500;
    
    std::vector<std::pair<std::string, std::string>> common_pairs = {
        {"UTF-8", "UTF-16LE"},
        {"UTF-8", "UTF-16BE"},
        {"UTF-16LE", "UTF-8"},
        {"UTF-16BE", "UTF-8"},
        {"UTF-8", "UTF-32LE"},
        {"UTF-32LE", "UTF-8"}
    };
    
    PerformanceTimer timer;
    std::vector<std::thread> threads;
    
    // Start multiple threads for concurrent cache access
    for (int t = 0; t < NUM_THREADS; ++t) {
        threads.emplace_back([converter, &test_data, &common_pairs, t, OPERATIONS_PER_THREAD]() {
            std::random_device rd;
            std::mt19937 gen(rd() + t);  // Each thread uses different seed
            std::uniform_int_distribution<> dis(0, static_cast<int>(common_pairs.size() - 1));
            
            for (int i = 0; i < OPERATIONS_PER_THREAD; ++i) {
                int idx = dis(gen);
                const auto& [from, to] = common_pairs[idx];
                
                auto result = converter->ConvertEncodingFast(test_data, from.c_str(), to.c_str());
                if (!result.IsSuccess()) {
                    // Conversion failed, continue to next
                }
            }
        });
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    double total_time = timer.ElapsedMilliseconds();
    auto final_stats = converter->GetPoolStatistics();
    
    std::cout << "Multi-thread test total time: " << std::fixed << std::setprecision(2) 
              << total_time << " ms" << std::endl;
    std::cout << "Total operations: " << (NUM_THREADS * OPERATIONS_PER_THREAD) << std::endl;
    std::cout << "Average time per operation: " << std::fixed << std::setprecision(4) 
              << (total_time / (NUM_THREADS * OPERATIONS_PER_THREAD)) << " ms" << std::endl;
    
    PrintStatistics(final_stats, "Multi-thread Cache Contention Test");
}

/**
 * @brief Main function
 */
int main() {
    std::cout << "==================================================" << std::endl;
    std::cout << "   UniConv LRU Cache Performance Test Program" << std::endl;
    std::cout << "==================================================" << std::endl;
    
    try {
        // Execute various tests
        TestBasicCachePerformance();
        TestLRUCacheStress();
        TestMultiThreadCacheContention();
        
        std::cout << "\nAll tests completed successfully!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception occurred during testing: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}