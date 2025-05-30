/**
 * @file Logger.h
 * @brief Simple thread-safe logging utility class
 * @details This file provides a basic logging utility that outputs messages
 *          to both console and file with timestamps. It serves as a simpler
 *          alternative to the more feature-rich LightLogWriteImpl system.
 * 
 * @author UniConv Development Team
 * @copyright Copyright (c) 2025. All rights reserved.
 * @license MIT License
 * @version 1.0.0.1
 * 
 * @par Features:
 * - Thread-safe logging using mutex
 * - Dual output: console and file
 * - Automatic timestamp generation
 * - Simple static interface
 * 
 * @par Comparison with LightLogWriteImpl:
 * - Logger: Simple, synchronous, static interface
 * - LightLogWriteImpl: Advanced, asynchronous, instance-based
 * 
 * @since 1.0.0.1
 */

#pragma once
#include <iostream>
#include <string>
#include <fstream>
#include <mutex>
#include <chrono>
#include <iomanip>

/**
 * @brief Simple thread-safe logger with dual output
 * @details Static logger class that provides thread-safe logging to both
 *          console and file with automatic timestamp generation. Uses
 *          synchronous logging suitable for simple applications.
 * 
 * @par Thread Safety:
 * All operations are protected by a static mutex to ensure thread-safe
 * access from multiple threads.
 * 
 * @par Output Format:
 * All log messages are formatted as: [YYYY-MM-DD HH:MM:SS] message
 * 
 * @par File Handling:
 * - Log file: log/TestLog.log
 * - Opens in append mode
 * - Automatically created on first use
 * - Requires manual close via Close()
 * 
 * @code{.cpp}
 * // Example usage
 * Logger::Log("Application started");
 * Logger::Log("Processing file: " + filename);
 * Logger::Log("Application finished");
 * Logger::Close(); // Optional cleanup
 * @endcode
 * 
 * @since 1.0.0.1
 */
class Logger {
private:
    static std::mutex log_mutex;        ///< Mutex for thread-safe operations
    static std::ofstream log_file;      ///< Output file stream
    static bool file_initialized;      ///< File initialization flag

public:
    /**
     * @brief Log a message to console and file
     * @param message String message to log
     * @details Thread-safe logging function that outputs the message to both
     *          console (std::cout) and log file with timestamp prefix.
     * 
     * @par Thread Safety:
     * Uses std::lock_guard to ensure thread-safe operation
     * 
     * @par File Initialization:
     * - Opens log file on first call (lazy initialization)
     * - Creates log/ directory if needed (system dependent)
     * - File remains open until Close() is called
     * 
     * @par Output Format:
     * [YYYY-MM-DD HH:MM:SS] {message}
     * 
     * @par Error Handling:
     * - Console output always works (std::cout)
     * - File output only if file is successfully opened
     * - No exception throwing on file errors
     * 
     * @since 1.0.0.1
     */
    static void Log(const std::string& message) {
        std::lock_guard<std::mutex> lock(log_mutex);
        
        if (!file_initialized) {
            log_file.open("log/TestLog.log", std::ios::app);            file_initialized = true;
        }
        
        // Get current time
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        
        // Output to console
        std::cout << "[" << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") << "] " << message << std::endl;
        
        // Output to file
        if (log_file.is_open()) {
            log_file << "[" << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") << "] " << message << std::endl;
            log_file.flush();
        }
    }
    
    /**
     * @brief Close the log file and cleanup resources
     * @details Thread-safe cleanup function that closes the log file stream.
     *          Should be called before program termination to ensure all
     *          log data is properly written to disk.
     * 
     * @par Thread Safety:
     * Uses mutex lock to ensure safe file closure
     * 
     * @par Usage:
     * Optional but recommended to call before program exit
     * 
     * @since 1.0.0.1
     */
    static void Close() {
        std::lock_guard<std::mutex> lock(log_mutex);
        if (log_file.is_open()) {
            log_file.close();
        }
    }
};

/**
 * @brief Static member definitions
 * @details Required definitions for static class members.
 *          Each static member needs explicit definition outside class.
 */
std::mutex Logger::log_mutex;
std::ofstream Logger::log_file;
bool Logger::file_initialized = false;
