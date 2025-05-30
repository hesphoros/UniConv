/**
 * @file main.cpp
 * @brief Main entry point for UniConv encoding conversion library test program
 * @details This file provides the main function that runs comprehensive tests
 *          for the UniConv encoding conversion library. It demonstrates the
 *          library's capabilities and validates its functionality across
 *          different encoding formats and conversion scenarios.
 * 
 * @author UniConv Development Team
 * @copyright Copyright (c) 2025. All rights reserved.
 * @license MIT License
 * @version 1.0.0.1
 * 
 * @par Program Overview:
 * This test program validates all major functionality of the UniConv library:
 * - Character encoding detection and conversion
 * - UTF-8, UTF-16, and local encoding conversions
 * - Round-trip conversion validation
 * - Error handling and edge cases
 * - Logging and file output generation
 * 
 * @par Output Files:
 * - Log files: log/TestNewConvert.log (or timestamped files)
 * - Test output: testdata/output/ directory
 * - Console output with test progress and results
 * 
 * @par Dependencies:
 * - UniConv.h: Core encoding conversion library
 * - LightLogWriteImpl.h: Logging system for test results
 * - Test.cpp: Test implementation functions
 * 
 * @since 1.0.0.1
 */

#include "UniConv.h"
#include "LightLogWriteImpl.h"
#include <iostream>

/**
 * @brief External test function declaration
 * @details Declares the main test runner function that contains all
 *          test implementations. This function is defined in Test.cpp
 *          and contains comprehensive validation of the UniConv library.
 * 
 * @see Test.cpp for implementation details
 * @since 1.0.0.1
 */
void RunAllTests();

/**
 * @brief Main program entry point
 * @return int Program exit code (0 for success, 1 for error)
 * @details Main function that initializes and runs the UniConv library test suite.
 *          Provides a complete validation of the encoding conversion functionality
 *          with proper error handling and user feedback.
 * 
 * @par Program Flow:
 * 1. Display program header and information
 * 2. Execute comprehensive test suite via RunAllTests()
 * 3. Handle any exceptions that occur during testing
 * 4. Report test completion and output file locations
 * 5. Return appropriate exit code
 * 
 * @par Error Handling:
 * - Catches all std::exception types during test execution
 * - Displays error messages to stderr
 * - Returns non-zero exit code on failure
 * 
 * @par Output Information:
 * Informs users about generated files:
 * - Log files in log/ directory
 * - Test output files in testdata/output/ directory
 * 
 * @code{.cpp}
 * // Example execution
 * // Command line: ./UniConv.exe
 * // Expected output:
 * // UniConv Encoding Conversion Library Test Program
 * // ====================================
 * // [Test execution details...]
 * // All tests completed! Please check log files and output files.
 * // Log files: log/TestNewConvert.log
 * // Output files: testdata/output/
 * @endcode
 * 
 * @since 1.0.0.1
 * @see RunAllTests()
 */
int main() {
    std::cout << "UniConv Encoding Conversion Library Test Program" << std::endl;
    std::cout << "====================================" << std::endl;
    
    try {
        // Run comprehensive test suite
        RunAllTests();
        
        std::cout << "All tests completed! Please check log files and output files." << std::endl;
        std::cout << "Log files: log/TestNewConvert.log" << std::endl;
        std::cout << "Output files: testdata/output/" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error occurred during testing: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
