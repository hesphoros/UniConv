#include "UniConv.h"
#include <iostream>
#include <string>
#include <Windows.h>

int main() {
    SetConsoleOutputCP(CP_UTF8);
    
    std::cout << "=== Testing Enhanced Convenience Methods ===" << std::endl;
    
    try {
        auto conv = UniConv::GetInstance();
        std::string testInput = "Hello World!";
        
        std::cout << "Input: " << testInput << std::endl;
        
        // Test basic conversion
        std::cout << "\n1. Testing ToUtf8FromLocaleEx:" << std::endl;
        auto result = conv->ToUtf8FromLocaleEx(testInput);
        
        if (result.IsSuccess()) {
            std::cout << "   Success: " << result.GetValue() << std::endl;
        } else {
            std::cout << "   Error: " << result.GetErrorMessage() << std::endl;
            std::cout << "   Error code: " << static_cast<int>(result.GetErrorCode()) << std::endl;
        }
        
        // Test empty input
        std::cout << "\n2. Testing empty input:" << std::endl;
        auto emptyResult = conv->ToUtf8FromLocaleEx("");
        if (emptyResult.IsSuccess()) {
            std::cout << "   Success: empty string (length=" << emptyResult.GetValue().size() << ")" << std::endl;
        } else {
            std::cout << "   Error: " << emptyResult.GetErrorMessage() << std::endl;
        }
        
        std::cout << "\n=== Test Complete - Success ===" << std::endl;
        
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