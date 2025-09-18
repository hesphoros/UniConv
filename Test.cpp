#if _MSC_VER >= 1600 
#pragma execution_character_set("utf-8")
#endif

#include "UniConv.h"
#include "common.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <io.h>
#include <fcntl.h>
#include <vector>
#include <direct.h>
#include <utility>

// Helper functions for test data handling
std::string ReadFileBytes(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        return "";
    }
    return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

bool WriteFileBytes(const std::string& filePath, const std::string& data) {
    std::ofstream file(filePath, std::ios::binary);
    if (!file) {
        return false;
    }
    file.write(data.data(), data.size());
    return file.good();
}

// Convert the byte data to a hexadecimal string
std::string BytesToHex(const std::string& data) {
    std::ostringstream oss;
    for (unsigned char c : data) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c) << " ";
    }
    return oss.str();
}

// Create necessary directories
void CreateDirectories(const std::string& path) {
    _mkdir("testdata");
    _mkdir("testdata\\output");
}

// Check file encoding and remove BOM if exists
std::pair<std::string, std::string> DetectEncodingAndRemoveBOM(const std::string& data) {
    if (data.empty()) {
        return std::make_pair("UTF-8", data);
    }

    // detect bom
    if (data.size() >= 3 &&
        static_cast<unsigned char>(data[0]) == 0xEF &&
        static_cast<unsigned char>(data[1]) == 0xBB &&
        static_cast<unsigned char>(data[2]) == 0xBF) {
        return std::make_pair("UTF-8", data.substr(3));
    }

    if (data.size() >= 2 &&
        static_cast<unsigned char>(data[0]) == 0xFF &&
        static_cast<unsigned char>(data[1]) == 0xFE) {
        return std::make_pair("UTF-16LE", data.substr(2));
    }

    if (data.size() >= 2 &&
        static_cast<unsigned char>(data[0]) == 0xFE &&
        static_cast<unsigned char>(data[1]) == 0xFF) {
        return std::make_pair("UTF-16BE", data.substr(2));
    }

    return std::make_pair("", data);
}

// Generate test files
void GenerateTestFiles() {
    // Create output directory
    CreateDirectories("testdata");
    
    // Test text (Chinese)
    std::string test_text = "Test Chinese Hello World 123";
    
    LOGINFO("Starting test file generation...");
    
    // 1. Generate UTF-8 file (no BOM)
    WriteFileBytes("testdata/input_utf8.txt", test_text);
    LOGINFO("Generated UTF-8 test file: testdata/input_utf8.txt");
    
    // 2. Generate GBK/GB2312 file
    auto conv = UniConv::GetInstance();
    auto gbk_result = conv->ConvertEncoding(test_text, "UTF-8", "GBK");
    if (gbk_result.IsSuccess()) {
        WriteFileBytes("testdata/input_gbk.txt", gbk_result.conv_result_str);
        LOGOK("Generated GBK test file successfully: testdata/input_gbk.txt");
    } else {
        LOGERROR("Failed to generate GBK test file: " + gbk_result.error_msg);
    }
    
    // 3. Generate UTF-16LE file (with BOM)
    auto utf16le_result = conv->ConvertEncoding(test_text, "UTF-8", "UTF-16LE");
    if (utf16le_result.IsSuccess()) {
        std::string utf16le_with_bom = "\xFF\xFE" + utf16le_result.conv_result_str;
        WriteFileBytes("testdata/input_utf16le.txt", utf16le_with_bom);
        LOGOK("Generated UTF-16LE test file successfully: testdata/input_utf16le.txt");
    } else {
        LOGERROR("Failed to generate UTF-16LE test file: " + utf16le_result.error_msg);
    }
    
    // 4. Generate UTF-16BE file (with BOM)
    auto utf16be_result = conv->ConvertEncoding(test_text, "UTF-8", "UTF-16BE");
    if (utf16be_result.IsSuccess()) {
        std::string utf16be_with_bom = "\xFE\xFF" + utf16be_result.conv_result_str;
        WriteFileBytes("testdata/input_utf16be.txt", utf16be_with_bom);
        LOGOK("Generated UTF-16BE test file successfully: testdata/input_utf16be.txt");
    } else {
        LOGERROR("Failed to generate UTF-16BE test file: " + utf16be_result.error_msg);
    }
    
    // 5. Generate local encoding file (GB2312)
    auto local_result = conv->ConvertEncoding(test_text, "UTF-8", "GB2312");
    if (local_result.IsSuccess()) {
        WriteFileBytes("testdata/input_local.txt", local_result.conv_result_str);
        LOGOK("Generated GB2312 test file successfully: testdata/input_local.txt");
    } else {
        LOGERROR("Failed to generate GB2312 test file: " + local_result.error_msg);
    }
    
    LOGINFO("Test file generation completed");
}

