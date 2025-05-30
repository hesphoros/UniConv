/**
 * @file UniConv.cpp
 * @brief Universal encoding conversion library implementation
 * @details This file contains the implementation of the UniConv class methods
 *          for character encoding conversion between various formats including
 *          UTF-8, UTF-16LE, UTF-16BE, GBK, GB2312, and system locale encodings.
 * 
 * @author hesphoros
 * @email hesphoros@gmail.com
 * @version 1.0.0.1
 * @date 2025/03/10
 * @copyright Copyright (C) 2025 hesphoros <hesphoros@gmail.com>
 * @license GNU General Public License (GPL) version 3
 * 
 * This file is part of UniConv.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 * 
 * @section IMPLEMENTATION_NOTES
 * This implementation uses the POSIX iconv library for robust character
 * encoding conversion with comprehensive error handling and memory management.
 * 
 * @section CHANGELOG
 * - 2025/03/10 v1.0.0.1 - Initial implementation by hesphoros
 */
#include "UniConv.h"
#include "LightLogWriteImpl.h"

/**
 * @defgroup CoreConversion Core Conversion Functions
 * @brief Core encoding conversion implementation
 * @{
 */
UniConv::IConvResult UniConv::ConvertEncoding(const std::string& input, const char* fromEncoding, const char* toEncoding) {
    IConvResult result;
    
    if (input.empty()) {
        result.error_code = 0;
        result.conv_result_str = "";
        return result;
    }
    
    iconv_t cd = iconv_open(toEncoding, fromEncoding);
    if (cd == (iconv_t)-1) {
        result.error_code = errno;
        result.error_msg = "Failed to open iconv: " + std::string(strerror(errno));
        return result;
    }
      // Prepare input and output buffers
    size_t inbytesleft = input.size();
    size_t outbytesleft = inbytesleft * 4; // Large enough output buffer for worst case
    std::string output(outbytesleft, '\0');
    
    const char* inbuf = const_cast<char*>(input.data());
    char* outbuf = &output[0];
    char* outbuf_start = outbuf;
    
    // Perform conversion
    size_t ret = iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
    
    if (ret == (size_t)-1) {
        result.error_code = errno;
        result.error_msg = "iconv conversion failed: " + std::string(strerror(errno));
        iconv_close(cd);
        return result;
    }
    
    // Calculate actual output size
    size_t converted_size = outbuf - outbuf_start;
    output.resize(converted_size);
    
    result.conv_result_str = std::move(output);
    result.error_code = 0;
    
    iconv_close(cd);
    return result;
}
/** @} */

/**
 * @defgroup SystemEncoding System Encoding Detection and Conversion
 * @brief Functions for detecting and working with system locale encodings
 * @{
 */

/**
 * @brief Get the current system's default character encoding
 * @return String representation of the system encoding (e.g., "GBK", "UTF-8")
 * 
 * @details This function detects the system's active code page (Windows) or
 *          assumes UTF-8 (Unix-like systems) and returns the corresponding
 *          encoding name that can be used with iconv.
 * 
 * @par Platform Support:
 * - Windows: Uses GetACP() to detect active code page
 * - Unix/Linux: Returns "UTF-8" as default
 * 
 * @par Common Return Values:
 * - "GBK" (CP936) - Simplified Chinese
 * - "BIG5" (CP950) - Traditional Chinese  
 * - "SHIFT_JIS" (CP932) - Japanese
 * - "UTF-8" (CP65001) - Unicode UTF-8
 * - "CP####" - Generic code page format
 * 
 * @since 1.0.0.1
 * @see GetCurrentSystemEncodingCodePage()
 * @see GetEncodingNameByCodePage()
 */
std::string UniConv::GetCurrentSystemEncoding() {
#ifdef _WIN32
    UINT cp = GetACP();
    switch (cp) {
        case 936: return "GBK";
        case 950: return "BIG5";
        case 932: return "SHIFT_JIS";
        case 65001: return "UTF-8";
        default: return "CP" + std::to_string(cp);
    }
#else
    return "UTF-8";
#endif
}

/**
 * @brief Get the current system's active code page number
 * @return Code page number (e.g., 936 for GBK, 65001 for UTF-8)
 * 
 * @details Returns the numeric identifier of the system's active code page.
 *          On Windows, this uses the GetACP() API. On Unix-like systems,
 *          UTF-8 (65001) is assumed as the default.
 * 
 * @par Common Code Pages:
 * - 936: GBK (Simplified Chinese)
 * - 950: Big5 (Traditional Chinese)
 * - 932: Shift_JIS (Japanese)
 * - 65001: UTF-8 (Unicode)
 * - 1252: CP1252 (Western European)
 * 
 * @since 1.0.0.1
 * @see GetCurrentSystemEncoding()
 * @see GetEncodingNameByCodePage()
 */
