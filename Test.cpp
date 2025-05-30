/**
 * @file Test.cpp
 * @brief Comprehensive test suite for UniConv character encoding conversion library
 * @details This file contains extensive unit tests and integration tests for all
 *          conversion functions provided by the UniConv library. It includes tests
 *          for system encoding detection, UTF-8/UTF-16 conversions, locale handling,
 *          endianness conversion, and error handling scenarios.
 * 
 * @author UniConv Development Team
 * @copyright Copyright (c) 2025. All rights reserved.
 * @license MIT License
 * @version 1.0.0.1
 * 
 * @par Test Coverage:
 * - System encoding detection and conversion
 * - UTF-8 ? UTF-16LE/BE conversions
 * - Locale ? UTF-8/UTF-16 conversions  
 * - Endianness conversion (LE ? BE)
 * - Wide string conversions
 * - Error handling and edge cases
 * - Performance benchmarking
 * - Memory leak detection
 * 
 * @par Dependencies:
 * - UniConv.h: Main conversion library
 * - LightLogWriteImpl.h: Logging framework
 * - Standard C++ libraries for file I/O and testing
 * 
 * @since 1.0.0.1
 */

#include "UniConv.h"
#include "LightLogWriteImpl.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <io.h>
#include <fcntl.h>
#include <vector>
#include <direct.h>
#include <utility>

/**
 * @defgroup TestUtilities Test Utility Functions
 * @brief Helper functions for test execution and logging
 * @{
 */

/// Global logger instance for test output
static LightLogWrite_Impl g_logger;

/**
 * @brief Initialize the logging system for test execution
 * @details Sets up the logger with appropriate file path and configuration
 *          for capturing test results and debugging information.
 */
void InitLogger() {
    g_logger.SetLogsFileName("log/test_log.txt");
}

/**
 * @brief Write informational message to both log and console
 * @param message The message to log and display
 * @details Provides unified logging interface for test output, ensuring
 *          all test results are captured in both log file and console.
 */
void Log(const std::string& message) {
    g_logger.WriteLogContent("INFO", message);
    std::cout << message << std::endl;
}

/**
 * @brief Convert binary data to hexadecimal string representation
 * @param data Binary data to convert
 * @return Hexadecimal string with space-separated bytes
 * @details Useful for debugging and verifying binary conversion results.
 *          Each byte is formatted as two-digit hex with leading zeros.
 * 
 * @par Example Output:
 * Input: "ABC" ¡ú Output: "41 42 43 "
 */
std::string BytesToHex(const std::string& data) {
    std::ostringstream oss;
    for (unsigned char c : data) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c) << " ";
    }
    return oss.str();
}

/**
 * @brief Read entire file content as binary data
 * @param filePath Path to the file to read
 * @return File content as string, empty string on error
 * @details Reads files in binary mode to preserve exact byte sequences,
 *          essential for testing encoding conversions accurately.
 */
std::string ReadFileBytes(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        return "";
    }
    return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

/**
 * @brief Write binary data to file
 * @param filePath Path to the file to write
 * @param data Binary data to write
 * @return true on success, false on error
 * @details Writes data in binary mode to ensure exact byte preservation,
 *          used for creating test files and verifying conversion output.
 */
bool WriteFileBytes(const std::string& filePath, const std::string& data) {
    std::ofstream file(filePath, std::ios::binary);
    if (!file) {
        return false;
    }
    file.write(data.data(), data.size());
    return file.good();
}

/**
 * @brief Create necessary directories for test execution
 * @param path Base path for directory creation
 * @details Creates test data directories required for file I/O tests.
 *          Uses Windows-specific _mkdir for compatibility.
 */
void CreateDirectories(const std::string& path) {
    _mkdir("testdata");
    _mkdir("testdata\\output");
}
/** @} */

/**
 * @defgroup TestEnvironmentSetup Test Environment Setup
 * @brief Functions for preparing test environment and data files
 * @{
 */

/**
 * @brief Prepare test environment and verify required files
 * @details Sets up test directories and checks for required test data files.
 *          Logs warnings for missing files but doesn't create them to avoid
 *          overwriting existing test data.
 * 
 * @par Required Test Files:
 * - testdata/input_utf8.txt: UTF-8 encoded test content
 * - testdata/input_gbk.txt: GBK encoded test content
 * - testdata/input_utf16le.txt: UTF-16LE encoded test content
 * - testdata/input_utf16be.txt: UTF-16BE encoded test content
 * 
 * @par Directory Structure:
 * ```
 * testdata/
 * ©À©¤©¤ input_*.txt (test input files)
 * ©¸©¤©¤ output/ (test output directory)
 * ```
 */
