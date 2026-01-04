/**
 * @file encoding_validation_test.cpp
 * @brief UniConv 编码验证功能测试
 * @details 测试编码名称有效性检查功能，验证热路径方法的优化效果
 */

#include "../include/UniConv.h"
#include <iostream>
#include <string>
#include <chrono>

int main() {
    std::cout << "=== UniConv Encoding Validation Test ===" << std::endl;
    
    // 获取 UniConv 实例
    auto converter = UniConv::Create();
    
    std::string test_input = "Hello, World! 测试";
    
    // 测试1: 有效编码名称（应该成功）
    std::cout << "\n1. Valid Encoding Names Test:" << std::endl;
    struct {
        const char* from_enc;
        const char* to_enc;
        const char* description;
    } valid_tests[] = {
        {"UTF-8", "UTF-16LE", "UTF-8 to UTF-16LE"},
        {"GB2312", "UTF-8", "GB2312 to UTF-8"},
        {"ISO-8859-1", "UTF-8", "ISO-8859-1 to UTF-8"},
        {"CP1252", "UTF-8", "CP1252 to UTF-8"},
        {"BIG5", "UTF-8", "BIG5 to UTF-8"}
    };
    
    for (const auto& test : valid_tests) {
        auto result = converter->ConvertEncodingFast(test_input, test.from_enc, test.to_enc);
        std::cout << test.description << ": " 
                  << (result.IsSuccess() ? "SUCCESS" : ("FAILED - " + std::string(result.GetErrorMessage()))) 
                  << std::endl;
    }
    
    // 测试2: 无效编码名称（应该快速失败）
    std::cout << "\n2. Invalid Encoding Names Test:" << std::endl;
    struct {
        const char* from_enc;
        const char* to_enc;
        const char* description;
        const char* expected_error;
    } invalid_tests[] = {
        {"INVALID_ENCODING", "UTF-8", "Invalid source encoding", "Invalid source encoding"},
        {"UTF-8", "INVALID_ENCODING", "Invalid target encoding", "Invalid target encoding"},
        {"", "UTF-8", "Empty source encoding", "Invalid parameter"},
        {"UTF-8", "", "Empty target encoding", "Invalid parameter"},
        {nullptr, "UTF-8", "NULL source encoding", "Invalid parameter"},
        {"UTF-8", nullptr, "NULL target encoding", "Invalid parameter"},
        {"@#$%^&*()", "UTF-8", "Special characters", "Invalid source encoding"},
        {"UTF-8", "!@#$%^&*()", "Special characters in target", "Invalid target encoding"},
        {"VERY_LONG_INVALID_ENCODING_NAME_THAT_EXCEEDS_REASONABLE_LENGTH", "UTF-8", "Very long invalid encoding", "Invalid source encoding"}
    };
    
    for (const auto& test : invalid_tests) {
        auto start = std::chrono::high_resolution_clock::now();
        auto result = converter->ConvertEncodingFast(test_input, test.from_enc, test.to_enc);
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        bool correct_error = !result.IsSuccess() && 
                           (strstr(result.GetErrorMessage(), "Invalid") != nullptr ||
                            strstr(result.GetErrorMessage(), "Invalid parameter") != nullptr);
        
        std::cout << test.description << ": " 
                  << (correct_error ? "CORRECT ERROR" : "UNEXPECTED RESULT")
                  << " (" << duration.count() << " μs) - " << result.GetErrorMessage()
                  << std::endl;
    }
    
    // 测试3: 性能对比（有效vs无效编码）
    std::cout << "\n3. Performance Comparison:" << std::endl;
    
    const int iterations = 1000;
    
    // 测试有效编码的性能
    auto valid_start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        auto result = converter->ConvertEncodingFast("test", "UTF-8", "UTF-16LE");
        (void)result; // 避免编译器优化
    }
    auto valid_end = std::chrono::high_resolution_clock::now();
    auto valid_duration = std::chrono::duration_cast<std::chrono::microseconds>(valid_end - valid_start);
    
    // 测试无效编码的性能（应该更快）
    auto invalid_start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        auto result = converter->ConvertEncodingFast("test", "INVALID_ENC", "UTF-16LE");
        (void)result; // 避免编译器优化
    }
    auto invalid_end = std::chrono::high_resolution_clock::now();
    auto invalid_duration = std::chrono::duration_cast<std::chrono::microseconds>(invalid_end - invalid_start);
    
    std::cout << "Valid encoding (" << iterations << " iterations): " << valid_duration.count() << " μs total, " 
              << (double)valid_duration.count() / iterations << " μs average" << std::endl;
    std::cout << "Invalid encoding (" << iterations << " iterations): " << invalid_duration.count() << " μs total, " 
              << (double)invalid_duration.count() / iterations << " μs average" << std::endl;
    
    double speedup = (double)valid_duration.count() / invalid_duration.count();
    std::cout << "Invalid encoding validation speedup: " << speedup << "x faster" << std::endl;
    
    // 测试4: 批量处理的编码验证
    std::cout << "\n4. Batch Processing Validation Test:" << std::endl;
    
    std::vector<std::string> inputs = {"Hello", "World", "Test", "Batch"};
    
    // 有效编码的批量处理
    auto batch_result = converter->ConvertEncodingBatch(inputs, "UTF-8", "UTF-16LE");
    std::cout << "Valid batch conversion: " << batch_result.size() << " results, "
              << "successes: ";
    int successes = 0;
    for (const auto& res : batch_result) {
        if (res.IsSuccess()) successes++;
    }
    std::cout << successes << std::endl;
    
    // 无效编码的批量处理
    auto invalid_batch_result = converter->ConvertEncodingBatch(inputs, "INVALID_ENC", "UTF-16LE");
    std::cout << "Invalid batch conversion: " << invalid_batch_result.size() << " results, "
              << "failures: ";
    int failures = 0;
    for (const auto& res : invalid_batch_result) {
        if (!res.IsSuccess()) failures++;
    }
    std::cout << failures << " (all should be failures)" << std::endl;
    
    std::cout << "\n=== Encoding Validation Test Completed ===" << std::endl;
    
    return 0;
}