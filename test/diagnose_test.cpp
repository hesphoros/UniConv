#include "UniConv.h"
#include <iostream>
#include <string>
#include <Windows.h>

int main() {
    SetConsoleOutputCP(CP_UTF8);
    
    std::cout << "=== Diagnosing Enhanced API Issue ===" << std::endl;
    
    try {
        auto conv = UniConv::GetInstance();
        
        // Test system encoding detection
        std::string systemEncoding = conv->GetCurrentSystemEncoding();
        std::cout << "System encoding: " << systemEncoding << std::endl;
        
        // Test different inputs
        std::vector<std::string> testCases = {
            "Hello World",              // ASCII only
            "Hello World 测试",         // Mixed ASCII + Chinese
            "测试中文字符",              // Chinese only  
            "Café résumé"               // European characters
        };
        
        for (size_t i = 0; i < testCases.size(); ++i) {
            const auto& testInput = testCases[i];
            std::cout << "\nTest case " << (i+1) << ": \"" << testInput << "\"" << std::endl;
            
            // Test enhanced convenience method
            auto enhancedResult = conv->ToUtf8FromLocaleEx(testInput);
            std::cout << "  Enhanced API: success=" << enhancedResult.IsSuccess();
            if (enhancedResult.IsSuccess()) {
                std::cout << ", result=\"" << enhancedResult.GetValue() << "\"" << std::endl;
            } else {
                std::cout << ", error=\"" << enhancedResult.GetErrorMessage() 
                         << "\", code=" << static_cast<int>(enhancedResult.GetErrorCode()) << std::endl;
            }
            
            // Test underlying ConvertEncodingFast directly
            auto directResult = conv->ConvertEncodingFast(testInput, systemEncoding.c_str(), "UTF-8");
            std::cout << "  Direct Fast API: success=" << directResult.IsSuccess();
            if (directResult.IsSuccess()) {
                std::cout << ", result=\"" << directResult.GetValue() << "\"" << std::endl;
            } else {
                std::cout << ", error=\"" << directResult.GetErrorMessage() 
                         << "\", code=" << static_cast<int>(directResult.GetErrorCode()) << std::endl;
            }
            
            // Test old API for comparison  
            auto oldResult = conv->ConvertEncoding(testInput, systemEncoding.c_str(), "UTF-8");
            std::cout << "  Old API: success=" << oldResult.IsSuccess();
            if (oldResult.IsSuccess()) {
                std::cout << ", result=\"" << oldResult.conv_result_str << "\"" << std::endl;
            } else {
                std::cout << ", error=\"" << oldResult.error_msg 
                         << "\", code=" << oldResult.error_code << std::endl;
            }
        }
        
        std::cout << "\n=== Diagnosis Complete ===" << std::endl;
        
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