/*****************************************************************************
*  UniConv
*  Copyright (C) 2025 hesphoros <hesphoros@gmail.com>
*
*  This file is part of UniConv.
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License version 3 as
*  published by the Free Software Foundation.
*
*  You should have received a copy of the GNU General Public License
*  along with this program. If not, see <http://www.gnu.org/licenses/>.
*
*  @file     UniConv.h
*  @brief    UniConv A c++ library for varaiable encoding conversion
*  @details  Unicode conversion library
*
*  @author   hesphoros
*  @email    hesphoros@gmail.com
*  @version  1.0.0.1
*  @date     2025/03/10
*  @license  GNU General Public License (GPL)
*---------------------------------------------------------------------------*
*  Remark         : None
*---------------------------------------------------------------------------*
*  Change History :
*  <Date>     | <Version> | <Author>       | <Description>
/**
 * @file UniConv.h
 * @brief Universal encoding conversion library header file
 * @details This file contains the UniConv class declaration which provides
 *          comprehensive text encoding conversion capabilities between various
 *          character encodings including UTF-8, UTF-16LE, UTF-16BE, GBK, GB2312,
 *          and system locale encodings.
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
 * @section DESCRIPTION
 * UniConv is a comprehensive C++ library for text encoding conversion.
 * It provides:
 * - Bidirectional conversion between major encodings
 * - System locale encoding detection and conversion
 * - UTF-16 endianness handling (LE/BE)
 * - Error handling and validation
 * - Thread-safe singleton pattern implementation
 * 
 * @section CHANGELOG
 * - 2025/03/10 v1.0.0.1 - Initial implementation by hesphoros
 */


/** @def __UNICONV_H__
 *  @brief Header guard macro to prevent multiple inclusions
 */
#define __UNICONV_H__

// Standard library includes for encoding conversion functionality
#include <iconv.h>          ///< POSIX iconv library for character set conversion
#include <iostream>         ///< Standard I/O stream library
#include <string>           ///< Standard string class
#include <malloc.h>         ///< Memory allocation functions
#include <unordered_map>    ///< Hash map container
#include <cwchar>           ///< Wide character support
#include <clocale>          ///< Locale support
#include <sstream>          ///< String stream operations
#include <vector>           ///< Dynamic array container
#include <string_view>      ///< Lightweight string view (C++17)
#include <cerrno>           ///< System error codes
#include <fstream>          ///< File stream operations
#include <cstring>          ///< C-style string functions
#include <system_error>     ///< System error handling
#include <mutex>            ///< Thread synchronization primitives
#include <memory>           ///< Smart pointer utilities
#include <functional>       ///< Function objects and utilities
#include <io.h>             ///< Platform-specific I/O functions
#include <fcntl.h>          ///< File control options
#include "Singleton.h"      ///< Singleton pattern implementation

#ifdef _WIN32
#include <windows.h>        ///< Windows API functions
#endif // _WIN32

#ifdef __linux__
#include <unistd.h>         ///< POSIX operating system API
#include <langinfo.h>       ///< Language information queries
#endif // __linux__

/** @def UNICONV_EXPORT
 *  @brief Export macro for dynamic library builds
 *  @details When UNICONV_DLL is defined, this macro expands to __declspec(dllexport)
 *           for Windows DLL exports. Otherwise, it expands to nothing.
 */
#ifdef UNICONV_DLL
#define UNICONV_EXPORT __declspec(dllexport)
#else
#define UNICONV_EXPORT
#endif

/** @def DEBUG
 *  @brief Debug build flag
 *  @details Enable debug-specific code paths and additional error checking
 */
#define DEBUG

/** @def CPP_STANDARD
 *  @brief Cross-platform C++ standard version detection
 *  @details Uses _MSVC_LANG for MSVC compiler, __cplusplus for others
 */
#if defined(_MSC_VER)
// MSVC compiler uses _MSVC_LANG to indicate the C++ standard version
#define CPP_STANDARD _MSVC_LANG
#else
#define CPP_STANDARD __cplusplus
#endif

/** @brief Compile-time assertion to ensure minimum C++11 support
 *  @details This static assertion ensures that the code is compiled with
 *           at least C++11 standard, which is required for proper functionality.
 */
static_assert(CPP_STANDARD >= 201103L, "Error: This code requires C++11 or later");

/**
 * @brief Get the current C++ standard version as a string
 * @return A string_view representing the current C++ standard version
 * @retval "C++20 or later" if the standard is C++20 or later
 * @retval "C++17" if the standard is C++17
 * @retval "C++14" if the standard is C++14
 * @retval "C++11" if the standard is C++11
 * @retval "C++03 or earlier" if the standard is older than C++11
 * 
 * @details This constexpr function provides compile-time detection of the
 *          C++ standard version being used. It can be useful for conditional
 *          compilation and feature detection.
 */
inline constexpr std::string_view current_cpp_standard() {
#if CPP_STANDARD >= 202002L
	return "C++20 or later";
#elif CPP_STANDARD >= 201703L
	return "C++17";
#elif CPP_STANDARD >= 201402L
	return "C++14";
#elif CPP_STANDARD >= 201103L
	return "C++11";
#else
	return "C++03 or earlier";
#endif
}



/**
 * @class UniConv
 * @brief Universal character encoding conversion library
 * @details UniConv is a comprehensive singleton class that provides bidirectional
 *          text encoding conversion between various character encodings including:
 *          - UTF-8 (Unicode Transformation Format 8-bit)
 *          - UTF-16LE (Unicode Transformation Format 16-bit Little Endian)
 *          - UTF-16BE (Unicode Transformation Format 16-bit Big Endian)
 *          - GBK (Chinese character encoding)
 *          - GB2312 (Simplified Chinese character encoding)
 *          - System locale encodings
 *          - Various code page encodings
 * 
 * @par Key Features:
 * - Thread-safe singleton pattern implementation
 * - Comprehensive error handling and reporting
 * - Support for both C-style and C++ string types
 * - Automatic system encoding detection
 * - UTF-16 endianness handling
 * - Memory-safe resource management using RAII
 * 
 * @par Usage Example:
 * @code
 * auto conv = UniConv::GetInstance();
 * auto result = conv->ConvertEncoding("Hello World", "UTF-8", "GBK");
 * if (result.IsSuccess()) {
 *     std::cout << "Conversion successful: " << result.conv_result_str << std::endl;
 * } else {
 *     std::cerr << "Error: " << result.error_msg << std::endl;
 * }
 * @endcode
 * 
 * @author hesphoros
 * @version 1.0.0.1
 * @date 2025/03/10
 * @since 1.0.0.1
 */
class UNICONV_EXPORT UniConv : public Singleton<UniConv>
{

	/**
	 * @brief Friend declaration for Singleton pattern access
	 * @details Allows the Singleton base class to access private constructors
	 */
	friend class Singleton<UniConv>;

private:
	/**
	 * @struct IconvDeleter
	 * @brief Custom deleter for iconv_t resource management
	 * @details This RAII deleter ensures proper cleanup of iconv conversion
	 *          descriptors when used with smart pointers. It safely closes
	 *          valid iconv descriptors and handles invalid descriptors gracefully.
	 */
	struct IconvDeleter {
		/**
		 * @brief Function call operator for deleting iconv_t resources
		 * @param cd The iconv conversion descriptor to close
		 * @details Safely closes the iconv conversion descriptor if it's valid.
		 *          Invalid descriptors (reinterpret_cast<iconv_t>(-1)) are ignored.
		 */
		void operator()(iconv_t cd) const {
			std::cerr << "Closing iconv_t: " << cd << std::endl;
			// Call iconv_close to release the iconv descriptor only if it is valid
			if (cd != reinterpret_cast<iconv_t>(-1)) {
				iconv_close(cd);
			}
		}
	};

	/** @typedef IconvSharedPtr
	 *  @brief Shared pointer type for iconv_t resource management
	 *  @details Type alias for shared_ptr managing iconv_t resources with
	 *           automatic cleanup via IconvDeleter.
	 */
	using IconvSharedPtr = std::shared_ptr <std::remove_pointer<iconv_t>::type>;

	/**
	 * @struct EncodingInfo
	 * @brief Structure to hold detailed encoding information
	 * @details This structure contains metadata about a specific text encoding,
	 *          including .NET framework naming conventions and additional
	 *          implementation-specific information.
	 */
	struct EncodingInfo
	{
		std::string dotNetName;  /*!< .NET framework encoding name for compatibility */
		std::string extra_info;	 /*!< Additional encoding-specific information */
	};

public:

	/**
	 * @struct IConvResult
	 * @brief Result structure for encoding conversion operations
	 * @details This structure encapsulates the result of an encoding conversion
	 *          operation, including the converted string, error information,
	 *          and convenience methods for status checking.
	 * 
	 * @par Usage Example:
	 * @code
	 * IConvResult result = conv->ConvertEncoding("test", "UTF-8", "GBK");
	 * if (result.IsSuccess()) {
	 *     std::cout << "Result: " << result.conv_result_str << std::endl;
	 * } else {
	 *     std::cerr << "Error " << result.error_code << ": " << result.error_msg << std::endl;
	 * }
	 * @endcode
	 */
	struct IConvResult {
		std::string        conv_result_str;	    /*!< The successfully converted string data */
		int                error_code = 0;      /*!< Error code (0 = success, non-zero = error) */
		std::string        error_msg = {NULL};  /*!< Human-readable error description */