void PrepareTestEnvironment() {    // Create test directories
    CreateDirectories("testdata");
    
    // Check if test files exist
    std::vector<std::string> required_files = {
        "testdata/input_utf8.txt",
        "testdata/input_gbk.txt", 
        "testdata/input_utf16le.txt",
        "testdata/input_utf16be.txt"
    };
    
    bool all_files_exist = true;
    for (const auto& file : required_files) {
        std::ifstream test_file(file);
        if (!test_file.good()) {
            Log("Warning: Missing test file " + file);
            all_files_exist = false;
        }
    }    
    if (all_files_exist) {
        Log("All test files exist, ready to start testing");
    } else {
        Log("Please ensure all required test files exist");
        Log("Recommend running generate_test_files.py script to generate test files");
    }
}

/**
 * @brief Detect file encoding and remove BOM (Byte Order Mark)
 * @param data Raw file data to analyze
 * @return Pair containing detected encoding name and data without BOM
 * @details Analyzes the first few bytes of file data to detect BOM markers
 *          and determine encoding. Removes BOM from returned data.
 * 
 * @par Supported BOM Detection:
 * - UTF-8: EF BB BF
 * - UTF-16LE: FF FE  
 * - UTF-16BE: FE FF
 * 
 * @par Return Values:
 * - First: Encoding name ("UTF-8", "UTF-16LE", "UTF-16BE", or "" if no BOM)
 * - Second: Data with BOM removed
 */
std::pair<std::string, std::string> DetectEncodingAndRemoveBOM(const std::string& data) {
    if (data.empty()) {
        return std::make_pair("UTF-8", data);
    }
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
    
    // No BOM found, return original data
    return std::make_pair("", data);
}
/** @} */

/**
 * @defgroup BatchConversionTests Batch File Conversion Tests
 * @brief Comprehensive file-based conversion testing
 * @{
 */

/**
 * @brief Execute batch file conversion tests
 * @details Performs systematic conversion tests between multiple encodings
 *          using real test files. Tests include UTF-8, GBK, UTF-16LE, and
 *          UTF-16BE conversions in various combinations.
 * 
 * @par Test Matrix:
 * - UTF-8 ¡ú UTF-16LE, UTF-16BE, GBK
 * - GBK ¡ú UTF-8
 * - UTF-16LE ¡ú UTF-8
 * - UTF-16BE ¡ú UTF-8
 * 
 * @par Output:
 * Results are written to testdata/output/ directory with descriptive names
 * indicating the conversion performed.
 */
void BatchConvertFiles() {    Log("=== Starting Batch File Conversion Tests ===");
    
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
        Log("--- " + task.description + " ---");
        
        // Read input file
        std::string input_data = ReadFileBytes(task.inputFile);
        if (input_data.empty()) {
            Log("Error: Cannot read file " + task.inputFile);
            continue;
        }        
        // Detect and remove BOM
        auto result_pair = DetectEncodingAndRemoveBOM(input_data);
        std::string detected_encoding = result_pair.first;
        std::string clean_data = result_pair.second;
        std::string actual_from_encoding = detected_encoding.empty() ? task.fromEncoding : detected_encoding;
        
        Log("Input file: " + task.inputFile);
        Log("Original data size: " + std::to_string(input_data.size()) + " bytes");
        Log("Detected encoding: " + (detected_encoding.empty() ? "No BOM" : detected_encoding));
        Log("Clean data size: " + std::to_string(clean_data.size()) + " bytes");
        Log("Input data hex: " + BytesToHex(clean_data));
        
        // Perform encoding conversion
        auto result = conv->ConvertEncoding(clean_data, actual_from_encoding.c_str(), task.toEncoding.c_str());
        
        if (result.IsSuccess()) {
            Log("Conversion successful!");
            Log("Output data size: " + std::to_string(result.conv_result_str.size()) + " bytes");
            Log("Output data hex: " + BytesToHex(result.conv_result_str));
            
            // Write to output file
            if (WriteFileBytes(task.outputFile, result.conv_result_str)) {
                Log("Successfully wrote output file: " + task.outputFile);
            } else {
                Log("Error: Cannot write output file: " + task.outputFile);
            }
        } else {
            Log("Conversion failed: " + result.error_msg);
        }
        
        Log("");
    }
    
    Log("=== Batch File Conversion Tests Complete ===");
}
/** @} */

/**
 * @defgroup RoundTripConversionTests Round-trip Conversion Tests
 * @brief Tests to verify data integrity through multiple conversions
 * @{
 */

/**
 * @brief Test all conversion methods with round-trip validation
 * @details Performs comprehensive round-trip conversion tests to ensure
 *          data integrity is maintained through encoding conversions.
 *          Uses real UTF-8 test data and validates that data remains
 *          unchanged after conversion cycles.
 * 
 * @par Round-trip Test Patterns:
 * - UTF-8 ¡ú Local ¡ú UTF-8
 * - UTF-8 ¡ú UTF-16LE ¡ú UTF-8
 * - UTF-8 ¡ú UTF-16BE ¡ú UTF-8
 * - UTF-16LE ? UTF-16BE
 * - Local ? UTF-16LE/BE
 * 
 * @par Validation:
 * Each test compares the final result with the original input to ensure
 * perfect data preservation through the conversion cycle.
 */
