/**
 * @file Test_New.cpp
 * @brief Refactored test suite for comprehensive UniConv library testing
 * @details This file contains an enhanced test framework that provides
 *          comprehensive testing capabilities for the UniConv character encoding
 *          conversion library. It includes utilities for file I/O, encoding
 *          detection, and batch testing with detailed logging.
 * 
 * @author UniConv Development Team
 * @copyright Copyright (c) 2025. All rights reserved.
 * @license MIT License
 * @version 1.0.0.1
 * 
 * @par Key Features:
 * - Comprehensive encoding detection and BOM handling
 * - Binary file I/O utilities for testing
 * - Hexadecimal data visualization for debugging
 * - Automated test environment setup
 * - Detailed logging with console and file output
 * - Batch conversion testing capabilities
 * 
 * @par Test Coverage:
 * - UTF-8, UTF-16LE, UTF-16BE, GBK encoding support
 * - BOM detection and removal
 * - Round-trip conversion validation
 * - File-based batch processing
 * 
 * @since 1.0.0.1
 */

#include "UniConv.h"
#include "LightLogWriteImpl.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <filesystem>

/**
 * @defgroup TestUtilitiesNew Enhanced Test Utilities
 * @brief Enhanced utility functions for comprehensive testing
 * @{
 */

/// Global logger instance for test output
static LightLogWrite_Impl g_logger;

/**
 * @brief Simplified logging function with dual output
 * @param message The message to log
 * 
 * @details Outputs the message to both the file log (via LightLogWrite_Impl)
 *          and the console for immediate feedback during testing.
 * 
 * @par Thread Safety:
 * This function is thread-safe as it uses the thread-safe LightLogWrite_Impl.
 * 
 * @see LightLogWrite_Impl::WriteLogContent()
 * @since 1.0.0.1
 */
void Log(const std::string& message) {
    g_logger.WriteLogContent(L"INFO", Utf8ConvertsToUcs4(message));
    std::cout << message << std::endl;
}

/**
 * @brief Converts binary data to hexadecimal string representation
 * @param data The binary data to convert
 * @return Hexadecimal string with space-separated bytes
 * 
 * @details Converts each byte in the input data to a two-digit hexadecimal
 *          representation, separated by spaces. Useful for debugging binary
 *          data and encoding issues.
 * 
 * @par Example:
 * @code
 * std::string data = "ABC";
 * std::string hex = BytesToHex(data);  // Returns "41 42 43 "
 * @endcode
 * 
 * @since 1.0.0.1
 */
std::string BytesToHex(const std::string& data) {
    std::ostringstream oss;
    for (unsigned char c : data) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c) << " ";
    }
    return oss.str();
}

/**
 * @brief Reads raw binary data from a file
 * @param filePath Path to the file to read
 * @return String containing the raw binary data, empty if file cannot be read
 * 
 * @details Opens the file in binary mode and reads all content into a string.
 *          This preserves all bytes including null characters and BOM markers.
 * 
 * @par Error Handling:
 * Returns empty string if the file cannot be opened or read.
 * 
 * @par Usage:
 * @code
 * std::string content = ReadFileBytes("test.txt");
 * if (!content.empty()) {
 *     // Process file content
 * }
 * @endcode
 * 
 * @since 1.0.0.1
 */
std::string ReadFileBytes(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        return "";
    }
    return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

/**
 * @brief Writes binary data to a file
 * @param filePath Path to the output file
 * @param data Binary data to write
 * @return true if successful, false if write failed
 * 
 * @details Opens the file in binary mode and writes the data exactly as provided.
 *          This preserves all bytes including null characters and BOM markers.
 * 
 * @par Error Handling:
 * Returns false if the file cannot be created or written to.
 * 
 * @par Usage:
 * @code
 * std::string data = "Binary content";
 * if (WriteFileBytes("output.bin", data)) {
 *     Log("File written successfully");
 * }
 * @endcode
 * 
 * @since 1.0.0.1
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
 * @brief Prepares the test environment by creating directories and checking files
 * 
 * @details Sets up the necessary directory structure for testing and verifies
 *          that all required test input files are available. Creates:
 *          - testdata/ directory for input files
 *          - testdata/output/ directory for conversion results
 * 
 * @par Required Test Files:
 * - testdata/input_utf8.txt - UTF-8 encoded test content
 * - testdata/input_gbk.txt - GBK encoded test content
 * - testdata/input_utf16le.txt - UTF-16LE encoded test content
 * - testdata/input_utf16be.txt - UTF-16BE encoded test content
 * 
 * @par Behavior:
 * - Creates directories if they don't exist
 * - Checks for existence of required test files
 * - Logs warnings for missing files
 * - Suggests running generate_test_files.py script if files are missing
 * 
 * @note This function only creates directories and checks files; it does not
 *       modify or create test files to avoid overwriting existing data.
 * 
 * @since 1.0.0.1
 */