		/**
		 * @brief Check if the conversion operation was successful
		 * @return true if the conversion succeeded (error_code == 0), false otherwise
		 * @details This is the primary method to check conversion status.
		 *          A successful conversion has error_code equal to 0.
		 */
		bool IsSuccess() const {
			return error_code == 0;
		}

		/**
		 * @brief Explicit boolean conversion operator
		 * @return true if the conversion was successful, false otherwise
		 * @details Allows the result to be used in boolean contexts:
		 *          if (result) { /* success */ }
		 */
		explicit operator bool() const {
			return IsSuccess();
		}

		/**
		 * @brief Logical NOT operator
		 * @return true if the conversion failed, false if successful
		 * @details Allows checking for failure: if (!result) { /* handle error */ }
		 */
		bool operator!() const {
			return !IsSuccess();
		}

		/**
		 * @brief Equality comparison with error code
		 * @param code The error code to compare with
		 * @return true if the result's error_code equals the given code
		 * @details Useful for checking specific error conditions
		 */
		bool operator==(int code) const {
			return error_code == code;
		}

		/**
		 * @brief Inequality comparison with error code
		 * @param code The error code to compare with
		 * @return true if the result's error_code does not equal the given code
		 * @details Useful for filtering out specific error conditions
		 */
		bool operator!=(int code) const {
			return error_code != code;
		}

		/**
		 * @brief Get C-style string representation
		 * @return Pointer to converted string if successful, error message if failed
		 * @details Returns conv_result_str.c_str() on success, error_msg.data() on failure.
		 *          Useful for C API compatibility.
		 */
		const char* c_str() const {
			return IsSuccess() ? conv_result_str.c_str() : error_msg.data();
		}
	};

	/**
	 * @brief Destructor for UniConv class
	 * @details Clean up any resources. Since this is a singleton class,
	 *          the destructor is typically called only on program termination.
	 */
	~UniConv() {
	}