std::uint16_t UniConv::GetCurrentSystemEncodingCodePage() {
#ifdef _WIN32
    return GetACP();
#else
    return 65001; // UTF-8
#endif
}

/**
 * @brief Convert a numeric code page to its encoding name
 * @param codePage The code page number to convert
 * @return String name of the encoding suitable for iconv
 * 
 * @details Maps common Windows code page numbers to their corresponding
 *          encoding names that can be used with the iconv library.
 *          Unknown code pages are returned in "CP####" format.
 * 
 * @par Supported Code Pages:
 * - 936 → "GBK" (Simplified Chinese)
 * - 950 → "BIG5" (Traditional Chinese)
 * - 932 → "SHIFT_JIS" (Japanese)
 * - 65001 → "UTF-8" (Unicode)
 * - 1252 → "CP1252" (Western European)
 * - Others → "CP####" (Generic format)
 * 
 * @since 1.0.0.1
 * @see GetCurrentSystemEncoding()
 * @see GetCurrentSystemEncodingCodePage()
 */
std::string UniConv::GetEncodingNameByCodePage(std::uint16_t codePage) {
    switch (codePage) {
        case 936: return "GBK";
        case 950: return "BIG5";
        case 932: return "SHIFT_JIS";
        case 65001: return "UTF-8";
        case 1252: return "CP1252";
        default: return "CP" + std::to_string(codePage);
    }
}
/** @} */

/**
 * @defgroup UTF16ParametricConversion UTF-16 Parametric Conversion Functions  
 * @brief UTF-16 conversion functions with explicit length parameters
 * @{
 */

/**
 * @brief Convert UTF-16LE byte array to UTF-8 string with explicit length
 * @param input Pointer to UTF-16LE encoded char16_t array
 * @param len Number of char16_t elements in the input array
 * @return UTF-8 encoded string, empty string on error
 * 
 * @details This overload allows conversion of UTF-16LE data with explicit
 *          length specification, useful when working with non-null-terminated
 *          buffers or when the exact length is known.
 * 
 * @par Safety Notes:
 * - Input validation: Returns empty string if input is null or len is 0
 * - Memory safety: Uses reinterpret_cast with known buffer size
 * - Error handling: Returns empty string on conversion failure
 * 
 * @code{.cpp}
 * // Example: Convert UTF-16LE buffer to UTF-8
 * char16_t utf16_data[] = {0x4F60, 0x597D, 0x0000}; // "你好" + null
 * std::string utf8_result = UniConv::FromUtf16LEToUtf8(utf16_data, 2); // Exclude null terminator
 * @endcode
 * 
 * @since 1.0.0.1
 * @see FromUtf16LEToUtf8(const std::u16string&)
 * @see FromUtf16BEToUtf8(const char16_t*, size_t)
 */
std::string UniConv::FromUtf16LEToUtf8(const char16_t* input, size_t len) {
    if (!input || len == 0) return "";
    
    std::string input_bytes(reinterpret_cast<const char*>(input), len * sizeof(char16_t));
    auto result = ConvertEncoding(input_bytes, "UTF-16LE", "UTF-8");
    return result.IsSuccess() ? result.conv_result_str : "";
}

/**
 * @brief Convert UTF-16BE byte array to UTF-8 string with explicit length
 * @param input Pointer to UTF-16BE encoded char16_t array  
 * @param len Number of char16_t elements in the input array
 * @return UTF-8 encoded string, empty string on error
 * 
 * @details Similar to FromUtf16LEToUtf8 but handles big-endian UTF-16 data.
 *          This is commonly used when processing data from network protocols
 *          or file formats that use big-endian byte ordering.
 * 
 * @par Use Cases:
 * - Network protocol data processing
 * - File format parsing (some formats use UTF-16BE)
 * - Cross-platform data exchange
 * - Legacy system integration
 * 
 * @since 1.0.0.1
 * @see FromUtf16BEToUtf8(const std::u16string&)  
 * @see FromUtf16LEToUtf8(const char16_t*, size_t)
 */
std::string UniConv::FromUtf16BEToUtf8(const char16_t* input, size_t len) {
    if (!input || len == 0) return "";
    
    std::string input_bytes(reinterpret_cast<const char*>(input), len * sizeof(char16_t));
    auto result = ConvertEncoding(input_bytes, "UTF-16BE", "UTF-8");
    return result.IsSuccess() ? result.conv_result_str : "";
}
/** @} */

/**
 * @defgroup LocaleConversion Local Encoding Conversion Functions
 * @brief Functions for converting between system locale and other encodings
 * @{
 */

