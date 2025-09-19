#include "../include/UniConv.h"
#include <iostream>
#include <string>

int main() {
    std::cout << "=== UniConv High-Performance Error Handling Demo ===" << std::endl;
    
    // Get UniConv instance
    auto uniconv = UniConv::UniConv::GetInstance();
    
    // Demo 1: High-performance encoding conversion
    std::cout << "\n1. High-performance encoding conversion demo:" << std::endl;
    std::string input = "Hello, World!";
    std::cout << "Input: " << input << std::endl;
    
    auto result = uniconv->ConvertEncodingFast(input, "UTF-8", "UTF-16LE");
    if (result.IsSuccess()) {
        std::cout << "Conversion successful! Output length: " << result.GetValue().size() << " bytes" << std::endl;
    } else {
        std::cout << "Conversion failed: " << result.GetErrorMessage() << std::endl;
    }
    
    // Demo 2: Fast system codepage retrieval
    std::cout << "\n2. Fast system codepage retrieval:" << std::endl;
    auto codepage_result = uniconv->GetSystemCodePageFast();
    if (codepage_result.IsSuccess()) {
        std::cout << "System codepage: " << codepage_result.GetValue() << std::endl;
    } else {
        std::cout << "Failed to get codepage: " << codepage_result.GetErrorMessage() << std::endl;
    }
    
    // Demo 3: Zero-allocation encoding name lookup
    std::cout << "\n3. Zero-allocation encoding name lookup:" << std::endl;
    int test_codepage = 65001; // UTF-8
    const char* encoding_name = uniconv->GetEncodingNamePtr(test_codepage);
    if (encoding_name) {
        std::cout << "Codepage " << test_codepage << " encoding: " << encoding_name << std::endl;
    } else {
        std::cout << "Encoding not found for codepage " << test_codepage << std::endl;
    }
    
    // Demo 4: CompactResult encoding name lookup
    std::cout << "\n4. CompactResult encoding name lookup:" << std::endl;
    auto name_result = uniconv->GetEncodingNameFast(1252); // Windows-1252
    if (name_result.IsSuccess()) {
        std::cout << "Codepage 1252 encoding: " << name_result.GetValue() << std::endl;
    } else {
        std::cout << "Lookup failed: " << name_result.GetErrorMessage() << std::endl;
    }
    
    // Demo 5: Error handling
    std::cout << "\n5. Error handling demo:" << std::endl;
    auto error_result = uniconv->ConvertEncodingFast("test", "INVALID_ENCODING", "UTF-8");
    if (!error_result.IsSuccess()) {
        std::cout << "Expected error: " << error_result.GetErrorMessage() << std::endl;
        std::cout << "Error code: " << static_cast<int>(error_result.GetErrorCode()) << std::endl;
    }
    
    std::cout << "\n=== Demo completed ===" << std::endl;
    
    return 0;
}