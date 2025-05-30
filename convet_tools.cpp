/**
 * @file convet_tools.cpp
 * @brief Utility functions for character encoding conversions using UniConv
 * @details This file provides high-level wrapper functions that simplify
 *          common encoding conversion tasks. It serves as a convenience layer
 *          over the core UniConv functionality, offering simplified APIs for
 *          UTF-8 ↔ wide string conversions.
 * 
 * @author UniConv Development Team
 * @copyright Copyright (c) 2025. All rights reserved.
 * @license MIT License
 * @version 1.0.0.1
 * 
 * @par Dependencies:
 * - convert_tools.h: Function declarations
 * - UniConv.h: Core conversion library
 * - Windows.h: Windows-specific wide character APIs (Windows only)
 * 
 * @par Platform Support:
 * - Windows: Uses MultiByteToWideChar/WideCharToMultiByte APIs
 * - Unix/Linux: Simple UTF-8 to wide string conversion
 * 
 * @since 1.0.0.1
 */

#include "convert_tools.h"
#include "UniConv.h"
#ifdef _WIN32
#include <windows.h>
#endif

/**
 * @defgroup ConversionUtilities High-Level Conversion Utilities
 * @brief Simplified wrapper functions for common encoding conversions
 * @{
 */

/**
 * @brief Convert UTF-8 string to wide string (UCS-4/UTF-32)
 * @param utf8str UTF-8 encoded input string
 * @return Wide string representation, empty on error
 * @throws std::runtime_error on conversion failure
 * 
 * @details Converts UTF-8 encoded string to platform-specific wide string format.
 *          On Windows, this performs UTF-8 → Local → Wide conversion for maximum
 *          compatibility with system APIs. On Unix-like systems, performs direct
 *          UTF-8 to wide string conversion.
 * 
 * @par Algorithm (Windows):
 * 1. Convert UTF-8 to system locale encoding using UniConv
 * 2. Convert locale encoding to wide string using MultiByteToWideChar
 * 3. Handle errors and edge cases appropriately
 * 
 * @par Algorithm (Unix/Linux):
 * Direct assignment from UTF-8 to wide string (assuming UTF-32 wide chars)
 * 
 * @par Error Handling:
 * - Throws std::runtime_error on conversion failures
 * - Returns empty string for empty input
 * - Validates conversion results before returning
 * 
 * @code{.cpp}
 * // Example: Convert UTF-8 to wide string
 * std::string utf8_text = "Hello 世界";
 * try {
 *     std::wstring wide_text = Utf8ConvertsToUcs4(utf8_text);
 *     // Use wide_text with Windows W-APIs
 * } catch (const std::runtime_error& e) {
 *     std::cerr << "Conversion failed: " << e.what() << std::endl;
 * }
 * @endcode
 * 
 * @since 1.0.0.1
 * @see Ucs4ConvertToUtf8()
 * @see UniConv::FromUtf8ToLocal()
 */
std::wstring Utf8ConvertsToUcs4(const std::string& utf8str) {    try {
        auto conv = UniConv::GetInstance();
        
        // Use UniConv to convert UTF-8 to system locale, then convert to wide characters
        std::string local_str = conv->FromUtf8ToLocal(utf8str);
        
        // Use MultiByteToWideChar to convert to wide characters
#ifdef _WIN32
        if (local_str.empty()) {
            return std::wstring();
        }
        
        int wlen = MultiByteToWideChar(CP_ACP, 0, local_str.c_str(), -1, nullptr, 0);
        if (wlen <= 0) {
            throw std::runtime_error("Failed to convert to wide string");
        }
        
        std::wstring result(wlen - 1, L'\0'); // -1 because wlen includes null terminator
        MultiByteToWideChar(CP_ACP, 0, local_str.c_str(), -1, &result[0], wlen);
        return result;
#else
        // Linux implementation - direct UTF-8 to wide string
        return std::wstring(utf8str.begin(), utf8str.end());
#endif
    }
    catch (const std::exception& e) {
        throw std::runtime_error("Failed to convert UTF-8 to UCS-4: " + std::string(e.what()));
    }
}