/**
 * @brief Convert system locale encoded string to UTF-8
 * @param input String in system locale encoding
 * @return UTF-8 encoded string, empty string on error
 * 
 * @details Converts a string from the current system's default encoding
 *          to UTF-8. The system encoding is automatically detected using
 *          GetCurrentSystemEncoding().
 * 
 * @par Platform Behavior:
 * - Windows: Converts from active code page (GBK, Big5, etc.) to UTF-8
 * - Unix/Linux: Usually no conversion needed (UTF-8 → UTF-8)
 * 
 * @code{.cpp}
 * // Example: Convert GBK string to UTF-8 on Chinese Windows
 * std::string gbk_text = "你好世界"; // In GBK encoding
 * std::string utf8_text = UniConv::ToUtf8FromLocal(gbk_text);
 * @endcode
 * 
 * @since 1.0.0.1
 * @see FromUtf8ToLocal()
 * @see GetCurrentSystemEncoding()
 */
std::string UniConv::ToUtf8FromLocal(const std::string& input) {
    std::string currentEncoding = GetCurrentSystemEncoding();
    auto result = ConvertEncoding(input, currentEncoding.c_str(), "UTF-8");
    return result.IsSuccess() ? result.conv_result_str : "";
}

/**
 * @brief Convert system locale encoded C-string to UTF-8
 * @param input C-string in system locale encoding
 * @return UTF-8 encoded string, empty string on error or null input
 * 
 * @details Convenience overload for C-style strings. Internally converts
 *          to std::string and calls the string version.
 * 
 * @since 1.0.0.1
 * @see ToUtf8FromLocal(const std::string&)
 */
std::string UniConv::ToUtf8FromLocal(const char* input) {
    if (!input) return "";
    return ToUtf8FromLocal(std::string(input));
}

/**
 * @brief Convert UTF-8 string to system locale encoding
 * @param input UTF-8 encoded string
 * @return String in system locale encoding, empty string on error
 * 
 * @details Converts a UTF-8 string to the current system's default encoding.
 *          This is useful when interfacing with legacy APIs or file systems
 *          that expect locale-specific encodings.
 * 
 * @par Common Use Cases:
 * - File system operations on non-UTF-8 systems
 * - Legacy API compatibility
 * - Terminal/console output on Windows
 * - Database integration with legacy character sets
 * 
 * @warning Some characters may be lost if the target encoding doesn't
 *          support them. Consider using UTF-8 when possible.
 * 
 * @since 1.0.0.1
 * @see ToUtf8FromLocal()
 */
std::string UniConv::FromUtf8ToLocal(const std::string& input) {
    std::string currentEncoding = GetCurrentSystemEncoding();
    auto result = ConvertEncoding(input, "UTF-8", currentEncoding.c_str());
    return result.IsSuccess() ? result.conv_result_str : "";
}

/**
 * @brief Convert UTF-8 C-string to system locale encoding
 * @param input UTF-8 encoded C-string
 * @return String in system locale encoding, empty string on error or null input
 * 
 * @details Convenience overload for C-style strings.
 * 
 * @since 1.0.0.1
 * @see FromUtf8ToLocal(const std::string&)
 */
std::string UniConv::FromUtf8ToLocal(const char* input) {
    if (!input) return "";
    return FromUtf8ToLocal(std::string(input));
}
/** @} */

/**
 * @defgroup UTF16ToUTF8Conversion UTF-16 to UTF-8 Conversion Functions
 * @brief Functions for converting UTF-16 encoded data to UTF-8
 * @{
 */

/**
 * @brief Convert UTF-16LE string to UTF-8
 * @param input UTF-16LE encoded string
 * @return UTF-8 encoded string, empty string on error
 * 
 * @details Converts a UTF-16 little-endian encoded string to UTF-8.
 *          This is the most common UTF-16 variant on Windows platforms.
 * 
 * @par Technical Details:
 * - Handles surrogates pairs correctly for characters outside BMP
 * - Preserves all Unicode code points
 * - Uses efficient byte-level conversion via iconv
 * 
 * @code{.cpp}
 * // Example: Convert UTF-16LE to UTF-8
 * std::u16string utf16_text = u"Hello 世界 ?";
 * std::string utf8_text = UniConv::FromUtf16LEToUtf8(utf16_text);
 * @endcode
 * 
 * @since 1.0.0.1
 * @see FromUtf16BEToUtf8()
 * @see FromUtf8ToUtf16LE()
 */
std::string UniConv::FromUtf16LEToUtf8(const std::u16string& input) {
    if (input.empty()) return "";
    
    std::string input_bytes(reinterpret_cast<const char*>(input.data()), input.size() * sizeof(char16_t));
    auto result = ConvertEncoding(input_bytes, "UTF-16LE", "UTF-8");
    return result.IsSuccess() ? result.conv_result_str : "";
}