// Batch file conversion test implementation
void BatchConvertFiles() {
    system("chcp 65001"); // Set console encoding to UTF-8
    system("cls"); // Clear screen

    LOGINFO("=== Starting batch file conversion test ===");
    
    auto conv = UniConv::GetInstance();
    
    // Conversion configuration: source file -> target encoding
    struct ConversionTask {
        std::string inputFile;
        std::string outputFile;
        std::string fromEncoding;
        std::string toEncoding;
        std::string description;
    };
    
    std::vector<ConversionTask> tasks = {
        {"testdata/input_utf8.txt", "testdata/output/output_utf16le.txt", "UTF-8", "UTF-16LE", "UTF-8 -> UTF-16LE"},
        {"testdata/input_utf8.txt", "testdata/output/output_utf16be.txt", "UTF-8", "UTF-16BE", "UTF-8 -> UTF-16BE"},
        {"testdata/input_utf8.txt", "testdata/output/output_gbk.txt", "UTF-8", "GBK", "UTF-8 -> GBK"},
        {"testdata/input_gbk.txt", "testdata/output/output_utf8_from_gbk.txt", "GBK", "UTF-8", "GBK -> UTF-8"},
        {"testdata/input_utf16le.txt", "testdata/output/output_utf8_from_utf16le.txt", "UTF-16LE", "UTF-8", "UTF-16LE -> UTF-8"},
        {"testdata/input_utf16be.txt", "testdata/output/output_utf8_from_utf16be.txt", "UTF-16BE", "UTF-8", "UTF-16BE -> UTF-8"}
    };
    
    for (const auto& task : tasks) {
        LOGINFO("--- " + task.description + " ---");
        
        // Read input file
        std::string input_data = ReadFileBytes(task.inputFile);
        if (input_data.empty()) {
            LOGERROR("Error: Unable to read file " + task.inputFile);
            continue;
        }
        
        // Detect and remove BOM
        auto result_pair = DetectEncodingAndRemoveBOM(input_data);
        std::string detected_encoding = result_pair.first;
        std::string clean_data = result_pair.second;
        std::string actual_from_encoding = detected_encoding.empty() ? task.fromEncoding : detected_encoding;
        
        LOGINFO("Input file: " + task.inputFile);
        LOGINFO("Original data size: " + std::to_string(input_data.size()) + " bytes");
        LOGINFO("Detected encoding: " + (detected_encoding.empty() ? "No BOM" : detected_encoding));
        LOGINFO("Clean data size: " + std::to_string(clean_data.size()) + " bytes");
        LOGDEBUG("Input data hex: " + BytesToHex(clean_data));
        
        // Execute encoding conversion
        auto result = conv->ConvertEncoding(clean_data, actual_from_encoding.c_str(), task.toEncoding.c_str());
        
        if (result.IsSuccess()) {
            LOGOK("Conversion successful");
            LOGINFO("Output data size: " + std::to_string(result.conv_result_str.size()) + " bytes");
            LOGDEBUG("Output data hex: " + BytesToHex(result.conv_result_str));
            
            // Write output file
            if (WriteFileBytes(task.outputFile, result.conv_result_str)) {
                LOGOK("Successfully wrote output file: " + task.outputFile);
            } else {
                LOGERROR("Error: Unable to write output file: " + task.outputFile);
            }
        } else {
            LOGERROR("Conversion failed: " + result.error_msg);
        }
        
        LOGINFO("");
    }
    
    LOGINFO("=== Batch file conversion test completed ===");
}

// Test all encoding conversion functions
void TestAllConversions() {
    LOGINFO("=== Starting test of all encoding conversion functions ===");
    
    auto conv = UniConv::GetInstance();
    std::string test_text = "Test text Hello World 123";
    
    LOGINFO("Original test text: " + test_text);
    LOGINFO("System encoding: " + conv->GetCurrentSystemEncoding());
    
    // Test UTF-8 <-> Local encoding
    {
        LOGINFO("--- Testing UTF-8 <-> Local encoding ---");
        auto local_result = conv->ToLocaleFromUtf8(test_text);
        LOGDEBUG("UTF-8 -> Local: " + BytesToHex(local_result));
        
        auto utf8_result = conv->ToUtf8FromLocale(local_result);
        LOGINFO("Local -> UTF-8: " + utf8_result);
        
        if (utf8_result == test_text) {
            LOGOK("Round-trip conversion successful");
        } else {
            LOGERROR("Round-trip conversion failed");
        }
    }
    
    // Test UTF-8 <-> UTF-16LE
    {
        LOGINFO("--- Testing UTF-8 <-> UTF-16LE ---");
        auto utf16le_result = conv->ToUtf16LEFromUtf8(test_text);
        LOGDEBUG("UTF-8 -> UTF-16LE: " + BytesToHex(std::string(reinterpret_cast<const char*>(utf16le_result.data()), utf16le_result.size() * 2)));
        
        auto utf8_result = conv->ToUtf8FromUtf16LE(utf16le_result);
        LOGINFO("UTF-16LE -> UTF-8: " + utf8_result);
        
        if (utf8_result == test_text) {
            LOGOK("Round-trip conversion successful");
        } else {
            LOGERROR("Round-trip conversion failed");
        }
    }
    
    // Test UTF-8 <-> UTF-16BE
    {
        LOGINFO("--- Testing UTF-8 <-> UTF-16BE ---");
        auto utf16be_result = conv->ToUtf16BEFromUtf8(test_text);
        LOGDEBUG("UTF-8 -> UTF-16BE: " + BytesToHex(std::string(reinterpret_cast<const char*>(utf16be_result.data()), utf16be_result.size() * 2)));
        
        auto utf8_result = conv->ToUtf8FromUtf16BE(utf16be_result);
        LOGINFO("UTF-16BE -> UTF-8: " + utf8_result);
        
        if (utf8_result == test_text) {
            LOGOK("Round-trip conversion successful");
        } else {
            LOGERROR("Round-trip conversion failed");
        }
    }
    
    LOGINFO("=== All encoding conversion tests completed ===");
}

// Run all test functions
void RunAllTests() {
    LOGINFO("=== Starting all tests ===");
    
    // Generate test files
    GenerateTestFiles();
    
    // Test all conversion functions
    TestAllConversions();
    
    // Batch file conversion
    BatchConvertFiles();
    
    LOGINFO("=== All tests completed ===");
}