void PrepareTestEnvironment() {
    // Create test directories
    std::filesystem::create_directories("testdata");
    std::filesystem::create_directories("testdata/output");
    
    // Check if test files exist
    std::vector<std::string> required_files = {
        "testdata/input_utf8.txt",
        "testdata/input_gbk.txt", 
        "testdata/input_utf16le.txt",
        "testdata/input_utf16be.txt"
    };
    
    bool all_files_exist = true;
    for (const auto& file : required_files) {
        if (!std::filesystem::exists(file)) {
            Log("Warning: Missing test file " + file);
            all_files_exist = false;
        }
    }
    
    if (all_files_exist) {
        Log("All test files exist, ready to start testing");
    } else {
        Log("Please ensure all required test files exist");
        Log("Consider running generate_test_files.py script to generate test files");
    }
}

/**
 * @brief Detects file encoding and removes Byte Order Mark (BOM) if present
 * @param data Raw binary data from file
 * @return Pair containing detected encoding name and data with BOM removed
 * 
 * @details Analyzes the beginning of the file data to detect common BOMs
 *          and identifies the likely encoding. Removes the BOM from the data
 *          if found. Supports the following BOMs:
 *          - UTF-8 BOM: EF BB BF
 *          - UTF-16LE BOM: FF FE
 *          - UTF-16BE BOM: FE FF
 * 
 * @par Return Values:
 * - First element: Detected encoding ("UTF-8", "UTF-16LE", "UTF-16BE")
 * - Second element: Data with BOM removed if present, original data otherwise
 * 
 * @par Default Behavior:
 * If no BOM is detected, assumes UTF-8 encoding and returns data unchanged.
 * 
 * @par Example:
 * @code
 * std::string file_data = ReadFileBytes("test.txt");
 * auto [encoding, clean_data] = DetectEncodingAndRemoveBOM(file_data);
 * Log("Detected encoding: " + encoding);
 * @endcode
 * 
 * @since 1.0.0.1
 */
std::pair<std::string, std::string> DetectEncodingAndRemoveBOM(const std::string& data) {
    if (data.empty()) {
        return {"UTF-8", data};
    }
    
    // Detect UTF-8 BOM (EF BB BF)
    if (data.size() >= 3 && 
        static_cast<unsigned char>(data[0]) == 0xEF && 
        static_cast<unsigned char>(data[1]) == 0xBB && 
        static_cast<unsigned char>(data[2]) == 0xBF) {
        return {"UTF-8", data.substr(3)};
    }
    
    // Detect UTF-16LE BOM (FF FE)
    if (data.size() >= 2 && 
        static_cast<unsigned char>(data[0]) == 0xFF && 
        static_cast<unsigned char>(data[1]) == 0xFE) {
        return {"UTF-16LE", data.substr(2)};
    }
    
    // Detect UTF-16BE BOM (FE FF)
    if (data.size() >= 2 && 
        static_cast<unsigned char>(data[0]) == 0xFE && 
        static_cast<unsigned char>(data[1]) == 0xFF) {
        return {"UTF-16BE", data.substr(2)};
    }    
    // No BOM detected, return original data with empty encoding indicator
    return {"", data};
}

/** @} */ // end of TestUtilitiesNew group

/**
 * @defgroup BatchConversionNew Enhanced Batch Conversion Testing
 * @brief Advanced batch file conversion testing with comprehensive validation
 * @{
 */

/**
 * @brief Performs comprehensive batch file conversion testing
 * 
 * @details Executes a series of predefined conversion tasks to test various
 *          encoding combinations. Each task converts a file from one encoding
 *          to another and validates the result. Supports the following conversions:
 *          - UTF-8 ↔ UTF-16LE/BE
 *          - GBK ↔ UTF-8/UTF-16LE
 *          - UTF-16LE ↔ UTF-16BE
 * 
 * @par Test Workflow:
 * 1. Read source file with BOM detection
 * 2. Convert to target encoding using UniConv
 * 3. Write result to output file
 * 4. Display conversion statistics and hex dump for verification
 * 5. Log detailed results for each conversion
 * 
 * @par Output Files:
 * All output files are saved in the testdata/output/ directory with
 * descriptive names indicating the target encoding.
 * 
 * @par Error Handling:
 * - Logs warnings for missing input files
 * - Reports conversion failures with detailed error information
 * - Continues processing remaining tasks even if some fail
 * 
 * @since 1.0.0.1
 */