/***************************************************************************/
/*========================= Get current encoding ==========================*/
/***************************************************************************/	/** @name System Encoding Detection and Conversion
	 *  @brief Functions for detecting and converting system locale encodings
	 *  @{
	 */

	/**
	 * @brief Get the current system's default character encoding
	 * @return String representing the current system encoding name
	 * @retval "UTF-8" if the system uses UTF-8 encoding
	 * @retval "GBK" if the system uses GBK encoding (Windows CP936)
	 * @retval "BIG5" if the system uses Big5 encoding (Windows CP950)
	 * @retval "SHIFT_JIS" if the system uses Shift_JIS encoding (Windows CP932)
	 * @retval "CP####" for other Windows code pages
	 * 
	 * @details This function detects the system's active code page on Windows
	 *          using GetACP() and returns the corresponding encoding name.
	 *          On Linux systems, it typically returns "UTF-8".
	 * 
	 * @par Platform Support:
	 * - Windows: Uses GetACP() to detect active code page
	 * - Linux: Returns "UTF-8" by default
	 * 
	 * @see GetCurrentSystemEncodingCodePage()
	 * @since 1.0.0.1
	 */
	std::string GetCurrentSystemEncoding();

	/**
	 * @brief Universal encoding conversion function using iconv
	 * @param input The input string data to be converted
	 * @param fromEncoding Source encoding name (e.g., "UTF-8", "GBK", "UTF-16LE")
	 * @param toEncoding Target encoding name (e.g., "UTF-8", "GBK", "UTF-16BE")
	 * @return IConvResult containing conversion result or error information
	 * 
	 * @details This is the core conversion function that handles conversion between
	 *          any two supported encodings using the iconv library. It provides
	 *          comprehensive error handling and memory management.
	 * 
	 * @par Supported Encodings:
	 * - UTF-8, UTF-16LE, UTF-16BE
	 * - GBK, GB2312, BIG5
	 * - Windows code pages (CP936, CP950, etc.)
	 * - ISO-8859 series
	 * - And many others supported by iconv
	 * 
	 * @par Error Handling:
	 * - Returns error code and message on failure
	 * - Handles invalid input sequences gracefully
	 * - Provides detailed error information
	 * 
	 * @par Example:
	 * @code
	 * auto result = conv->ConvertEncoding("你好", "UTF-8", "GBK");
	 * if (result.IsSuccess()) {
	 *     std::cout << "Converted successfully" << std::endl;
	 * }
	 * @endcode
	 * 
	 * @see IConvResult
	 * @since 1.0.0.1
	 */
	IConvResult ConvertEncoding(const std::string& input, const char* fromEncoding, const char* toEncoding);

	/**
	 * @brief Get the current system's active code page number
	 * @return The numeric code page identifier
	 * @retval 936 for GBK (Simplified Chinese)
	 * @retval 950 for Big5 (Traditional Chinese)
	 * @retval 932 for Shift_JIS (Japanese)
	 * @retval 65001 for UTF-8
	 * @retval Other values for various code pages
	 * 
	 * @details This static function returns the numeric identifier of the
	 *          system's active code page. On Windows, it uses GetACP().
	 *          On Linux systems, it returns 65001 (UTF-8) by default.
	 * 
	 * @see GetCurrentSystemEncoding(), GetEncodingNameByCodePage()
	 * @since 1.0.0.1
	 */
	static std::uint16_t GetCurrentSystemEncodingCodePage();

	/**
	 * @brief Convert a code page number to its corresponding encoding name
	 * @param codePage The numeric code page identifier
	 * @return String containing the encoding name
	 * @retval "GBK" for code page 936
	 * @retval "BIG5" for code page 950
	 * @retval "SHIFT_JIS" for code page 932
	 * @retval "UTF-8" for code page 65001
	 * @retval "CP1252" for code page 1252
	 * @retval "CP####" for other code pages (#### = code page number)
	 * 
	 * @details This static utility function provides a mapping from Windows
	 *          code page numbers to standard encoding names used by iconv.
	 * 
	 * @see GetCurrentSystemEncodingCodePage()
	 * @since 1.0.0.1
	 */
	static std::string GetEncodingNameByCodePage(std::uint16_t codePage);

	/** @} */ // end of System Encoding group

	/** @name Local Encoding to UTF-8 Conversion
	 *  @brief High-level functions for converting between system locale and UTF-8
	 *  @{
	 */

	/**
	 * @brief Convert system locale encoded string to UTF-8
	 * @param input Input string in system locale encoding
	 * @return UTF-8 encoded string, empty string on conversion failure
	 * 
	 * @details This function automatically detects the system's locale encoding
	 *          and converts the input string to UTF-8. It's a convenience wrapper
	 *          around ConvertEncoding() that handles system encoding detection.
	 * 
	 * @par Example:
	 * @code
	 * // On a GBK system, convert GBK string to UTF-8
	 * std::string gbk_text = "你好";  // Assumes this is GBK encoded
	 * std::string utf8_text = conv->ToUtf8FromLocal(gbk_text);
	 * @endcode
	 * 
	 * @see ConvertEncoding(), GetCurrentSystemEncoding()
	 * @since 1.0.0.1
	 */
	std::string ToUtf8FromLocal(const std::string& input);

	/**
	 * @brief Convert system locale encoded C-string to UTF-8
	 * @param input Input C-style string in system locale encoding
	 * @return UTF-8 encoded string, empty string on conversion failure or null input
	 * 
	 * @details Overloaded version that accepts C-style strings (const char*).
	 *          Handles null input gracefully by returning an empty string.
	 * 
	 * @par Example:
	 * @code
	 * const char* gbk_text = "你好";  // GBK encoded C-string
	 * std::string utf8_text = conv->ToUtf8FromLocal(gbk_text);
	 * @endcode
	 * 
	 * @see ToUtf8FromLocal(const std::string&)
	 * @since 1.0.0.1
	 */
	std::string ToUtf8FromLocal(const char* input);

	/**
	 * @brief Convert UTF-8 string to system locale encoding
	 * @param input Input UTF-8 encoded string
	 * @return String in system locale encoding, empty string on conversion failure
	 * 
	 * @details This function converts UTF-8 text to the system's default locale
	 *          encoding. Useful for preparing text for display in legacy applications
	 *          or for system API calls that expect locale-encoded strings.
	 * 
	 * @par Example:
	 * @code
	 * std::string utf8_text = "你好";  // UTF-8 encoded
	 * std::string local_text = conv->FromUtf8ToLocal(utf8_text);  // Convert to GBK on GBK system
	 * @endcode
	 * 
	 * @see ConvertEncoding(), GetCurrentSystemEncoding()
	 * @since 1.0.0.1
	 */
	std::string FromUtf8ToLocal(const std::string& input);

	/**
	 * @brief Convert UTF-8 C-string to system locale encoding
	 * @param input Input UTF-8 encoded C-style string
	 * @return String in system locale encoding, empty string on conversion failure or null input
	 * 
	 * @details Overloaded version that accepts C-style strings (const char*).
	 *          Handles null input gracefully by returning an empty string.
	 * 
	 * @see FromUtf8ToLocal(const std::string&)
	 * @since 1.0.0.1
	 */
	std::string FromUtf8ToLocal(const char* input);
	/** @} */ // end of Local Encoding group

	/** @name UTF-16 Conversion Functions
	 *  @brief Functions for converting between UTF-16 and other encodings
	 *  @{
	 */

	/**
	 * @brief Convert system locale encoded string to UTF-16LE
	 * @param input Input string in system locale encoding
	 * @return UTF-16LE encoded string (std::u16string)
	 * 
	 * @details This function converts text from the system's default locale
	 *          encoding to UTF-16 Little Endian format. UTF-16LE is commonly
	 *          used on Windows platforms and for data interchange.
	 * 
	 * @par Example:
	 * @code
	 * // On a GBK system
	 * std::string gbk_text = "你好世界";
	 * std::u16string utf16_text = conv->ToUtf16LEFromLocal(gbk_text);
	 * @endcode
	 * 
	 * @see FromUtf16LEToLocal(), ToUtf16BEFromLocal()
	 * @since 1.0.0.1
	 */
	std::u16string ToUtf16LEFromLocal(const std::string& input);

	/**
	 * @brief Convert system locale encoded C-string to UTF-16LE
	 * @param input Input C-style string in system locale encoding
	 * @return UTF-16LE encoded string (std::u16string)
	 * 
	 * @details Overloaded version that accepts C-style strings.
	 *          Handles null input gracefully by returning an empty string.
	 * 
	 * @see ToUtf16LEFromLocal(const std::string&)
	 * @since 1.0.0.1
	 */
	std::u16string ToUtf16LEFromLocal(const char* input);

	/**
	 * @brief Convert system locale encoded string to UTF-16BE
	 * @param input Input string in system locale encoding
	 * @return UTF-16BE encoded string (std::u16string)
	 * 
	 * @details This function converts text from the system's default locale
	 *          encoding to UTF-16 Big Endian format. UTF-16BE is used in
	 *          some network protocols and file formats.
	 * 
	 * @par Example:
	 * @code
	 * std::string local_text = "Hello World";
	 * std::u16string utf16be_text = conv->ToUtf16BEFromLocal(local_text);
	 * @endcode
	 * 
	 * @see FromUtf16BEToLocal(), ToUtf16LEFromLocal()
	 * @since 1.0.0.1
	 */
	std::u16string ToUtf16BEFromLocal(const std::string& input);

	/**
	 * @brief Convert system locale encoded C-string to UTF-16BE
	 * @param input Input C-style string in system locale encoding
	 * @return UTF-16BE encoded string (std::u16string)
	 * 
	 * @details Overloaded version that accepts C-style strings.
	 *          Handles null input gracefully by returning an empty string.
	 * 
	 * @see ToUtf16BEFromLocal(const std::string&)
	 * @since 1.0.0.1
	 */
	std::u16string ToUtf16BEFromLocal(const char* input);

	/**
	 * @brief Convert UTF-16LE encoded string to system locale encoding
	 * @param input UTF-16LE encoded string to convert
	 * @return String in system locale encoding
	 * 
	 * @details This function converts UTF-16 Little Endian text to the
	 *          system's default locale encoding. Useful for preparing
	 *          Unicode text for display in legacy applications.
	 * 
	 * @par Example:
	 * @code
	 * std::u16string utf16_text = u"Hello World";
	 * std::string local_text = conv->FromUtf16LEToLocal(utf16_text);
	 * @endcode
	 * 
	 * @see ToUtf16LEFromLocal(), FromUtf16BEToLocal()
	 * @since 1.0.0.1
	 */
	std::string FromUtf16LEToLocal(const std::u16string& input);

	/**
	 * @brief Convert UTF-16LE C-style string to system locale encoding
	 * @param input UTF-16LE C-style string to convert
	 * @return String in system locale encoding
	 * 
	 * @details Overloaded version that accepts C-style char16_t strings.
	 *          Handles null input gracefully by returning an empty string.
	 * 
	 * @see FromUtf16LEToLocal(const std::u16string&)
	 * @since 1.0.0.1
	 */
	std::string FromUtf16LEToLocal(const char16_t* input);

	/**
	 * @brief Convert UTF-16BE encoded string to system locale encoding
	 * @param input UTF-16BE encoded string to convert
	 * @return String in system locale encoding
	 * 
	 * @details This function converts UTF-16 Big Endian text to the
	 *          system's default locale encoding.
	 * 
	 * @see ToUtf16BEFromLocal(), FromUtf16LEToLocal()
	 * @since 1.0.0.1
	 */
	std::string FromUtf16BEToLocal(const std::u16string& input);

	/**
	 * @brief Convert UTF-16BE C-style string to system locale encoding
	 * @param input UTF-16BE C-style string to convert
	 * @return String in system locale encoding
	 * 
	 * @details Overloaded version that accepts C-style char16_t strings.
	 *          Handles null input gracefully by returning an empty string.
	 * 
	 * @see FromUtf16BEToLocal(const std::u16string&)
	 * @since 1.0.0.1
	 */
	std::string FromUtf16BEToLocal(const char16_t* input);

	/** @} */ // end of UTF-16 Conversion group

	/** @name UTF-16 to UTF-8 Conversion
	 *  @brief Functions for converting between UTF-16 and UTF-8 encodings
	 *  @{
	 */

	/**
	 * @brief Convert UTF-16LE string to UTF-8
	 * @param input UTF-16LE string to convert
	 * @return UTF-8 encoded string
	 * 
	 * @details This function converts UTF-16 Little Endian text to UTF-8.
	 *          Both encodings represent Unicode text but use different
	 *          byte sequences.
	 * 
	 * @par Example:
	 * @code
	 * std::u16string utf16_text = u"Hello 世界";
	 * std::string utf8_text = conv->FromUtf16LEToUtf8(utf16_text);
	 * @endcode
	 * 
	 * @see FromUtf8ToUtf16LE(), FromUtf16BEToUtf8()
	 * @since 1.0.0.1
	 */
	std::string FromUtf16LEToUtf8(const std::u16string& input);

	/**
	 * @brief Convert UTF-16LE buffer with length to UTF-8
	 * @param input Pointer to UTF-16LE data
	 * @param len Length of the input buffer in char16_t units
	 * @return UTF-8 encoded string
	 * 
	 * @details This overload allows conversion of UTF-16LE data with
	 *          explicit length specification, useful for processing
	 *          binary data or buffers that may not be null-terminated.
	 * 
	 * @see FromUtf16LEToUtf8(const std::u16string&)
	 * @since 1.0.0.1
	 */
	std::string FromUtf16LEToUtf8(const char16_t* input, size_t len);

	/**
	 * @brief Convert UTF-16LE C-style string to UTF-8
	 * @param input UTF-16LE C-style string to convert
	 * @return UTF-8 encoded string
	 * 
	 * @details Overloaded version that accepts null-terminated C-style
	 *          char16_t strings. Handles null input gracefully.
	 * 
	 * @see FromUtf16LEToUtf8(const std::u16string&)
	 * @since 1.0.0.1
	 */
	std::string FromUtf16LEToUtf8(const char16_t* input);

	/**
	 * @brief Convert UTF-16BE string to UTF-8
	 * @param input UTF-16BE string to convert
	 * @return UTF-8 encoded string
	 * 
	 * @details This function converts UTF-16 Big Endian text to UTF-8.
	 *          Handles the endianness conversion automatically.
	 * 
	 * @see FromUtf8ToUtf16BE(), FromUtf16LEToUtf8()
	 * @since 1.0.0.1
	 */
	std::string FromUtf16BEToUtf8(const std::u16string& input);

	/**
	 * @brief Convert UTF-16BE buffer with length to UTF-8
	 * @param input Pointer to UTF-16BE data
	 * @param len Length of the input buffer in char16_t units
	 * @return UTF-8 encoded string
	 * 
	 * @details This overload allows conversion of UTF-16BE data with
	 *          explicit length specification.
	 * 
	 * @see FromUtf16BEToUtf8(const std::u16string&)
	 * @since 1.0.0.1
	 */
	std::string FromUtf16BEToUtf8(const char16_t* input, size_t len);

	/**
	 * @brief Convert UTF-16BE C-style string to UTF-8
	 * @param input UTF-16BE C-style string to convert
	 * @return UTF-8 encoded string
	 * 
	 * @details Overloaded version that accepts null-terminated C-style
	 *          char16_t strings. Handles null input gracefully.
	 * 
	 * @see FromUtf16BEToUtf8(const std::u16string&)
	 * @since 1.0.0.1
	 */
	std::string FromUtf16BEToUtf8(const char16_t* input);

	/** @} */ // end of UTF-16 to UTF-8 group

	/** @name UTF-8 to UTF-16 Conversion
	 *  @brief Functions for converting UTF-8 to UTF-16 encodings
	 *  @{
	 */

	/**
	 * @brief Convert UTF-8 string to UTF-16LE
	 * @param input UTF-8 string to convert
	 * @return UTF-16LE encoded string (std::u16string)
	 * 
	 * @details This function converts UTF-8 text to UTF-16 Little Endian.
	 *          This is commonly needed for Windows API calls that expect
	 *          wide character strings.
	 * 
	 * @par Example:
	 * @code
	 * std::string utf8_text = "Hello 世界";
	 * std::u16string utf16_text = conv->FromUtf8ToUtf16LE(utf8_text);
	 * @endcode
	 * 
	 * @see FromUtf16LEToUtf8(), FromUtf8ToUtf16BE()
	 * @since 1.0.0.1
	 */
	std::u16string FromUtf8ToUtf16LE(const std::string& input);

	/**
	 * @brief Convert UTF-8 C-style string to UTF-16LE
	 * @param input UTF-8 C-style string to convert
	 * @return UTF-16LE encoded string (std::u16string)
	 * 
	 * @details Overloaded version that accepts C-style strings.
	 *          Handles null input gracefully by returning an empty string.
	 * 
	 * @see FromUtf8ToUtf16LE(const std::string&)
	 * @since 1.0.0.1
	 */
	std::u16string FromUtf8ToUtf16LE(const char* input);

	/**
	 * @brief Convert UTF-8 string to UTF-16BE
	 * @param input UTF-8 string to convert
	 * @return UTF-16BE encoded string (std::u16string)
	 * 
	 * @details This function converts UTF-8 text to UTF-16 Big Endian.
	 *          Useful for network protocols and file formats that specify
	 *          big-endian byte order.
	 * 
	 * @see FromUtf16BEToUtf8(), FromUtf8ToUtf16LE()
	 * @since 1.0.0.1
	 */
	std::u16string FromUtf8ToUtf16BE(const std::string& input);

	/**
	 * @brief Convert UTF-8 C-style string to UTF-16BE
	 * @param input UTF-8 C-style string to convert
	 * @return UTF-16BE encoded string (std::u16string)
	 * 
	 * @details Overloaded version that accepts C-style strings.
	 *          Handles null input gracefully by returning an empty string.
	 * 
	 * @see FromUtf8ToUtf16BE(const std::string&)
	 * @since 1.0.0.1
	 */
	std::u16string FromUtf8ToUtf16BE(const char* input);

	/** @} */ // end of UTF-8 to UTF-16 group

	/** @name UTF-16 Endianness Conversion
	 *  @brief Functions for converting between UTF-16LE and UTF-16BE
	 *  @{
	 */

	/**
	 * @brief Convert UTF-16LE string to UTF-16BE
	 * @param input UTF-16LE string to convert
	 * @return UTF-16BE encoded string (std::u16string)
	 * 
	 * @details This function converts between UTF-16 endianness formats.
	 *          The text content remains the same, but the byte order changes
	 *          from little-endian to big-endian.
	 * 
	 * @par Technical Details:
	 * - Swaps byte order of each 16-bit code unit
	 * - Preserves Unicode code points and text meaning
	 * - Handles surrogate pairs correctly
	 * 
	 * @see FromUtf16BEToUtf16LE()
	 * @since 1.0.0.1
	 */
	std::u16string FromUtf16LEToUtf16BE(const std::u16string& input);

	/**
	 * @brief Convert UTF-16LE C-style string to UTF-16BE
	 * @param input UTF-16LE C-style string to convert
	 * @return UTF-16BE encoded string (std::u16string)
	 * 
	 * @details Overloaded version that accepts C-style char16_t strings.
	 *          Handles null input gracefully by returning an empty string.
	 * 
	 * @see FromUtf16LEToUtf16BE(const std::u16string&)
	 * @since 1.0.0.1
	 */
	std::u16string FromUtf16LEToUtf16BE(const char16_t* input);

	/**
	 * @brief Convert UTF-16BE string to UTF-16LE
	 * @param input UTF-16BE string to convert
	 * @return UTF-16LE encoded string (std::u16string)
	 * 
	 * @details This function converts between UTF-16 endianness formats.
	 *          The text content remains the same, but the byte order changes
	 *          from big-endian to little-endian.
	 * 
	 * @see FromUtf16LEToUtf16BE()
	 * @since 1.0.0.1
	 */
	std::u16string FromUtf16BEToUtf16LE(const std::u16string& input);

	/**
	 * @brief Convert UTF-16BE C-style string to UTF-16LE
	 * @param input UTF-16BE C-style string to convert
	 * @return UTF-16LE encoded string (std::u16string)
	 * 
	 * @details Overloaded version that accepts C-style char16_t strings.
	 *          Handles null input gracefully by returning an empty string.
	 * 
	 * @see FromUtf16BEToUtf16LE(const std::u16string&)
	 * @since 1.0.0.1
	 */
	std::u16string FromUtf16BEToUtf16LE(const char16_t* input);

	/** @} */ // end of UTF-16 Endianness group


	/** @name Wide String Conversion Functions
	 *  @brief Functions for converting between narrow and wide strings
	 *  @details Wide strings (std::wstring) use platform-specific wide characters:
	 *           - Windows: 16-bit wchar_t (UTF-16)
	 *           - Linux: 32-bit wchar_t (UTF-32)
	 *  @{
	 */

	/**
	 * @brief Convert locale-encoded string to wide string
	 * @param sInput Input string in system locale encoding
	 * @return Wide string (std::wstring) representation
	 * 
	 * @details This function converts a system locale encoded string to
	 *          a wide string using the platform's native wide character
	 *          encoding. On Windows, this typically results in UTF-16
	 *          encoded wide strings.
	 * 
	 * @par Platform Differences:
	 * - Windows: wchar_t is 16-bit, uses UTF-16 encoding
	 * - Linux: wchar_t is 32-bit, uses UTF-32 encoding
	 * 
	 * @par Example:
	 * @code
	 * std::string local_text = "Hello World";
	 * std::wstring wide_text = conv->LocaleToWideString(local_text);
	 * @endcode
	 * 
	 * @see WideConvertToLocale()
	 * @since 1.0.0.1
	 */
	std::wstring LocaleToWideString(const std::string& sInput);

	/**
	 * @brief Convert locale-encoded C-string to wide string
	 * @param sInput Input C-style string in system locale encoding
	 * @return Wide string (std::wstring) representation
	 * 
	 * @details Overloaded version that accepts C-style strings.
	 *          Handles null input gracefully by returning an empty string.
	 * 
	 * @see LocaleToWideString(const std::string&)
	 * @since 1.0.0.1
	 */
	std::wstring LocaleToWideString(const char* sInput);

	/**
	 * @brief Convert wide string to locale-encoded string
	 * @param sInput Wide string to convert
	 * @return String in system locale encoding
	 * 
	 * @details This function converts a wide string back to the system's
	 *          locale encoding. Useful for preparing text for APIs that
	 *          expect narrow character strings.
	 * 
	 * @see LocaleToWideString()
	 * @since 1.0.0.1
	 */
	std::string WideConvertToLocale(const std::wstring& sInput);

	/**
	 * @brief Convert wide C-string to locale-encoded string
	 * @param sInput Wide C-style string to convert
	 * @return String in system locale encoding
	 * 
	 * @details Overloaded version that accepts C-style wide strings.
	 *          Handles null input gracefully by returning an empty string.
	 * 
	 * @see WideConvertToLocale(const std::wstring&)
	 * @since 1.0.0.1
	 */
	std::string WideConvertToLocale(const wchar_t* sInput);

	/**
	 * @brief Convert wide string to locale-encoded string (alias)
	 * @param sInput Wide string to convert
	 * @return String in system locale encoding
	 * 
	 * @details Alternative function name for WideConvertToLocale().
	 *          Provides consistent naming with other conversion functions.
	 * 
	 * @see WideConvertToLocale()
	 * @since 1.0.0.1
	 */
	std::string WideStringToLocale(const std::wstring& sInput);

	/**
	 * @brief Convert wide C-string to locale-encoded string (alias)
	 * @param sInput Wide C-style string to convert
	 * @return String in system locale encoding
	 * 
	 * @details Alternative function name for WideConvertToLocale().
	 *          Handles null input gracefully by returning an empty string.
	 * 
	 * @see WideConvertToLocale(const wchar_t*)
	 * @since 1.0.0.1
	 */
	std::string WideStringToLocale(const wchar_t* sInput);

	/** @} */ // end of Wide String group

	/** @name UTF-8 and Wide String Conversion
	 *  @brief Functions for converting between UTF-8 and wide strings
	 *  @{
	 */

	/**
	 * @brief Convert UTF-8 string to wide string
	 * @param sInput UTF-8 encoded string to convert
	 * @return Wide string (std::wstring) representation
	 * 
	 * @details This function converts UTF-8 text to a wide string.
	 *          The resulting wide string encoding depends on the platform:
	 *          Windows uses UTF-16, Linux typically uses UTF-32.
	 * 
	 * @deprecated This method may be removed in future versions.
	 *             Consider using platform-specific UTF-16 or UTF-32 functions.
	 * 
	 * @par Example:
	 * @code
	 * std::string utf8_text = "Hello 世界";
	 * std::wstring wide_text = conv->Utf8ConvertToWide(utf8_text);
	 * @endcode
	 * 
	 * @see WideConvertToUtf8()
	 * @since 1.0.0.1
	 */
	std::wstring Utf8ConvertToWide(const std::string& sInput);

	/**
	 * @brief Convert UTF-8 C-string to wide string
	 * @param sInput UTF-8 encoded C-style string to convert
	 * @return Wide string (std::wstring) representation
	 * 
	 * @deprecated This method may be removed in future versions.
	 * @see Utf8ConvertToWide(const std::string&)
	 * @since 1.0.0.1
	 */
	std::wstring Utf8ConvertToWide(const char* sInput);

	/**
	 * @brief Convert wide string to UTF-8
	 * @param sInput Wide string to convert
	 * @return UTF-8 encoded string
	 * 
	 * @details This function converts a wide string to UTF-8 encoding.
	 *          Works regardless of the platform's wide character size.
	 * 
	 * @par Example:
	 * @code
	 * std::wstring wide_text = L"Hello 世界";
	 * std::string utf8_text = conv->WideConvertToUtf8(wide_text);
	 * @endcode
	 * 
	 * @see Utf8ConvertToWide()
	 * @since 1.0.0.1
	 */
	std::string WideConvertToUtf8(const std::wstring& sInput);

	/**
	 * @brief Convert wide C-string to UTF-8
	 * @param sInput Wide C-style string to convert
	 * @return UTF-8 encoded string
	 * 
	 * @details Overloaded version that accepts C-style wide strings.
	 *          Handles null input gracefully by returning an empty string.
	 * 
	 * @see WideConvertToUtf8(const std::wstring&)
	 * @since 1.0.0.1
	 */
	std::string WideConvertToUtf8(const wchar_t* sInput);

	/** @} */ // end of UTF-8 Wide String group

	/** @name UTF-32 Conversion Functions
	 *  @brief Functions for converting between UTF-32 and other Unicode encodings
	 *  @details UTF-32 uses fixed-width 32-bit code units, making it the simplest
	 *           Unicode encoding for character processing (no surrogates needed).
	 *  @{
	 */

	/**
	 * @brief Convert UTF-32 string to UTF-8
	 * @param sInput UTF-32 string to convert
	 * @return UTF-8 encoded string
	 * 
	 * @details This function converts UTF-32 text to UTF-8 encoding.
	 *          UTF-32 uses fixed 32-bit code units while UTF-8 uses
	 *          variable-length encoding (1-4 bytes per character).
	 * 
	 * @par Example:
	 * @code
	 * std::u32string utf32_text = U"Hello 世界 ?";
	 * std::string utf8_text = conv->Utf32ConvertToUtf8(utf32_text);
	 * @endcode
	 * 
	 * @see Utf8ConvertToUtf32()
	 * @since 1.0.0.1
	 */
	std::string Utf32ConvertToUtf8(const std::u32string& sInput);

	/**
	 * @brief Convert UTF-32 string to UTF-16LE
	 * @param sInput UTF-32 string to convert
	 * @return UTF-16LE encoded string (std::u16string)
	 * 
	 * @details This function converts UTF-32 text to UTF-16 Little Endian.
	 *          Characters above U+FFFF are encoded as surrogate pairs
	 *          in the resulting UTF-16 text.
	 * 
	 * @par Surrogate Handling:
	 * - Characters U+0000-U+FFFF: Single 16-bit code unit
	 * - Characters U+10000-U+10FFFF: Surrogate pair (2 code units)
	 * 
	 * @see Utf16LEConvertToUtf32()
	 * @since 1.0.0.1
	 */
	std::u16string Utf32ConvertToUtf16LE(const std::u32string& sInput);

	/**
	 * @brief Convert UTF-32 string to UTF-16BE
	 * @param sInput UTF-32 string to convert
	 * @return UTF-16BE encoded string (std::u16string)
	 * 
	 * @details This function converts UTF-32 text to UTF-16 Big Endian.
	 *          Handles surrogate pair generation and big-endian byte ordering.
	 * 
	 * @see Utf16BEConvertToUtf32()
	 * @since 1.0.0.1
	 */
	std::u16string Utf32ConvertToUtf16BE(const std::u32string& sInput);

	/**
	 * @brief Convert UTF-8 string to UTF-32
	 * @param sInput UTF-8 string to convert
	 * @return UTF-32 encoded string (std::u32string)
	 * 
	 * @details This function converts UTF-8 text to UTF-32 encoding.
	 *          Each Unicode code point becomes a single 32-bit code unit
	 *          in the resulting string.
	 * 
	 * @par Benefits of UTF-32:
	 * - Fixed-width encoding (no variable-length sequences)
	 * - Direct indexing by character position
	 * - No surrogate pairs needed
	 * - Simplified string processing algorithms
	 * 
	 * @see Utf32ConvertToUtf8()
	 * @since 1.0.0.1
	 */
	std::u32string Utf8ConvertToUtf32(const std::string& sInput);

	/**
	 * @brief Convert UTF-16LE string to UTF-32
	 * @param sInput UTF-16LE string to convert
	 * @return UTF-32 encoded string (std::u32string)
	 * 
	 * @details This function converts UTF-16 Little Endian text to UTF-32.
	 *          Surrogate pairs in the input are correctly decoded to
	 *          single 32-bit code points.
	 * 
	 * @par Surrogate Processing:
	 * - Single code units → Direct conversion to UTF-32
	 * - Surrogate pairs → Combined into single UTF-32 code point
	 * - Invalid surrogates → Handled according to error policy
	 * 
	 * @see Utf32ConvertToUtf16LE()
	 * @since 1.0.0.1
	 */
	std::u32string Utf16LEConvertToUtf32(const std::u16string& sInput);

	/**
	 * @brief Convert UTF-16BE string to UTF-32
	 * @param sInput UTF-16BE string to convert
	 * @return UTF-32 encoded string (std::u32string)
	 * 
	 * @details This function converts UTF-16 Big Endian text to UTF-32.
	 *          Handles big-endian byte order and surrogate pair decoding.
	 * 
	 * @see Utf32ConvertToUtf16BE()
	 * @since 1.0.0.1
	 */
	std::u32string Utf16BEConvertToUtf32(const std::u16string& sInput);

	/** @} */ // end of UTF-32 group


	/** @name String Conversion Utilities
	 *  @brief Utility functions for basic string type conversions
	 *  @{
	 */

	/**
	 * @brief Convert locale string to wide string (utility function)
	 * @param str Input string in system locale encoding
	 * @return Wide string (std::wstring) representation
	 * 
	 * @details This utility function converts a system locale encoded
	 *          string to a wide string. It's a convenience wrapper that
	 *          provides consistent naming with other string conversion functions.
	 * 
	 * @todo Complete the corresponding C-style method implementation
	 * 
	 * @see LocaleToWideString()
	 * @since 1.0.0.1
	 */
	std::wstring StringConvertToWstring(const std::string& str);

	/**
	 * @brief Convert wide string to locale string
	 * @param wstr Wide string to convert
	 * @return String in system locale encoding
	 * 
	 * @details This utility function converts a wide string back to
	 *          the system's locale encoding. Provides consistent naming
	 *          with other conversion functions.
	 * 
	 * @see WideConvertToLocale()
	 * @since 1.0.0.1
	 */
	std::string WstringConvertToString(const std::wstring& wstr);

	/** @} */ // end of String Utilities group

	/** @name Core Conversion Functions
	 *  @brief Low-level conversion functions using iconv directly
	 *  @{
	 */

	/**
	 * @brief Core string conversion function using iconv
	 * @param in Input string view to convert
	 * @param fromcode Source encoding name (e.g., "UTF-8", "GBK")
	 * @param tocode Target encoding name (e.g., "UTF-16LE", "UTF-8")
	 * @return IConvResult containing conversion result or error information
	 * 
	 * @details This is the primary low-level conversion function that performs
	 *          the actual encoding conversion using the iconv library. It provides
	 *          comprehensive error handling and supports all encodings available
	 *          in the system's iconv implementation.
	 * 
	 * @par Error Handling:
	 * - Invalid encoding names → Error with appropriate message
	 * - Invalid input sequences → Error with character position
	 * - Memory allocation failures → Error with system message
	 * - Incomplete sequences → Error with recovery suggestions
	 * 
	 * @par Encoding Support:
	 * The function supports any encoding recognized by iconv, including:
	 * - Unicode variants: UTF-8, UTF-16LE, UTF-16BE, UTF-32LE, UTF-32BE
	 * - Chinese: GBK, GB2312, GB18030, BIG5, HZ
	 * - Japanese: SHIFT_JIS, EUC-JP, ISO-2022-JP
	 * - Korean: EUC-KR, UHC, JOHAB
	 * - Western: ISO-8859-1, CP1252, ASCII
	 * - And many others
	 * 
	 * @par Example:
	 * @code
	 * auto result = conv->Convert("Hello 世界", "UTF-8", "GBK");
	 * if (result.IsSuccess()) {
	 *     std::cout << "Converted: " << result.conv_result_str << std::endl;
	 * } else {
	 *     std::cerr << "Error " << result.error_code << ": " << result.error_msg << std::endl;
	 * }
	 * @endcode
	 * 
	 * @note This method performs the actual conversion work that higher-level
	 *       convenience functions delegate to.
	 * 
	 * @see ConvertEncoding(), IConvResult
	 * @since 1.0.0.1
	 */
	IConvResult Convert(std::string_view in, const char* fromcode, const char* tocode);

	/**
	 * @brief Core wide string conversion function using iconv
	 * @param in Input wide string view to convert
	 * @param fromcode Source encoding name
	 * @param tocode Target encoding name  
	 * @return IConvResult containing conversion result or error information
	 * 
	 * @details This function performs encoding conversion on wide string input.
	 *          The wide string is first converted to an appropriate byte sequence
	 *          based on the source encoding, then converted using iconv.
	 * 
	 * @todo Complete the implementation of this method
	 * 
	 * @see Convert(std::string_view, const char*, const char*)
	 * @since 1.0.0.1
	 */
	IConvResult Convert(std::wstring_view in, const char* fromcode, const char* tocode);

	/** @} */ // end of Core Conversion group