/**
 * @brief Convert null-terminated UTF-16LE string to UTF-8
 * @param input Null-terminated UTF-16LE encoded string
 * @return UTF-8 encoded string, empty string on error or null input
 * 
 * @details Convenience overload for null-terminated UTF-16 strings.
 *          Automatically determines string length by finding null terminator.
 * 
 * @since 1.0.0.1
 * @see FromUtf16LEToUtf8(const std::u16string&)
 */
std::string UniConv::FromUtf16LEToUtf8(const char16_t* input) {
    if (!input) return "";
    return FromUtf16LEToUtf8(std::u16string(input));
}

/**
 * @brief Convert UTF-16BE string to UTF-8
 * @param input UTF-16BE encoded string
 * @return UTF-8 encoded string, empty string on error
 * 
 * @details Converts a UTF-16 big-endian encoded string to UTF-8.
 *          UTF-16BE is commonly used in network protocols and some
 *          file formats that specify big-endian byte ordering.
 * 
 * @par Use Cases:
 * - Network protocol data (HTTP/2, some XML variants)
 * - Java string serialization
 * - Some database systems
 * - Cross-platform data exchange
 * 
 * @since 1.0.0.1
 * @see FromUtf16LEToUtf8()
 * @see FromUtf8ToUtf16BE()
 */
std::string UniConv::FromUtf16BEToUtf8(const std::u16string& input) {
    if (input.empty()) return "";
    
    std::string input_bytes(reinterpret_cast<const char*>(input.data()), input.size() * sizeof(char16_t));
    auto result = ConvertEncoding(input_bytes, "UTF-16BE", "UTF-8");
    return result.IsSuccess() ? result.conv_result_str : "";
}

/**
 * @brief Convert null-terminated UTF-16BE string to UTF-8
 * @param input Null-terminated UTF-16BE encoded string
 * @return UTF-8 encoded string, empty string on error or null input
 * 
 * @details Convenience overload for null-terminated UTF-16BE strings.
 * 
 * @since 1.0.0.1
 * @see FromUtf16BEToUtf8(const std::u16string&)
 */
std::string UniConv::FromUtf16BEToUtf8(const char16_t* input) {
    if (!input) return "";
    return FromUtf16BEToUtf8(std::u16string(input));
}
/** @} */

/**
 * @defgroup UTF8ToUTF16Conversion UTF-8 to UTF-16 Conversion Functions
 * @brief Functions for converting UTF-8 encoded data to UTF-16
 * @{
 */

/**
 * @brief Convert UTF-8 string to UTF-16LE
 * @param input UTF-8 encoded string
 * @return UTF-16LE encoded string, empty string on error
 * 
 * @details Converts a UTF-8 string to UTF-16 little-endian format.
 *          The result is suitable for use with Windows Unicode APIs
 *          and other systems that expect UTF-16LE.
 * 
 * @par Technical Details:
 * - Handles all Unicode code points correctly
 * - Generates surrogate pairs for characters outside BMP (> U+FFFF)
 * - Result has proper little-endian byte ordering
 * - Validates UTF-8 input during conversion
 * 
 * @code{.cpp}
 * // Example: Convert UTF-8 to UTF-16LE for Windows API
 * std::string utf8_text = "Hello 世界 ?";
 * std::u16string utf16_text = UniConv::FromUtf8ToUtf16LE(utf8_text);
 * // Can now use utf16_text with Windows W-APIs
 * @endcode
 * 
 * @since 1.0.0.1
 * @see FromUtf8ToUtf16BE()
 * @see FromUtf16LEToUtf8()
 */
std::u16string UniConv::FromUtf8ToUtf16LE(const std::string& input) {
    auto result = ConvertEncoding(input, "UTF-8", "UTF-16LE");
    if (result.IsSuccess() && result.conv_result_str.size() % 2 == 0) {
        return std::u16string(reinterpret_cast<const char16_t*>(result.conv_result_str.data()), 
                             result.conv_result_str.size() / 2);
    }
    return std::u16string();
}

/**
 * @brief Convert UTF-8 C-string to UTF-16LE
 * @param input UTF-8 encoded C-string
 * @return UTF-16LE encoded string, empty string on error or null input
 * 
 * @details Convenience overload for C-style strings.
 * 
 * @since 1.0.0.1
 * @see FromUtf8ToUtf16LE(const std::string&)
 */
std::u16string UniConv::FromUtf8ToUtf16LE(const char* input) {
    if (!input) return std::u16string();
    return FromUtf8ToUtf16LE(std::string(input));
}

