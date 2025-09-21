#include <iostream>
#include <string>
#include "UniConv.h"

int main() {
    std::cout << "=== FetchContent UniConv Example ===" << std::endl;
    
    try {
        // Get UniConv singleton instance
        auto uniconv = UniConv::GetInstance();
        
        // Test basic encoding conversion functionality
        std::string utf8_text = "Hello UniConv! Test text with ASCII only.";
        std::cout << "Original UTF-8 text: " << utf8_text << std::endl;
        
        // Convert to GBK using the high-performance API
        auto gbk_result = uniconv->ConvertEncodingFast(utf8_text, "UTF-8", "GBK");
        if (!gbk_result.IsSuccess()) {
            std::cout << "ERROR: Conversion to GBK failed: " << gbk_result.GetErrorMessage() << std::endl;
            return 1;
        }
        std::cout << "Converted to GBK successfully, length: " << gbk_result.GetValue().size() << " bytes" << std::endl;
        
        // Convert back to UTF-8
        auto utf8_result = uniconv->ConvertEncodingFast(gbk_result.GetValue(), "GBK", "UTF-8");
        if (!utf8_result.IsSuccess()) {
            std::cout << "ERROR: Conversion back to UTF-8 failed: " << utf8_result.GetErrorMessage() << std::endl;
            return 1;
        }
        std::cout << "Converted back to UTF-8: " << utf8_result.GetValue() << std::endl;
        
        // Verify round-trip conversion correctness
        if (utf8_text == utf8_result.GetValue()) {
            std::cout << "SUCCESS: Round-trip conversion successful!" << std::endl;
        } else {
            std::cout << "ERROR: Round-trip conversion failed" << std::endl;
            std::cout << "Original: '" << utf8_text << "'" << std::endl;
            std::cout << "Result:   '" << utf8_result.GetValue() << "'" << std::endl;
            return 1;
        }
        
        // Test system codepage detection
        auto codepage_result = uniconv->GetSystemCodePageFast();
        if (codepage_result.IsSuccess()) {
            std::cout << "System codepage detected: " << codepage_result.GetValue() << std::endl;
        } else {
            std::cout << "Warning: Could not detect system codepage: " << codepage_result.GetErrorMessage() << std::endl;
        }
        
        std::cout << std::endl << "=== FetchContent Integration Test Successful! ===" << std::endl;
        std::cout << "UniConv has been successfully integrated as a subproject via FetchContent" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}