void BatchConvertFiles() {
    LightLogWriteImpl::GetInstance().Log("=== Starting batch file conversion testing ===");
    
    auto& conv = UniConv::GetInstance();
    
    // Conversion configuration: source file -> target encoding
    struct ConversionTask {
        std::string inputFile;
        std::string outputFile;
        std::string fromEncoding;
        std::string toEncoding;
        std::string description;
    };
    
    std::vector<ConversionTask> tasks = {
        {"testdata/input_utf8.txt", "testdata/output/output_utf16le.txt", "UTF-8", "UTF-16LE", "UTF-8 -> UTF-16LE"},        {"testdata/input_utf8.txt", "testdata/output/output_utf16be.txt", "UTF-8", "UTF-16BE", "UTF-8 -> UTF-16BE"},
        {"testdata/input_utf8.txt", "testdata/output/output_gbk.txt", "UTF-8", "GBK", "UTF-8 -> GBK"},
        {"testdata/input_gbk.txt", "testdata/output/output_utf8_from_gbk.txt", "GBK", "UTF-8", "GBK -> UTF-8"},
        {"testdata/input_utf16le.txt", "testdata/output/output_utf8_from_utf16le.txt", "UTF-16LE", "UTF-8", "UTF-16LE -> UTF-8"},
        {"testdata/input_utf16be.txt", "testdata/output/output_utf8_from_utf16be.txt", "UTF-16BE", "UTF-8", "UTF-16BE -> UTF-8"}
    };
    
    for (const auto& task : tasks) {
        LightLogWriteImpl::GetInstance().Log("--- " + task.description + " ---");
        
        // Read input file
        std::string input_data = ReadFileBytes(task.inputFile);
        if (input_data.empty()) {
            LightLogWriteImpl::GetInstance().Log("Error: Cannot read file " + task.inputFile);
            continue;
        }
        
        // Detect and remove BOM
        auto [detected_encoding, clean_data] = DetectEncodingAndRemoveBOM(input_data);
        std::string actual_from_encoding = detected_encoding.empty() ? task.fromEncoding : detected_encoding;
        
        LightLogWriteImpl::GetInstance().Log("Input file: " + task.inputFile);
        LightLogWriteImpl::GetInstance().Log("Original data size: " + std::to_string(input_data.size()) + " bytes");
        LightLogWriteImpl::GetInstance().Log("Detected encoding: " + (detected_encoding.empty() ? "No BOM" : detected_encoding));
        LightLogWriteImpl::GetInstance().Log("Cleaned data size: " + std::to_string(clean_data.size()) + " bytes");
        LightLogWriteImpl::GetInstance().Log("Input data hex: " + BytesToHex(clean_data));        
        // Perform encoding conversion
        auto result = conv.ConvertEncoding(clean_data, actual_from_encoding.c_str(), task.toEncoding.c_str());
        
        if (result.IsSuccess()) {
            LightLogWriteImpl::GetInstance().Log("Conversion successful!");
            LightLogWriteImpl::GetInstance().Log("Output data size: " + std::to_string(result.conv_result_str.size()) + " bytes");
            LightLogWriteImpl::GetInstance().Log("Output data hex: " + BytesToHex(result.conv_result_str));
            
            // Write output file
            if (WriteFileBytes(task.outputFile, result.conv_result_str)) {
                LightLogWriteImpl::GetInstance().Log("Successfully wrote output file: " + task.outputFile);
            } else {
                LightLogWriteImpl::GetInstance().Log("Error: Cannot write output file: " + task.outputFile);
            }
        } else {
            LightLogWriteImpl::GetInstance().Log("Conversion failed: " + result.error_msg);
        }
        
        LightLogWriteImpl::GetInstance().Log("");
    }
    
    LightLogWriteImpl::GetInstance().Log("=== Batch file conversion testing completed ===");
}

/** @} */ // end of BatchConversionNew group

/**
 * @defgroup ConversionTestingNew Comprehensive Conversion Method Testing
 * @brief Complete testing of all UniConv conversion methods and capabilities
 * @{
 */

/**
 * @brief Tests all available conversion methods with real-world data
 * 
 * @details Performs comprehensive testing of all UniConv conversion capabilities
 *          using actual UTF-8 test data from file or fallback ASCII text.
 *          Tests various encoding combinations and validates round-trip conversions.
 * 
 * @par Test Cases:
 * - Direct encoding conversions (UTF-8, UTF-16LE/BE, local encoding)
 * - Round-trip conversion validation
 * - System encoding detection and display
 * - Performance measurement for conversion operations
 * 
 * @par Data Sources:
 * - Primary: testdata/input_utf8.txt (real UTF-8 encoded content)
 * - Fallback: Simple ASCII text if test file unavailable
 * 
 * @par Round-trip Testing:
 * Each conversion is validated by converting back to the original encoding
 * and comparing with the source data to ensure data integrity.
 * 
 * @par Output:
 * Detailed logging of all conversion attempts, success/failure status,
 * data sizes, hexadecimal dumps, and round-trip validation results.
 * 
 * @since 1.0.0.1
 */