private:

	/** @name Private Data Members
	 *  @brief Internal data structures for encoding management and caching
	 *  @{
	 */

	/*!< @brief Static encoding information map
	 *   @details Maps Windows code page numbers to encoding information structures.
	 *            Provides fast lookup for encoding metadata and .NET compatibility names.
	 */
	static const std::unordered_map<std::uint16_t,EncodingInfo>  m_encodingMap;

	/*!< @brief Encoding name to code page mapping
	 *   @details Maps iconv encoding names to Windows code page numbers.
	 *            Enables reverse lookup from encoding names to numeric identifiers.
	 */
	static const std::unordered_map<std::string,std::uint16_t>   m_encodingToCodePageMap;

	/*!< @brief Mutex for thread-safe iconv descriptor cache access
	 *   @details Protects the shared iconv descriptor cache from race conditions
	 *            in multi-threaded environments. Ensures thread-safe access to
	 *            cached conversion descriptors.
	 */
	std::mutex m_iconvcCacheMutex;

	/*!< @brief Static iconv error code to message mapping
	 *   @details Maps numeric iconv error codes to human-readable error messages.
	 *            Provides consistent error reporting across the library.
	 */
	static const std::unordered_map<int,std::string_view> m_iconvErrorMap;

	/*!< @brief Shared iconv descriptor cache
	 *   @details Thread-safe cache of iconv conversion descriptors. Reuses descriptors
	 *            for the same encoding pairs to improve performance. Uses shared_ptr
	 *            with custom deleter for automatic resource management.
	 */
	static std::unordered_map<std::string, IconvSharedPtr> m_iconvDesscriptorCacheMapS;

	/** @} */ // end of Private Data Members group

	/** @name Private Helper Functions
	 *  @brief Internal utility functions for error handling and descriptor management
	 *  @{
	 */

	/**
	 * @brief Get human-readable error message for iconv error code
	 * @param err_code The iconv error code to translate
	 * @return Human-readable error message string
	 * 
	 * @details This static function provides standardized error messages for
	 *          iconv error codes. It maps common errno values that iconv
	 *          functions return to descriptive text.
	 * 
	 * @par Common Error Codes:
	 * - EINVAL: Invalid encoding names or unsupported conversion
	 * - EILSEQ: Invalid byte sequence in input
	 * - E2BIG: Output buffer too small
	 * - ENOMEM: Memory allocation failure
	 * 
	 * @since 1.0.0.1
	 */
	static std::string GetIconvErrorString(int err_code);

	/**
	 * @brief Get cached or create new iconv descriptor for encoding conversion
	 * @param fromcode Source encoding name
	 * @param tocode Target encoding name
	 * @return Shared pointer to iconv descriptor with automatic cleanup
	 * 
	 * @details This function manages the iconv descriptor cache, providing
	 *          thread-safe access to conversion descriptors. If a descriptor
	 *          for the requested encoding pair exists in cache, it returns
	 *          the cached version. Otherwise, it creates a new descriptor
	 *          and adds it to the cache.
	 * 
	 * @par Caching Benefits:
	 * - Reduces overhead of repeated iconv_open() calls
	 * - Improves performance for repeated conversions
	 * - Automatic cleanup via RAII when no longer needed
	 * - Thread-safe access with mutex protection
	 * 
	 * @par Error Handling:
	 * Returns nullptr if iconv_open() fails (invalid encoding names)
	 * 
	 * @since 1.0.0.1
	 */
	IconvSharedPtr GetIconvDescriptorS(const char* fromcode, const char* tocode);

	/** @} */ // end of Private Helper Functions group

	/** @name Private Constructors
	 *  @brief Singleton pattern implementation details
	 *  @{
	 */

	/**
	 * @brief Private default constructor
	 * @details Ensures the class can only be instantiated through the
	 *          Singleton pattern. The constructor is intentionally private
	 *          and only accessible to the Singleton base class.
	 */
	UniConv() {
	}

	/**
	 * @brief Deleted copy constructor
	 * @details Prevents copying of the singleton instance. Part of the
	 *          singleton pattern implementation.
	 */
	UniConv(const UniConv&) = delete;

	/**
	 * @brief Deleted assignment operator
	 * @details Prevents assignment of the singleton instance. Part of the
	 *          singleton pattern implementation.
	 */
	UniConv& operator=(const UniConv&) = delete;

	/** @} */ // end of Private Constructors group