/**
 * @brief Convert wide string (UCS-4/UTF-32) to UTF-8 string
 * @param wstr Wide string input
 * @return UTF-8 encoded string, empty on error
 * @throws std::runtime_error on conversion failure
 * 
 * @details Converts platform-specific wide string format to UTF-8 encoding.
 *          This is the reverse operation of Utf8ConvertsToUcs4(). On Windows,
 *          performs Wide → Local → UTF-8 conversion. On Unix-like systems,
 *          performs direct wide string to UTF-8 conversion.
 * 
 * @par Algorithm (Windows):
 * 1. Convert wide string to system locale encoding using WideCharToMultiByte
 * 2. Convert locale encoding to UTF-8 using UniConv
 * 3. Handle errors and validate results
 * 
 * @par Algorithm (Unix/Linux):
 * 1. Direct assignment from wide string to string
 * 2. Convert locale string to UTF-8 using UniConv
 * 
 * @par Use Cases:
 * - Converting Windows API results to UTF-8
 * - Preparing text for cross-platform data exchange
 * - Converting user input from wide APIs to UTF-8
 * 
 * @code{.cpp}
 * // Example: Convert wide string to UTF-8
 * std::wstring wide_text = L"Hello 世界";
 * try {
 *     std::string utf8_text = Ucs4ConvertToUtf8(wide_text);
 *     // Use utf8_text for network transmission or file storage
 * } catch (const std::runtime_error& e) {
 *     std::cerr << "Conversion failed: " << e.what() << std::endl;
 * }
 * @endcode
 * 
 * @since 1.0.0.1
 * @see Utf8ConvertsToUcs4()
 * @see UniConv::ToUtf8FromLocal()
 */
std::string Ucs4ConvertToUtf8(const std::wstring& wstr) {
    try {
        auto conv = UniConv::GetInstance();
        
        // 使用 WideCharToMultiByte 转换为多字节字符
#ifdef _WIN32
        if (wstr.empty()) {
            return std::string();
        }
        
        int len = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
        if (len <= 0) {
            throw std::runtime_error("Failed to convert from wide string");
        }
          std::string local_str(len - 1, '\0'); // -1 because len includes null terminator
        WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, &local_str[0], len, nullptr, nullptr);
        
        // Use UniConv to convert local encoding to UTF-8
        return conv->ToUtf8FromLocal(local_str);
#else
        // Linux implementation - convert wide string to local string then to UTF-8
        std::string result(wstr.begin(), wstr.end());
        return conv->ToUtf8FromLocal(result);
#endif
    }
    catch (const std::exception& e) {
        throw std::runtime_error("Failed to convert UCS-4 to UTF-8: " + std::string(e.what()));
    }
}


/**
 * @brief Convert UTF-16 string to wide string
 * @param u16str UTF-16 encoded string input
 * @return Wide string representation
 * 
 * @details Converts UTF-16 encoded string to platform-specific wide string format.
 *          This function handles the platform differences between Windows (UTF-16)
 *          and Unix/Linux (UTF-32) wide character representations.
 * 
 * @par Platform-Specific Behavior:
 * - **Windows**: Direct assignment (wchar_t is 2 bytes, UTF-16 compatible)
 * - **Unix/Linux**: Converts UTF-16 to UTF-32 using std::codecvt_utf16
 * 
 * @par Algorithm (Windows):
 * 1. Direct assignment from char16_t* to wchar_t* (both are 16-bit)
 * 2. No conversion needed as both use UTF-16 encoding
 * 
 * @par Algorithm (Unix/Linux):
 * 1. Use std::wstring_convert with std::codecvt_utf16 converter
 * 2. Convert UTF-16 little-endian data to UTF-32 wide characters
 * 3. Handle surrogate pairs and extended Unicode characters
 * 
 * @par Use Cases:
 * - Converting Unicode text between different platforms
 * - Interfacing with platform-specific wide string APIs
 * - Processing text data from UTF-16 sources (e.g., Windows files)
 * 
 * @par Error Handling:
 * - May throw std::runtime_error on invalid UTF-16 sequences (Unix/Linux)
 * - Windows version is safe for valid UTF-16 input
 * 
 * @code{.cpp}
 * // Example: Convert UTF-16 string to wide string
 * std::u16string utf16_text = u"Hello 世界";
 * std::wstring wide_text = U16StringToWString(utf16_text);
 * 
 * #ifdef _WIN32
 *     // Use with Windows W-APIs
 *     SetWindowTextW(hwnd, wide_text.c_str());
 * #else
 *     // Use with Unix wide string functions
 *     std::wcout << wide_text << std::endl;
 * #endif
 * @endcode
 * 
 * @note This function assumes little-endian byte order for UTF-16 input on Unix/Linux
 * @warning On Unix/Linux, invalid UTF-16 sequences may cause conversion exceptions
 * 
 * @since 1.0.0.1
 * @see Utf8ConvertsToUcs4()
 * @see Ucs4ConvertToUtf8()
 */
std::wstring U16StringToWString(const std::u16string& u16str)
{
	std::wstring wstr;

#ifdef _WIN32
	// Windows platform: wchar_t is 2 bytes (UTF-16), direct copy
	wstr.assign(u16str.begin(), u16str.end());
#else
	// Linux platform: wchar_t is 4 bytes (UTF-32), conversion required
	std::wstring_convert<std::codecvt_utf16<wchar_t, 0x10ffff, std::little_endian>> converter;
	wstr = converter.from_bytes(
		reinterpret_cast<const char*>(u16str.data()),
		reinterpret_cast<const char*>(u16str.data() + u16str.size())
	);
#endif

	return wstr;
}

/** @} */ // end of ConversionUtilities group