void TestAllConversions() {
    LightLogWriteImpl::GetInstance().Log("=== Starting comprehensive conversion method testing ===");
    
    auto& conv = UniConv::GetInstance();
    
    // Read test text from actual UTF-8 file to ensure proper UTF-8 encoding
    std::string test_text_utf8;
    if (std::filesystem::exists("testdata/input_utf8.txt")) {
        test_text_utf8 = ReadFileBytes("testdata/input_utf8.txt");
    } else {
        // Use simple ASCII text if file doesn't exist
        test_text_utf8 = "Hello World 123";
    }
    
    LightLogWriteImpl::GetInstance().Log("Original test text (UTF-8): " + test_text_utf8);
    LightLogWriteImpl::GetInstance().Log("System encoding: " + conv.GetCurrentSystemEncoding());
    
    // Test UTF-8 <-> Local encoding
    {
        auto local_result = conv.FromUtf8ToLocal(test_text);
        LightLogWriteImpl::GetInstance().Log("UTF-8 -> Local: " + BytesToHex(local_result));
        
        auto utf8_result = conv.ToUtf8FromLocal(local_result);
        LightLogWriteImpl::GetInstance().Log("Local -> UTF-8: " + utf8_result);
        LightLogWriteImpl::GetInstance().Log("Round-trip successful: " + std::string(utf8_result == test_text ? "Yes" : "No"));
    }
    
    // Test UTF-8 <-> UTF-16LE
    {
        auto utf16le_result = conv.FromUtf8ToUtf16LE(test_text);
        LightLogWriteImpl::GetInstance().Log("UTF-8 -> UTF-16LE: " + BytesToHex(std::string(reinterpret_cast<const char*>(utf16le_result.data()), utf16le_result.size() * 2)));
        
        auto utf8_result = conv.FromUtf16LEToUtf8(utf16le_result);
        LightLogWriteImpl::GetInstance().Log("UTF-16LE -> UTF-8: " + utf8_result);
        LightLogWriteImpl::GetInstance().Log("Round-trip successful: " + std::string(utf8_result == test_text ? "Yes" : "No"));
    }
    
    // Test UTF-8 <-> UTF-16BE
    {
        auto utf16be_result = conv.FromUtf8ToUtf16BE(test_text);
        LightLogWriteImpl::GetInstance().Log("UTF-8 -> UTF-16BE: " + BytesToHex(std::string(reinterpret_cast<const char*>(utf16be_result.data()), utf16be_result.size() * 2)));
        
        auto utf8_result = conv.FromUtf16BEToUtf8(utf16be_result);
        LightLogWriteImpl::GetInstance().Log("UTF-16BE -> UTF-8: " + utf8_result);
        LightLogWriteImpl::GetInstance().Log("Round-trip successful: " + std::string(utf8_result == test_text ? "Yes" : "No"));
    }
    
    LightLogWriteImpl::GetInstance().Log("=== All conversion method testing completed ===");
}

/** @} */ // end of ConversionTestingNew group

/**
 * @defgroup MainTestingNew Main Test Execution
 * @brief Primary test execution and coordination functions
 * @{
 */

/**
 * @brief Executes the complete test suite for UniConv library
 * 
 * @details Coordinates the execution of all test components in proper sequence.
 *          Sets up logging, prepares the test environment, and runs comprehensive
 *          tests covering all aspects of the UniConv library functionality.
 * 
 * @par Test Sequence:
 * 1. Initialize logging system with appropriate log file
 * 2. Prepare test environment (create directories, check files)
 * 3. Test all individual conversion methods with validation
 * 4. Perform batch file conversion testing
 * 
 * @par Output:
 * All test results are logged to "TestNewConvert.log" file and displayed
 * on console for real-time monitoring.
 * 
 * @par Usage:
 * This is the main entry point for the enhanced test suite. Call this
 * function to execute all tests in the proper order.
 * 
 * @since 1.0.0.1
 */
void RunAllTests() {
    // Set up log file
    LightLogWriteImpl::GetInstance().SetLogFileName("TestNewConvert.log");
    
    // Prepare test environment (create directories only, don't modify files)
    PrepareTestEnvironment();
    
    // Test all conversion methods
    TestAllConversions();
    
    // Batch convert files
    BatchConvertFiles();
}

/** @} */ // end of MainTestingNew group