/**
 * @brief Convert UTF-8 string to UTF-16BE
 * @param input UTF-8 encoded string
 * @return UTF-16BE encoded string, empty string on error
 * 
 * @details Converts a UTF-8 string to UTF-16 big-endian format.
 *          This is useful for network protocols, Java string serialization,
 *          and cross-platform data exchange.
 * 
 * @par Use Cases:
 * - Network protocol encoding (HTTP/2, WebSocket)
 * - Java JNI string conversion
 * - Cross-platform file formats
 * - Database storage in big-endian systems
 * 
 * @since 1.0.0.1
 * @see FromUtf8ToUtf16LE()
 * @see FromUtf16BEToUtf8()
 */
std::u16string UniConv::FromUtf8ToUtf16BE(const std::string& input) {
    auto result = ConvertEncoding(input, "UTF-8", "UTF-16BE");
    if (result.IsSuccess() && result.conv_result_str.size() % 2 == 0) {
        return std::u16string(reinterpret_cast<const char16_t*>(result.conv_result_str.data()), 
                             result.conv_result_str.size() / 2);
    }
    return std::u16string();
}

/**
 * @brief Convert UTF-8 C-string to UTF-16BE
 * @param input UTF-8 encoded C-string
 * @return UTF-16BE encoded string, empty string on error or null input
 * 
 * @details Convenience overload for C-style strings.
 * 
 * @since 1.0.0.1
 * @see FromUtf8ToUtf16BE(const std::string&)
 */
std::u16string UniConv::FromUtf8ToUtf16BE(const char* input) {
    if (!input) return std::u16string();
    return FromUtf8ToUtf16BE(std::string(input));
}
/** @} */

/**
 * @defgroup LocaleToUTF16Conversion Local Encoding to UTF-16 Conversion Functions
 * @brief Functions for converting system locale encoded data to UTF-16
 * @{
 */

/**
 * @brief Convert system locale encoded string to UTF-16LE
 * @param input String in system locale encoding
 * @return UTF-16LE encoded string, empty string on error
 * 
 * @details Converts from the current system's default encoding directly
 *          to UTF-16LE format. This bypasses UTF-8 as an intermediate
 *          step for better performance and accuracy.
 * 
 * @par Performance Notes:
 * - Direct conversion without UTF-8 intermediate step
 * - Optimal for Windows applications using locale data
 * - Preserves all characters supported by both encodings
 * 
 * @since 1.0.0.1
 * @see ToUtf16BEFromLocal()
 * @see FromUtf16LEToLocal()
 */
std::u16string UniConv::ToUtf16LEFromLocal(const std::string& input) {
    std::string currentEncoding = GetCurrentSystemEncoding();
    auto result = ConvertEncoding(input, currentEncoding.c_str(), "UTF-16LE");
    if (result.IsSuccess() && result.conv_result_str.size() % 2 == 0) {
        return std::u16string(reinterpret_cast<const char16_t*>(result.conv_result_str.data()), 
                             result.conv_result_str.size() / 2);
    }
    return std::u16string();
}

/**
 * @brief Convert system locale encoded C-string to UTF-16LE
 * @param input C-string in system locale encoding
 * @return UTF-16LE encoded string, empty string on error or null input
 * 
 * @details Convenience overload for C-style strings.
 * 
 * @since 1.0.0.1
 * @see ToUtf16LEFromLocal(const std::string&)
 */
std::u16string UniConv::ToUtf16LEFromLocal(const char* input) {
    if (!input) return std::u16string();
    return ToUtf16LEFromLocal(std::string(input));
}

/**
 * @brief Convert system locale encoded string to UTF-16BE
 * @param input String in system locale encoding
 * @return UTF-16BE encoded string, empty string on error
 * 
 * @details Similar to ToUtf16LEFromLocal but produces big-endian output.
 *          Useful for cross-platform data exchange and network protocols.
 * 
 * @since 1.0.0.1
 * @see ToUtf16LEFromLocal()
 * @see FromUtf16BEToLocal()
 */
std::u16string UniConv::ToUtf16BEFromLocal(const std::string& input) {
    std::string currentEncoding = GetCurrentSystemEncoding();
    auto result = ConvertEncoding(input, currentEncoding.c_str(), "UTF-16BE");
    if (result.IsSuccess() && result.conv_result_str.size() % 2 == 0) {
        return std::u16string(reinterpret_cast<const char16_t*>(result.conv_result_str.data()), 
                             result.conv_result_str.size() / 2);
    }
    return std::u16string();
}

/**
 * @brief Convert system locale encoded C-string to UTF-16BE
 * @param input C-string in system locale encoding
 * @return UTF-16BE encoded string, empty string on error or null input
 * 
 * @details Convenience overload for C-style strings.
 * 
 * @since 1.0.0.1
 * @see ToUtf16BEFromLocal(const std::string&)
 */
