/**
 * @file convert_tools.h
 * @brief High-level wrapper functions for character encoding conversions
 * @details This header provides simplified APIs for common encoding conversion
 *          tasks using the UniConv library. These functions serve as a convenience
 *          layer for applications that need straightforward UTF-8 ↔ wide string
 *          conversions without dealing with the complexity of the full UniConv API.
 * 
 * @author UniConv Development Team
 * @copyright Copyright (c) 2025. All rights reserved.
 * @license MIT License
 * @version 1.0.0.1
 * 
 * @par Dependencies:
 * - Standard C++ library headers for string types and conversion utilities
 * - UniConv.h for core conversion functionality
 * 
 * @par Supported Conversions:
 * - UTF-8 ↔ Wide string (platform-specific)
 * - UTF-16 → Wide string (platform-specific)
 * 
 * @since 1.0.0.1
 */

#pragma once
#include <string.h>
#include <fstream>
#include <codecvt>
#include <locale>

/**
 * @brief Convert UTF-8 string to wide string (UCS-4/UTF-32)
 * @param utf8str UTF-8 encoded input string
 * @return Wide string representation, empty on error
 * @throws std::runtime_error on conversion failure
 * 
 * @details Platform-specific conversion from UTF-8 to wide string format.
 *          Uses system locale encoding as intermediate format on Windows.
 * 
 * @see Ucs4ConvertToUtf8() for the reverse operation
 * @since 1.0.0.1
 */
std::wstring    Utf8ConvertsToUcs4(const  std::string& utf8str);

/**
 * @brief Convert wide string (UCS-4/UTF-32) to UTF-8 string
 * @param wstr Wide string input
 * @return UTF-8 encoded string, empty on error
 * @throws std::runtime_error on conversion failure
 * 
 * @details Platform-specific conversion from wide string to UTF-8 format.
 *          Uses system locale encoding as intermediate format on Windows.
 * 
 * @see Utf8ConvertsToUcs4() for the reverse operation
 * @since 1.0.0.1
 */
std::string     Ucs4ConvertToUtf8(const   std::wstring& wstr);

/**
 * @brief Convert UTF-16 string to wide string
 * @param u16str UTF-16 encoded string input
 * @return Wide string representation
 * 
 * @details Converts UTF-16 to platform-specific wide string format.
 *          Direct copy on Windows, UTF-16→UTF-32 conversion on Unix/Linux.
 * 
 * @note Assumes little-endian byte order for UTF-16 input on Unix/Linux
 * @since 1.0.0.1
 */
std::wstring    U16StringToWString(const  std::u16string& u16str);
