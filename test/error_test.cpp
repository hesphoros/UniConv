#include "UniConv.h"
#include <iostream>
#include <string>
#include <Windows.h>

int main() {
    SetConsoleOutputCP(CP_UTF8);
    
    std::cout << "=== Testing Error Handling Consistency ===" << std::endl;
    
    try {
        auto conv = UniConv::GetInstance();
        
        // Test 1: Invalid encoding handling
        std::cout << "\n1. Testing invalid encoding handling:" << std::endl;
        
        // Old API
        auto oldResult = conv->ConvertEncoding("Hello", "INVALID_FROM", "UTF-8");
        std::cout << "   Old API (ConvertEncoding): " << std::endl;
        std::cout << "   Success: " << oldResult.IsSuccess() << std::endl;
        if (!oldResult.IsSuccess()) {
            std::cout << "   Error code: " << oldResult.error_code << std::endl;
            std::cout << "   Error msg: " << oldResult.error_msg << std::endl;
        }
        
        // New API
        auto newResult = conv->ConvertEncodingFast("Hello", "INVALID_FROM", "UTF-8");
        std::cout << "   New API (ConvertEncodingFast): " << std::endl;
        std::cout << "   Success: " << newResult.IsSuccess() << std::endl;
        if (!newResult.IsSuccess()) {
            std::cout << "   Error code: " << static_cast<int>(newResult.GetErrorCode()) << std::endl;
            std::cout << "   Error msg: " << newResult.GetErrorMessage() << std::endl;
        }
        
        // Enhanced convenience method
        auto enhancedResult = conv->ToUtf8FromLocaleEx("Hello");
        std::cout << "   Enhanced API (ToUtf8FromLocaleEx): " << std::endl;
        std::cout << "   Success: " << enhancedResult.IsSuccess() << std::endl;
        if (!enhancedResult.IsSuccess()) {
            std::cout << "   Error code: " << static_cast<int>(enhancedResult.GetErrorCode()) << std::endl;
            std::cout << "   Error msg: " << enhancedResult.GetErrorMessage() << std::endl;
        }
        
        // Test 2: Empty input handling
        std::cout << "\n2. Testing empty input handling:" << std::endl;
        
        auto oldEmpty = conv->ConvertEncoding("", "UTF-8", "GBK");
        std::cout << "   Old API empty result: success=" << oldEmpty.IsSuccess() 
                  << ", length=" << oldEmpty.conv_result_str.size() << std::endl;
        
        auto newEmpty = conv->ConvertEncodingFast("", "UTF-8", "GBK");
        std::cout << "   New API empty result: success=" << newEmpty.IsSuccess() 
                  << ", length=" << newEmpty.GetValue().size() << std::endl;
        
        auto enhancedEmpty = conv->ToUtf8FromLocaleEx("");
        std::cout << "   Enhanced API empty result: success=" << enhancedEmpty.IsSuccess() 
                  << ", length=" << enhancedEmpty.GetValue().size() << std::endl;
        
        // Test 3: Valid conversion
        std::cout << "\n3. Testing valid conversion:" << std::endl;
        std::string testText = "Hello World 测试";
        
        auto oldValid = conv->ConvertEncoding(testText, "UTF-8", "UTF-8");
        std::cout << "   Old API valid: success=" << oldValid.IsSuccess() 
                  << ", result=" << oldValid.conv_result_str << std::endl;
        
        auto newValid = conv->ConvertEncodingFast(testText, "UTF-8", "UTF-8");
        std::cout << "   New API valid: success=" << newValid.IsSuccess() 
                  << ", result=" << newValid.GetValue() << std::endl;
        
        auto enhancedValid = conv->ToUtf8FromLocaleEx(testText);
        std::cout << "   Enhanced API valid: success=" << enhancedValid.IsSuccess() 
                  << ", result=" << enhancedValid.GetValue() << std::endl;
        
        std::cout << "\n=== Error Handling Consistency Test Complete ===" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "Exception caught: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cout << "Unknown exception caught" << std::endl;
        return 1;
    }
    
    system("pause");
    return 0;
}