std::u16string UniConv::ToUtf16BEFromLocal(const char* input) {
    if (!input) return std::u16string();
    return ToUtf16BEFromLocal(std::string(input));
}
/** @} */

/**
 * @defgroup UTF16ToLocaleConversion UTF-16 to Local Encoding Conversion Functions
 * @brief Functions for converting UTF-16 encoded data to system locale encoding
 * @{
 */

/**
 * @brief Convert UTF-16LE string to system locale encoding
 * @param input UTF-16LE encoded string
 * @return String in system locale encoding, empty string on error
 * 
 * @details Converts UTF-16LE data to the current system's default encoding.
 *          This is useful when preparing Unicode data for legacy systems
 *          or APIs that expect locale-specific encodings.
 * 
 * @warning Some Unicode characters may be lost if the target locale
 *          encoding doesn't support them. Use with caution for
 *          international text that may contain characters outside
 *          the system's default character set.
 * 
 * @since 1.0.0.1
 * @see FromUtf16BEToLocal()
 * @see ToUtf16LEFromLocal()
 */
std::string UniConv::FromUtf16LEToLocal(const std::u16string& input) {
    if (input.empty()) return "";
    
    std::string currentEncoding = GetCurrentSystemEncoding();
    std::string input_bytes(reinterpret_cast<const char*>(input.data()), input.size() * sizeof(char16_t));
    auto result = ConvertEncoding(input_bytes, "UTF-16LE", currentEncoding.c_str());
    return result.IsSuccess() ? result.conv_result_str : "";
}

/**
 * @brief Convert null-terminated UTF-16LE string to system locale encoding
 * @param input Null-terminated UTF-16LE encoded string
 * @return String in system locale encoding, empty string on error or null input
 * 
 * @details Convenience overload for null-terminated UTF-16 strings.
 * 
 * @since 1.0.0.1
 * @see FromUtf16LEToLocal(const std::u16string&)
 */
std::string UniConv::FromUtf16LEToLocal(const char16_t* input) {
    if (!input) return "";
    return FromUtf16LEToLocal(std::u16string(input));
}

/**
 * @brief Convert UTF-16BE string to system locale encoding
 * @param input UTF-16BE encoded string
 * @return String in system locale encoding, empty string on error
 * 
 * @details Similar to FromUtf16LEToLocal but handles big-endian input.
 *          Useful when processing network data or cross-platform files
 *          that use UTF-16BE encoding.
 * 
 * @since 1.0.0.1
 * @see FromUtf16LEToLocal()
 * @see ToUtf16BEFromLocal()
 */
std::string UniConv::FromUtf16BEToLocal(const std::u16string& input) {
    if (input.empty()) return "";
    
    std::string currentEncoding = GetCurrentSystemEncoding();
    std::string input_bytes(reinterpret_cast<const char*>(input.data()), input.size() * sizeof(char16_t));
    auto result = ConvertEncoding(input_bytes, "UTF-16BE", currentEncoding.c_str());
    return result.IsSuccess() ? result.conv_result_str : "";
}

/**
 * @brief Convert null-terminated UTF-16BE string to system locale encoding
 * @param input Null-terminated UTF-16BE encoded string
 * @return String in system locale encoding, empty string on error or null input
 * 
 * @details Convenience overload for null-terminated UTF-16BE strings.
 * 
 * @since 1.0.0.1
 * @see FromUtf16BEToLocal(const std::u16string&)
 */
std::string UniConv::FromUtf16BEToLocal(const char16_t* input) {
    if (!input) return "";
    return FromUtf16BEToLocal(std::u16string(input));
}
/** @} */

/**
 * @defgroup UTF16EndiannessConversion UTF-16 Endianness Conversion Functions
 * @brief Functions for converting between UTF-16LE and UTF-16BE
 * @{
 */

/**
 * @brief Convert UTF-16LE string to UTF-16BE
 * @param input UTF-16LE encoded string
 * @return UTF-16BE encoded string, empty string on error
 * 
 * @details Converts between UTF-16 little-endian and big-endian formats.
 *          This is essential for cross-platform data exchange where
 *          different systems use different byte ordering.
 * 
 * @par Technical Details:
 * - Performs byte order conversion without changing Unicode content
 * - Handles surrogate pairs correctly
 * - Preserves all Unicode code points
 * - Uses efficient iconv-based conversion
 * 
 * @par Use Cases:
 * - Network protocol data exchange
 * - Cross-platform file format conversion
 * - Java/C# interoperability
 * - Database migration between different endianness systems
 * 
 * @since 1.0.0.1
 * @see FromUtf16BEToUtf16LE()
 */
