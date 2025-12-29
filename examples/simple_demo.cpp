/**
 * @file simple_demo.cpp
 * @brief UniConv 基础功能演示
 * @details 演示 UniConv 库的核心编码转换功能和错误处理
 */

#include "../include/UniConv.h"
#include <iostream>
#include <string>

int main() {
    std::cout << "=== UniConv Basic Features Demo ===" << std::endl;
    
    // 获取 UniConv 实例
    auto converter = UniConv::Create();
    
    // 演示1: 基础编码转换
    std::cout << "\n1. Basic Encoding Conversion:" << std::endl;
    std::string input_text = "Hello, World! Test string";
    std::cout << "Input: " << input_text << std::endl;
    
    // 使用高性能转换方法
    auto result = converter->ConvertEncodingFast(input_text, "UTF-8", "UTF-16LE");
    if (result.IsSuccess()) {
        std::cout << "Success! Output size: " << result.GetValue().size() << " bytes" << std::endl;
    } else {
        std::cout << "Failed: " << result.GetErrorMessage() << std::endl;
    }
    
    // 演示2: 获取系统代码页
    std::cout << "\n2. Get System Codepage:" << std::endl;
    auto codepage_result = converter->GetSystemCodePageFast();
    if (codepage_result.IsSuccess()) {
        std::cout << "System codepage: " << codepage_result.GetValue() << std::endl;
    } else {
        std::cout << "Failed: " << codepage_result.GetErrorMessage() << std::endl;
    }
    
    // 演示3: 零分配编码名称查询
    std::cout << "\n3. Zero-Allocation Encoding Name Lookup:" << std::endl;
    int cp_utf8 = 65001;
    const char* encoding_name = converter->GetEncodingNamePtr(cp_utf8);
    if (encoding_name) {
        std::cout << "Codepage " << cp_utf8 << ": " << encoding_name << std::endl;
    } else {
        std::cout << "Not found for codepage " << cp_utf8 << std::endl;
    }
    
    // 演示4: CompactResult 编码名称查询
    std::cout << "\n4. CompactResult Encoding Name:" << std::endl;
    auto name_result = converter->GetEncodingNameFast(1252);
    if (name_result.IsSuccess()) {
        std::cout << "Codepage 1252: " << name_result.GetValue() << std::endl;
    } else {
        std::cout << "Lookup failed: " << name_result.GetErrorMessage() << std::endl;
    }
    
    // 演示5: 错误处理
    std::cout << "\n5. Error Handling:" << std::endl;
    auto error_result = converter->ConvertEncodingFast("test", "INVALID_ENCODING", "UTF-8");
    if (!error_result.IsSuccess()) {
        std::cout << "Expected error: " << error_result.GetErrorMessage() << std::endl;
        std::cout << "Error code: " << static_cast<int>(error_result.GetErrorCode()) << std::endl;
    }
    
    std::cout << "\n=== Demo Completed ===" << std::endl;
    
    return 0;
}