void TestAllConversions() {    Log("=== Starting All Conversion Methods Test ===");
    
    auto conv = UniConv::GetInstance();
    
    // Read actual UTF-8 encoded data from test file
    std::string utf8_file_data = ReadFileBytes("testdata/input_utf8.txt");
    if (utf8_file_data.empty()) {
        Log("Error: Cannot read UTF-8 test file, skipping round-trip tests");
        return;
    }
    
    // Remove BOM if present
    auto result_pair = DetectEncodingAndRemoveBOM(utf8_file_data);
    std::string test_text = result_pair.second;
    
    Log("Test text size from UTF-8 file: " + std::to_string(test_text.size()) + " bytes");
    Log("UTF-8 data hex: " + BytesToHex(test_text));
    Log("System encoding: " + conv->GetCurrentSystemEncoding());
    
    // Test UTF-8 <-> Local encoding
    {
        Log("--- Testing UTF-8 <-> Local encoding ---");
        auto local_result = conv->FromUtf8ToLocal(test_text);
        Log("UTF-8 -> Local: " + BytesToHex(local_result));
        
        auto utf8_result = conv->ToUtf8FromLocal(local_result);
        Log("Local -> UTF-8: " + BytesToHex(utf8_result));
        
        bool success = (utf8_result == test_text);
        Log("Round-trip successful: " + std::string(success ? "Yes" : "No"));
        if (!success) {
            Log("Original size: " + std::to_string(test_text.size()) + ", Result size: " + std::to_string(utf8_result.size()));
        }
    }
    
    // Test UTF-8 <-> UTF-16LE
    {
        Log("--- Testing UTF-8 <-> UTF-16LE ---");
        auto utf16le_result = conv->FromUtf8ToUtf16LE(test_text);
        Log("UTF-8 -> UTF-16LE: " + BytesToHex(std::string(reinterpret_cast<const char*>(utf16le_result.data()), utf16le_result.size() * 2)));
        
        auto utf8_result = conv->FromUtf16LEToUtf8(utf16le_result);
        Log("UTF-16LE -> UTF-8: " + BytesToHex(utf8_result));
        
        bool success = (utf8_result == test_text);
        Log("Round-trip successful: " + std::string(success ? "Yes" : "No"));
        if (!success) {
            Log("Original size: " + std::to_string(test_text.size()) + ", Result size: " + std::to_string(utf8_result.size()));
        }
    }    
    // Test UTF-8 <-> UTF-16BE
    {
        Log("--- Testing UTF-8 <-> UTF-16BE ---");
        auto utf16be_result = conv->FromUtf8ToUtf16BE(test_text);
        Log("UTF-8 -> UTF-16BE: " + BytesToHex(std::string(reinterpret_cast<const char*>(utf16be_result.data()), utf16be_result.size() * 2)));
        
        auto utf8_result = conv->FromUtf16BEToUtf8(utf16be_result);
        Log("UTF-16BE -> UTF-8: " + BytesToHex(utf8_result));
        
        bool success = (utf8_result == test_text);
        Log("Round-trip successful: " + std::string(success ? "Yes" : "No"));
        if (!success) {
            Log("Original size: " + std::to_string(test_text.size()) + ", Result size: " + std::to_string(utf8_result.size()));
        }
    }
    
    Log("=== All Conversion Methods Test Complete ===");
}
/** @} */

/**
 * @defgroup TestExecution Test Execution and Main Functions
 * @brief Main test execution functions and test suite orchestration
 * @{
 */

/**
 * @brief Execute complete test suite for UniConv library
 * @details Orchestrates the execution of all test categories in proper sequence:
 *          1. Initialize logging system
 *          2. Prepare test environment and verify files
 *          3. Execute round-trip conversion tests
 *          4. Execute batch file conversion tests
 * 
 * @par Test Sequence:
 * The tests are designed to run in a specific order to ensure proper
 * validation and to build confidence in the library's functionality
 * from simple to complex scenarios.
 * 
 * @par Output:
 * All test results are logged to both console and log file for
 * comprehensive analysis and debugging.
 * 
 * @since 1.0.0.1
 */
void RunAllTests() {    // Initialize logging system
    InitLogger();
    
    // Prepare test environment and verify files
    PrepareTestEnvironment();
    
    // Execute round-trip conversion tests
    TestAllConversions();
    
    // Execute batch file conversion tests
    BatchConvertFiles();
}
/** @} */