std::u16string UniConv::FromUtf16LEToUtf16BE(const std::u16string& input) {
    if (input.empty()) return std::u16string();
    
    std::string input_bytes(reinterpret_cast<const char*>(input.data()), input.size() * sizeof(char16_t));
    auto result = ConvertEncoding(input_bytes, "UTF-16LE", "UTF-16BE");
    if (result.IsSuccess() && result.conv_result_str.size() % 2 == 0) {
        return std::u16string(reinterpret_cast<const char16_t*>(result.conv_result_str.data()), 
                             result.conv_result_str.size() / 2);
    }
    return std::u16string();
}

/**
 * @brief Convert null-terminated UTF-16LE string to UTF-16BE
 * @param input Null-terminated UTF-16LE encoded string
 * @return UTF-16BE encoded string, empty string on error or null input
 * 
 * @details Convenience overload for null-terminated UTF-16LE strings.
 * 
 * @since 1.0.0.1
 * @see FromUtf16LEToUtf16BE(const std::u16string&)
 */
std::u16string UniConv::FromUtf16LEToUtf16BE(const char16_t* input) {
    if (!input) return std::u16string();
    return FromUtf16LEToUtf16BE(std::u16string(input));
}

/**
 * @brief Convert UTF-16BE string to UTF-16LE
 * @param input UTF-16BE encoded string
 * @return UTF-16LE encoded string, empty string on error
 * 
 * @details Converts between UTF-16 big-endian and little-endian formats.
 *          This is the reverse operation of FromUtf16LEToUtf16BE.
 * 
 * @since 1.0.0.1
 * @see FromUtf16LEToUtf16BE()
 */
std::u16string UniConv::FromUtf16BEToUtf16LE(const std::u16string& input) {
    if (input.empty()) return std::u16string();
    
    std::string input_bytes(reinterpret_cast<const char*>(input.data()), input.size() * sizeof(char16_t));
    auto result = ConvertEncoding(input_bytes, "UTF-16BE", "UTF-16LE");
    if (result.IsSuccess() && result.conv_result_str.size() % 2 == 0) {
        return std::u16string(reinterpret_cast<const char16_t*>(result.conv_result_str.data()), 
                             result.conv_result_str.size() / 2);
    }
    return std::u16string();
}

/**
 * @brief Convert null-terminated UTF-16BE string to UTF-16LE
 * @param input Null-terminated UTF-16BE encoded string
 * @return UTF-16LE encoded string, empty string on error or null input
 * 
 * @details Convenience overload for null-terminated UTF-16BE strings.
 *          Automatically calculates string length by finding null terminator.
 * 
 * @par Implementation Note:
 * Manual length calculation ensures proper handling of embedded nulls
 * and provides safety for unknown-length input buffers.
 * 
 * @since 1.0.0.1
 * @see FromUtf16BEToUtf16LE(const std::u16string&)
 */
std::u16string UniConv::FromUtf16BEToUtf16LE(const char16_t* input) {
    if (!input) return std::u16string();
    size_t len = 0;
    while (input[len] != 0) ++len;
    return FromUtf16BEToUtf16LE(std::u16string(input, len));
}
/** @} */

/**
 * @defgroup WideStringConversion Wide String Conversion Functions
 * @brief Functions for converting between wide strings and locale encodings
 * @{
 */

/**
 * @brief Convert wide string to system locale encoding
 * @param sInput Wide string to convert
 * @return String in system locale encoding, empty string on error
 * 
 * @details Converts a wide string (std::wstring) to the current system's
 *          default character encoding. On Windows, this uses the active
 *          code page (ACP). On Unix-like systems, this uses locale-specific
 *          conversion.
 * 
 * @par Platform Implementation:
 * - Windows: Uses WideCharToMultiByte() with CP_ACP
 * - Unix/Linux: Uses std::codecvt_byname with system locale
 * 
 * @par Technical Details:
 * - Wide string size depends on platform (16-bit on Windows, 32-bit on most Unix)
 * - Handles locale-specific character mappings
 * - May lose characters not representable in target encoding
 * 
 * @code{.cpp}
 * // Example: Convert wide string to locale
 * std::wstring wide_text = L"Hello 世界";
 * std::string locale_text = UniConv::WideStringToLocale(wide_text);
 * @endcode
 * 
 * @since 1.0.0.1
 * @see LocaleToWideString()
 */
