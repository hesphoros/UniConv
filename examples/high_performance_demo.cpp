/**
 * @file high_performance_demo.cpp
 * @brief 高性能错误处理系统演示
 * @details 展示轻量级ErrorCode和CompactResult的使用
 */

#include "../include/UniConv.h"
#include <iostream>
#include <string>

int main() {
    std::cout << "=== UniConv High-Performance Error Handling Demo ===" << std::endl;
    
    // 获取UniConv实例
    auto uniconv = UniConv::UniConv::GetInstance();
    
    // 演示1: 高性能编码转换
    std::cout << "\n1. 高性能编码转换演示:" << std::endl;
    std::string input = "Hello, 世界! 你好世界!";
    std::cout << "输入: " << input << std::endl;
    
    auto result = uniconv->ConvertEncodingFast(input, "UTF-8", "UTF-16LE");
    if (result.IsSuccess()) {
        std::cout << "转换成功! 输出长度: " << result.GetValue().size() << " 字节" << std::endl;
    } else {
        std::cout << "转换失败: " << result.GetErrorMessage() << std::endl;
    }
    
    // 演示2: 快速系统代码页获取
    std::cout << "\n2. 快速系统代码页获取:" << std::endl;
    auto codepage_result = uniconv->GetSystemCodePageFast();
    if (codepage_result.IsSuccess()) {
        std::cout << "系统代码页: " << codepage_result.GetValue() << std::endl;
    } else {
        std::cout << "获取代码页失败: " << codepage_result.GetErrorMessage() << std::endl;
    }
    
    // 演示3: 零分配编码名查找
    std::cout << "\n3. 零分配编码名查找:" << std::endl;
    int test_codepage = 65001; // UTF-8
    const char* encoding_name = uniconv->GetEncodingNamePtr(test_codepage);
    if (encoding_name) {
        std::cout << "代码页 " << test_codepage << " 对应编码: " << encoding_name << std::endl;
    } else {
        std::cout << "未找到代码页 " << test_codepage << " 对应的编码" << std::endl;
    }
    
    // 演示4: 使用CompactResult的编码名查找
    std::cout << "\n4. CompactResult编码名查找:" << std::endl;
    auto name_result = uniconv->GetEncodingNameFast(1252); // Windows-1252
    if (name_result.IsSuccess()) {
        std::cout << "代码页 1252 对应编码: " << name_result.GetValue() << std::endl;
    } else {
        std::cout << "查找失败: " << name_result.GetErrorMessage() << std::endl;
    }
    
    // 演示5: 错误处理
    std::cout << "\n5. 错误处理演示:" << std::endl;
    auto error_result = uniconv->ConvertEncodingFast("test", "INVALID_ENCODING", "UTF-8");
    if (!error_result.IsSuccess()) {
        std::cout << "预期的错误: " << error_result.GetErrorMessage() << std::endl;
        std::cout << "错误代码: " << static_cast<int>(error_result.GetErrorCode()) << std::endl;
    }
    
    // 演示6: 使用ValueOr提供默认值
    std::cout << "\n6. 默认值处理演示:" << std::endl;
    auto invalid_name = uniconv->GetEncodingNameFast(99999); // 无效代码页
    std::string name_with_default = std::string(invalid_name.ValueOr("Unknown"));
    std::cout << "代码页 99999 编码名 (含默认值): " << name_with_default << std::endl;
    
    std::cout << "\n=== 演示完成 ===" << std::endl;
    
    return 0;
}