public:

	/** @name Encoding Constants
	 *  @brief Predefined constants for commonly used character encodings
	 *  @details These constants provide type-safe, compile-time string constants
	 *           for encoding names. Using these constants helps prevent typos
	 *           and provides better IDE support with autocomplete.
	 *  @{
	 */

	/** @name European Language Encodings
	 *  @brief Character encodings for European languages
	 *  @{
	 */

	static constexpr const char* ascii_encoding = "ASCII";                    /*!< @brief 7-bit ASCII encoding */
	static constexpr const char* iso_8859_1_encoding = "ISO-8859-1";         /*!< @brief Latin-1 Western European */
	static constexpr const char* iso_8859_2_encoding = "ISO-8859-2";         /*!< @brief Latin-2 Central European */
	static constexpr const char* iso_8859_3_encoding = "ISO-8859-3";         /*!< @brief Latin-3 South European */
	static constexpr const char* iso_8859_4_encoding = "ISO-8859-4";         /*!< @brief Latin-4 North European */
	static constexpr const char* iso_8859_5_encoding = "ISO-8859-5";         /*!< @brief Latin/Cyrillic */
	static constexpr const char* iso_8859_7_encoding = "ISO-8859-7";         /*!< @brief Latin/Greek */
	static constexpr const char* iso_8859_9_encoding = "ISO-8859-9";         /*!< @brief Latin-5 Turkish */
	static constexpr const char* iso_8859_10_encoding = "ISO-8859-10";       /*!< @brief Latin-6 Nordic */
	static constexpr const char* iso_8859_13_encoding = "ISO-8859-13";       /*!< @brief Latin-7 Baltic */
	static constexpr const char* iso_8859_14_encoding = "ISO-8859-14";       /*!< @brief Latin-8 Celtic */
	static constexpr const char* iso_8859_15_encoding = "ISO-8859-15";       /*!< @brief Latin-9 Western European with Euro */
	static constexpr const char* iso_8859_16_encoding = "ISO-8859-16";       /*!< @brief Latin-10 South-Eastern European */
	static constexpr const char* koi8_r_encoding = "KOI8-R";                 /*!< @brief Russian Cyrillic */
	static constexpr const char* koi8_u_encoding = "KOI8-U";                 /*!< @brief Ukrainian Cyrillic */
	static constexpr const char* koi8_ru_encoding = "KOI8-RU";               /*!< @brief Russian/Ukrainian Cyrillic */

	/** @} */ // end of European Language Encodings

	/** @name Windows Code Pages
	 *  @brief Windows-specific character encodings
	 *  @{
	 */

	static constexpr const char* cp1250_encoding = "CP1250";                 /*!< @brief Windows Central European */
	static constexpr const char* cp1251_encoding = "CP1251";                 /*!< @brief Windows Cyrillic */
	static constexpr const char* cp1252_encoding = "CP1252";                 /*!< @brief Windows Western European */
	static constexpr const char* cp1253_encoding = "CP1253";                 /*!< @brief Windows Greek */
	static constexpr const char* cp1254_encoding = "CP1254";                 /*!< @brief Windows Turkish */
	static constexpr const char* cp1257_encoding = "CP1257";                 /*!< @brief Windows Baltic */
	static constexpr const char* cp850_encoding = "CP850";                   /*!< @brief DOS Latin-1 */
	static constexpr const char* cp866_encoding = "CP866";                   /*!< @brief DOS Russian */
	static constexpr const char* cp1131_encoding = "CP1131";                 /*!< @brief BelarusianCP */

	/** @} */ // end of Windows Code Pages

	/** @name Macintosh Encodings
	 *  @brief Legacy Macintosh character encodings
	 *  @{
	 */

	static constexpr const char* mac_roman_encoding = "MacRoman";             /*!< @brief Macintosh Roman */
	static constexpr const char* mac_central_europe_encoding = "MacCentralEurope"; /*!< @brief Macintosh Central European */
	static constexpr const char* mac_iceland_encoding = "MacIceland";         /*!< @brief Macintosh Icelandic */
	static constexpr const char* mac_croatian_encoding = "MacCroatian";       /*!< @brief Macintosh Croatian */
	static constexpr const char* mac_romania_encoding = "MacRomania";         /*!< @brief Macintosh Romanian */
	static constexpr const char* mac_cyrillic_encoding = "MacCyrillic";       /*!< @brief Macintosh Cyrillic */
	static constexpr const char* mac_ukraine_encoding = "MacUkraine";         /*!< @brief Macintosh Ukrainian */
	static constexpr const char* mac_greek_encoding = "MacGreek";             /*!< @brief Macintosh Greek */
	static constexpr const char* mac_turkish_encoding = "MacTurkish";         /*!< @brief Macintosh Turkish */
	static constexpr const char* macintosh_encoding = "Macintosh";            /*!< @brief Macintosh (alias for MacRoman) */

	/** @} */ // end of Macintosh Encodings

	/** @name Semitic Language Encodings
	 *  @brief Character encodings for Hebrew and Arabic languages
	 *  @{
	 */

	static constexpr const char* iso_8859_6_encoding = "ISO-8859-6";         /*!< @brief Latin/Arabic */
	static constexpr const char* iso_8859_8_encoding = "ISO-8859-8";         /*!< @brief Latin/Hebrew */
	static constexpr const char* cp1255_encoding = "CP1255";                 /*!< @brief Windows Hebrew */
	static constexpr const char* cp1256_encoding = "CP1256";                 /*!< @brief Windows Arabic */
	static constexpr const char* cp862_encoding = "CP862";                   /*!< @brief DOS Hebrew */
	static constexpr const char* mac_hebrew_encoding = "MacHebrew";           /*!< @brief Macintosh Hebrew */
	static constexpr const char* mac_arabic_encoding = "MacArabic";           /*!< @brief Macintosh Arabic */

	/** @} */ // end of Semitic Language Encodings

	/** @name Japanese Encodings
	 *  @brief Character encodings for Japanese language
	 *  @{
	 */

	static constexpr const char* euc_jp_encoding = "EUC-JP";                 /*!< @brief Extended Unix Code for Japanese */
	static constexpr const char* shift_jis_encoding = "SHIFT_JIS";           /*!< @brief Shift JIS Japanese */
	static constexpr const char* cp932_encoding = "CP932";                   /*!< @brief Windows Japanese (Shift_JIS variant) */
	static constexpr const char* iso_2022_jp_encoding = "ISO-2022-JP";       /*!< @brief ISO-2022 Japanese */
	static constexpr const char* iso_2022_jp_2_encoding = "ISO-2022-JP-2";   /*!< @brief ISO-2022 Japanese extended */
	static constexpr const char* iso_2022_jp_1_encoding = "ISO-2022-JP-1";   /*!< @brief ISO-2022 Japanese variant 1 */
	static constexpr const char* iso_2022_jp_ms_encoding = "ISO-2022-JP-MS"; /*!< @brief ISO-2022 Japanese Microsoft variant */

	/** @} */ // end of Japanese Encodings

	/** @name Chinese Encodings
	 *  @brief Character encodings for Chinese languages
	 *  @{
	 */

	static constexpr const char* euc_cn_encoding = "EUC-CN";                 /*!< @brief Extended Unix Code for Chinese (GB2312) */
	static constexpr const char* hz_encoding = "HZ";                         /*!< @brief HZ Chinese encoding */
	static constexpr const char* gbk_encoding = "GBK";                       /*!< @brief Chinese National Standard (extension of GB2312) */
	static constexpr const char* cp936_encoding = "CP936";                   /*!< @brief Windows Simplified Chinese (GBK) */
	static constexpr const char* gb18030_encoding = "GB18030";               /*!< @brief Chinese National Standard GB18030 */
	static constexpr const char* gb18030_2022_encoding = "GB18030:2022";     /*!< @brief GB18030 2022 revision */
	static constexpr const char* euc_tw_encoding = "EUC-TW";                 /*!< @brief Extended Unix Code for Traditional Chinese */
	static constexpr const char* big5_encoding = "BIG5";                     /*!< @brief Big5 Traditional Chinese */
	static constexpr const char* cp950_encoding = "CP950";                   /*!< @brief Windows Traditional Chinese (Big5) */
	static constexpr const char* big5_hkscs_encoding = "BIG5-HKSCS";         /*!< @brief Big5 with Hong Kong extensions */
	static constexpr const char* big5_hkscs_2004_encoding = "BIG5-HKSCS:2004"; /*!< @brief Big5 HKSCS 2004 */
	static constexpr const char* big5_hkscs_2001_encoding = "BIG5-HKSCS:2001"; /*!< @brief Big5 HKSCS 2001 */
	static constexpr const char* big5_hkscs_1999_encoding = "BIG5-HKSCS:1999"; /*!< @brief Big5 HKSCS 1999 */
	static constexpr const char* iso_2022_cn_encoding = "ISO-2022-CN";       /*!< @brief ISO-2022 Chinese */
	static constexpr const char* iso_2022_cn_ext_encoding = "ISO-2022-CN-EXT"; /*!< @brief ISO-2022 Chinese extended */

	/** @} */ // end of Chinese Encodings

	/** @name Korean Encodings
	 *  @brief Character encodings for Korean language
	 *  @{
	 */

	static constexpr const char* euc_kr_encoding = "EUC-KR";                 /*!< @brief Extended Unix Code for Korean */
	static constexpr const char* cp949_encoding = "CP949";                   /*!< @brief Windows Korean (Unified Hangul Code) */
	static constexpr const char* iso_2022_kr_encoding = "ISO-2022-KR";       /*!< @brief ISO-2022 Korean */
	static constexpr const char* johab_encoding = "JOHAB";                   /*!< @brief Korean JOHAB encoding */

	/** @} */ // end of Korean Encodings

	/** @name Other Language Encodings
	 *  @brief Character encodings for various other languages
	 *  @{
	 */

	static constexpr const char* armscii_8_encoding = "ARMSCII-8";           /*!< @brief Armenian ARMSCII-8 */
	static constexpr const char* georgian_academy_encoding = "Georgian-Academy"; /*!< @brief Georgian Academy encoding */
	static constexpr const char* georgian_ps_encoding = "Georgian-PS";       /*!< @brief Georgian PS encoding */
	static constexpr const char* koi8_t_encoding = "KOI8-T";                 /*!< @brief Tajik KOI8-T */
	static constexpr const char* pt154_encoding = "PT154";                   /*!< @brief Kazakh PT154 */
	static constexpr const char* rk1048_encoding = "RK1048";                 /*!< @brief Kazakh RK1048 */
	static constexpr const char* tis_620_encoding = "TIS-620";               /*!< @brief Thai TIS-620 */
	static constexpr const char* cp874_encoding = "CP874";                   /*!< @brief Windows Thai */
	static constexpr const char* mac_thai_encoding = "MacThai";               /*!< @brief Macintosh Thai */
	static constexpr const char* mulelao_1_encoding = "MuleLao-1";           /*!< @brief Laotian MuleLao-1 */
	static constexpr const char* cp1133_encoding = "CP1133";                 /*!< @brief IBM Laotian */
	static constexpr const char* viscii_encoding = "VISCII";                 /*!< @brief Vietnamese VISCII */
	static constexpr const char* tcvn_encoding = "TCVN";                     /*!< @brief Vietnamese TCVN */
	static constexpr const char* cp1258_encoding = "CP1258";                 /*!< @brief Windows Vietnamese */

	/** @} */ // end of Other Language Encodings

	/** @name Platform-Specific Encodings
	 *  @brief Encodings specific to certain platforms or systems
	 *  @{
	 */

	static constexpr const char* hp_roman8_encoding = "HP-ROMAN8";           /*!< @brief HP Roman-8 encoding */
	static constexpr const char* nextstep_encoding = "NEXTSTEP";             /*!< @brief NeXTSTEP encoding */

	/** @} */ // end of Platform-Specific Encodings

	/** @name Unicode Encodings
	 *  @brief Full Unicode character encodings
	 *  @{
	 */

	static constexpr const char* utf_8_encoding = "UTF-8";                   /*!< @brief UTF-8 Unicode */
	static constexpr const char* ucs_2_encoding = "UCS-2";                   /*!< @brief UCS-2 Unicode (16-bit) */
	static constexpr const char* ucs_2be_encoding = "UCS-2BE";               /*!< @brief UCS-2 Big Endian */
	static constexpr const char* ucs_2le_encoding = "UCS-2LE";               /*!< @brief UCS-2 Little Endian */
	static constexpr const char* ucs_4_encoding = "UCS-4";                   /*!< @brief UCS-4 Unicode (32-bit) */
	static constexpr const char* ucs_4be_encoding = "UCS-4BE";               /*!< @brief UCS-4 Big Endian */
	static constexpr const char* ucs_4le_encoding = "UCS-4LE";               /*!< @brief UCS-4 Little Endian */
	static constexpr const char* utf_16_encoding = "UTF-16";                 /*!< @brief UTF-16 Unicode */
	static constexpr const char* utf_16be_encoding = "UTF-16BE";             /*!< @brief UTF-16 Big Endian */
	static constexpr const char* utf_16le_encoding = "UTF-16LE";             /*!< @brief UTF-16 Little Endian */
	static constexpr const char* utf_32_encoding = "UTF-32";                 /*!< @brief UTF-32 Unicode */
	static constexpr const char* utf_32be_encoding = "UTF-32BE";             /*!< @brief UTF-32 Big Endian */
	static constexpr const char* utf_32le_encoding = "UTF-32LE";             /*!< @brief UTF-32 Little Endian */
	static constexpr const char* utf_7_encoding = "UTF-7";                   /*!< @brief UTF-7 Unicode */

	/** @} */ // end of Unicode Encodings

	/** @name Locale-Dependent Encodings
	 *  @brief Platform and locale-dependent character encodings
	 *  @{
	 */

	static constexpr const char* char_encoding = "char";                     /*!< @brief Locale-dependent char encoding */
	static constexpr const char* wchar_t_encoding = "wchar_t";               /*!< @brief Locale-dependent wide char encoding */

	/** @} */ // end of Locale-Dependent Encodings

	/** @name EBCDIC Encodings
	 *  @brief IBM EBCDIC character encodings (mainframe systems)
	 *  @details EBCDIC encodings are not ASCII-compatible and are primarily
	 *           used on IBM mainframe systems. These are rarely used in
	 *           modern applications but may be needed for legacy data processing.
	 *  @{
	 */	static constexpr const char* ibm_037_encoding = "IBM-037";               /*!< @brief IBM EBCDIC US-Canada */
	static constexpr const char* ibm_273_encoding = "IBM-273";               /*!< @brief IBM EBCDIC Germany */
	static constexpr const char* ibm_277_encoding = "IBM-277";               /*!< @brief IBM EBCDIC Denmark-Norway */
	static constexpr const char* ibm_278_encoding = "IBM-278";               /*!< @brief IBM EBCDIC Finland-Sweden */
	static constexpr const char* ibm_280_encoding = "IBM-280";               /*!< @brief IBM EBCDIC Italy */
	static constexpr const char* ibm_282_encoding = "IBM-282";               /*!< @brief IBM EBCDIC Portugal */
	static constexpr const char* ibm_284_encoding = "IBM-284";               /*!< @brief IBM EBCDIC Spain */
	static constexpr const char* ibm_285_encoding = "IBM-285";               /*!< @brief IBM EBCDIC United Kingdom */
	static constexpr const char* ibm_297_encoding = "IBM-297";               /*!< @brief IBM EBCDIC France */
	static constexpr const char* ibm_423_encoding = "IBM-423";               /*!< @brief IBM EBCDIC Greek */
	static constexpr const char* ibm_500_encoding = "IBM-500";               /*!< @brief IBM EBCDIC International */
	static constexpr const char* ibm_870_encoding = "IBM-870";               /*!< @brief IBM EBCDIC Multilingual/ROECE (Latin-2) */
	static constexpr const char* ibm_871_encoding = "IBM-871";               /*!< @brief IBM EBCDIC Iceland */
	static constexpr const char* ibm_875_encoding = "IBM-875";               /*!< @brief IBM EBCDIC Greek Modern */
	static constexpr const char* ibm_880_encoding = "IBM-880";               /*!< @brief IBM EBCDIC Cyrillic (Russian) */
	static constexpr const char* ibm_905_encoding = "IBM-905";               /*!< @brief IBM EBCDIC Turkish */
	static constexpr const char* ibm_924_encoding = "IBM-924";               /*!< @brief IBM EBCDIC Latin-9--Euro */
	static constexpr const char* ibm_1025_encoding = "IBM-1025";             /*!< @brief IBM EBCDIC Cyrillic (Serbian, Bulgarian) */
	static constexpr const char* ibm_1026_encoding = "IBM-1026";             /*!< @brief IBM EBCDIC Turkish (Latin-5) */
	static constexpr const char* ibm_1047_encoding = "IBM-1047";             /*!< @brief IBM EBCDIC Latin-1/Open System */
	static constexpr const char* ibm_1112_encoding = "IBM-1112";             /*!< @brief IBM EBCDIC Estonia, Latvia, Lithuania */
	static constexpr const char* ibm_1122_encoding = "IBM-1122";             /*!< @brief IBM EBCDIC Estonia */
	static constexpr const char* ibm_1123_encoding = "IBM-1123";             /*!< @brief IBM EBCDIC Ukraine */
	static constexpr const char* ibm_1140_encoding = "IBM-1140";             /*!< @brief IBM EBCDIC US-Canada (037 + Euro symbol) */
	static constexpr const char* ibm_1141_encoding = "IBM-1141";             /*!< @brief IBM EBCDIC Germany (20273 + Euro symbol) */
	static constexpr const char* ibm_1142_encoding = "IBM-1142";             /*!< @brief IBM EBCDIC Denmark-Norway (20277 + Euro symbol) */
	static constexpr const char* ibm_1143_encoding = "IBM-1143";             /*!< @brief IBM EBCDIC Finland-Sweden (20278 + Euro symbol) */
	static constexpr const char* ibm_1144_encoding = "IBM-1144";             /*!< @brief IBM EBCDIC Italy (20280 + Euro symbol) */
	static constexpr const char* ibm_1145_encoding = "IBM-1145";             /*!< @brief IBM EBCDIC Spain (20284 + Euro symbol) */
	static constexpr const char* ibm_1146_encoding = "IBM-1146";             /*!< @brief IBM EBCDIC United Kingdom (20285 + Euro symbol) */
	static constexpr const char* ibm_1147_encoding = "IBM-1147";             /*!< @brief IBM EBCDIC France (20297 + Euro symbol) */
	static constexpr const char* ibm_1148_encoding = "IBM-1148";             /*!< @brief IBM EBCDIC International (500 + Euro symbol) */
	static constexpr const char* ibm_1149_encoding = "IBM-1149";             /*!< @brief IBM EBCDIC Iceland (20871 + Euro symbol) */
	static constexpr const char* ibm_1153_encoding = "IBM-1153";             /*!< @brief IBM EBCDIC Latin-2 (20870 + Euro symbol) */
	static constexpr const char* ibm_1154_encoding = "IBM-1154";             /*!< @brief IBM EBCDIC Cyrillic (20880 + Euro symbol) */
	static constexpr const char* ibm_1155_encoding = "IBM-1155";             /*!< @brief IBM EBCDIC Turkey (20905 + Euro symbol) */
	static constexpr const char* ibm_1156_encoding = "IBM-1156";             /*!< @brief IBM EBCDIC Baltic */
	static constexpr const char* ibm_1157_encoding = "IBM-1157";             /*!< @brief IBM EBCDIC Estonia */
	static constexpr const char* ibm_1158_encoding = "IBM-1158";             /*!< @brief IBM EBCDIC Ukraine */
	static constexpr const char* ibm_1165_encoding = "IBM-1165";             /*!< @brief IBM EBCDIC Latin-2 */
	static constexpr const char* ibm_1166_encoding = "IBM-1166";             /*!< @brief IBM EBCDIC Cyrillic */
	static constexpr const char* ibm_4971_encoding = "IBM-4971";             /*!< @brief IBM EBCDIC Greek */
	static constexpr const char* ibm_424_encoding = "IBM-424";               /*!< @brief IBM EBCDIC Hebrew */
	static constexpr const char* ibm_425_encoding = "IBM-425";               /*!< @brief IBM EBCDIC Arabic */
	static constexpr const char* ibm_12712_encoding = "IBM-12712";           /*!< @brief IBM EBCDIC Hebrew */
	static constexpr const char* ibm_16804_encoding = "IBM-16804";           /*!< @brief IBM EBCDIC Arabic */
	static constexpr const char* ibm_1097_encoding = "IBM-1097";             /*!< @brief IBM EBCDIC Farsi */
	static constexpr const char* ibm_838_encoding = "IBM-838";               /*!< @brief IBM EBCDIC Thai */
	static constexpr const char* ibm_1160_encoding = "IBM-1160";             /*!< @brief IBM EBCDIC Thai */
	static constexpr const char* ibm_1132_encoding = "IBM-1132";             /*!< @brief IBM EBCDIC Lao */
	static constexpr const char* ibm_1130_encoding = "IBM-1130";             /*!< @brief IBM EBCDIC Vietnamese */
	static constexpr const char* ibm_1164_encoding = "IBM-1164";             /*!< @brief IBM EBCDIC Vietnamese */
	static constexpr const char* ibm_1137_encoding = "IBM-1137";             /*!< @brief IBM EBCDIC Devanagari */

	/** @} */ // end of EBCDIC Encodings

	/** @} */ // end of Encoding Constants group

};

#endif // UNICONV_H__