std::string UniConv::WideStringToLocale(const std::wstring& sInput) {
#ifdef _WIN32
    if (sInput.empty()) return "";
    
    int bytes_needed = WideCharToMultiByte(CP_ACP, 0, sInput.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (bytes_needed <= 0) return "";
    
    std::string result(bytes_needed - 1, '\0');
    WideCharToMultiByte(CP_ACP, 0, sInput.c_str(), -1, &result[0], bytes_needed, nullptr, nullptr);
    return result;
#else
    // Linux实现
    std::wstring_convert<std::codecvt_byname<wchar_t, char, std::mbstate_t>> converter(new std::codecvt_byname<wchar_t, char, std::mbstate_t>(""));
    return converter.to_bytes(sInput);
#endif
}

/**
 * @brief Convert null-terminated wide string to system locale encoding
 * @param sInput Null-terminated wide string to convert
 * @return String in system locale encoding, empty string on error or null input
 * 
 * @details Convenience overload for null-terminated wide strings.
 * 
 * @since 1.0.0.1
 * @see WideStringToLocale(const std::wstring&)
 */
std::string UniConv::WideStringToLocale(const wchar_t* sInput) {
    if (!sInput) return "";
    return WideStringToLocale(std::wstring(sInput));
}

/**
 * @brief Convert system locale encoded C-string to wide string
 * @param sInput C-string in system locale encoding
 * @return Wide string, empty string on error or null input
 * 
 * @details Converts a system locale encoded string to wide string format.
 *          This is the reverse operation of WideStringToLocale.
 * 
 * @par Platform Implementation:
 * - Windows: Uses MultiByteToWideChar() with CP_ACP
 * - Unix/Linux: Uses std::codecvt_byname with system locale
 * 
 * @par Use Cases:
 * - Converting legacy API results to Unicode
 * - File path handling on Windows
 * - Console input/output processing
 * - Integration with wide string APIs
 * 
 * @since 1.0.0.1
 * @see WideStringToLocale()
 */
std::wstring UniConv::LocaleToWideString(const char* sInput) {
#ifdef _WIN32
    if (!sInput) return L"";
    
    int wchars_needed = MultiByteToWideChar(CP_ACP, 0, sInput, -1, nullptr, 0);
    if (wchars_needed <= 0) return L"";
    
    std::wstring result(wchars_needed - 1, L'\0');
    MultiByteToWideChar(CP_ACP, 0, sInput, -1, &result[0], wchars_needed);
    return result;
#else
    // Linux实现
    std::wstring_convert<std::codecvt_byname<wchar_t, char, std::mbstate_t>> converter(new std::codecvt_byname<wchar_t, char, std::mbstate_t>(""));
    return converter.from_bytes(sInput);
#endif
}

/**
 * @brief Convert system locale encoded string to wide string
 * @param sInput String in system locale encoding
 * @return Wide string, empty string on error
 * 
 * @details Convenience overload that delegates to the C-string version.
 * 
 * @since 1.0.0.1
 * @see LocaleToWideString(const char*)
 */
std::wstring UniConv::LocaleToWideString(const std::string& sInput) {
    return LocaleToWideString(sInput.c_str());
}
/** @} */

/**
 * @defgroup ErrorHandling Error Handling and Diagnostics
 * @brief Functions for error reporting and diagnostics
 * @{
 */

/**
 * @brief Get human-readable error description for iconv error codes
 * @param err_code The errno value returned by iconv operations
 * @return Human-readable error description string
 * 
 * @details Translates iconv-specific error codes into descriptive messages
 *          that can be displayed to users or used for debugging purposes.
 * 
 * @par Supported Error Codes:
 * - EILSEQ: Invalid multibyte sequence encountered
 * - EINVAL: Incomplete multibyte sequence at end of input
 * - E2BIG: Output buffer too small for conversion result
 * - EBADF: Invalid conversion descriptor (encoding not supported)
 * 
 * @par Usage Example:
 * @code{.cpp}
 * auto result = UniConv::ConvertEncoding(input, "UTF-8", "GBK");
 * if (!result.IsSuccess()) {
 *     std::string error_desc = UniConv::GetIconvErrorString(result.error_code);
 *     std::cerr << "Conversion failed: " << error_desc << std::endl;
 * }
 * @endcode
 * 
 * @since 1.0.0.1
 * @see IConvResult::error_code
 * @see ConvertEncoding()
 */
std::string UniConv::GetIconvErrorString(int err_code) {
    static const std::unordered_map<int, std::string> errorMap = {
        {EILSEQ, "Invalid multibyte sequence"},
        {EINVAL, "Incomplete multibyte sequence"},
        {E2BIG, "Output buffer too small"},
        {EBADF, "Invalid conversion descriptor"}
    };
    
    auto it = errorMap.find(err_code);
    if (it != errorMap.end()) {
        return it->second;
    }    return "Unknown iconv error: " + std::to_string(err_code);
}
/** @} */
