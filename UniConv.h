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
*  @brief    UniConv A c++ library for variable encoding conversion
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
*  2025/03/10 | 1.0.0.1   | hesphoros      | Create file
*****************************************************************************/

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#ifndef __UNICONV_H__
#define __UNICONV_H__

#include <iconv.h>
#include <iostream>
#include <string>
#include <malloc.h>
#include <unordered_map>
#include <cwchar>
#include <clocale>
#include <sstream>
#include <utility>
#include <vector>
#include <string_view>
#include <cerrno>
#include <fstream>
#include <cstring>
#include <system_error>
#include <mutex>
#include <shared_mutex>
#include <memory>
#include <algorithm>
#include <functional>
#include <io.h>
#include <fcntl.h>




#ifdef _WIN32
#include <windows.h>
#endif // _WIN32

#ifdef __linux__
#include <unistd.h>
#include <langinfo.h>
#endif // __linux__


#ifdef UNICONV_DLL
#define UNICONV_EXPORT __declspec(dllexport)
#else
#define UNICONV_EXPORT
#endif

#if defined(_DEBUG) || !defined(NDEBUG)   // Debug Mode
#define UNICONV_DEBUG_MODE 1
#else                                     // Release Mode
#define UNICONV_DEBUG_MODE 0
#endif

// C++ standard version detection
#if defined(_MSC_VER)
// MSVC compiler uses _MSVC_LANG to indicate the C++ standard version
#define CPP_STANDARD _MSVC_LANG
#else
#define CPP_STANDARD __cplusplus
#endif

// Make sure the C++ standard version is at least C++11
static_assert(CPP_STANDARD >= 201103L, "Error: This code requires C++11 or later");

/**
 * @brief Get the current C++ standard version as a string.
 * @return A string_view representing the current C++ standard version.
 * @retval "C++20 or later" if the standard is C++20 or later.
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


template <typename T>
class Singleton {
protected:

	Singleton() = default;
	Singleton(const Singleton<T>&) = delete;
	Singleton& operator=(const Singleton<T>& st) = delete;

	static std::shared_ptr<T> _instance;
public:
	static std::shared_ptr<T> GetInstance() {
		static std::once_flag s_flag;
		std::call_once(s_flag, [&]() {
			_instance = std::shared_ptr<T>(new T);
			});

		return _instance;
	}
	~Singleton() {
		//std::cout << "this is singleton destruct" << std::endl;
	}
};

template <typename T>
std::shared_ptr<T> Singleton<T>::_instance = nullptr;


class EncodingNames;


class UNICONV_EXPORT UniConv : public Singleton<UniConv>
{

	/**
	 * @brief Singleton class for UniConv.
	 */
	friend class Singleton<UniConv>;

private:
	/**
	 * @brief Custom deleter for iconv_t to ensure proper cleanup.
	 * * This deleter will be used with std::shared_ptr to manage the iconv_t resource.
	 */
	struct IconvDeleter {
		void operator()(iconv_t cd) const {
			std::cerr << "Closing iconv_t: " << cd << std::endl;
			// call iconv_close to release the iconv descriptor only if it is valid
			if (cd != reinterpret_cast<iconv_t>(-1)) {
				iconv_close(cd);
			}
		}
	};

	/*using IconvUniquePtr = std::unique_ptr <std::remove_pointer<iconv_t>::type, UniConv::IconvDeleter>;*/
	using IconvSharedPtr = std::shared_ptr <std::remove_pointer<iconv_t>::type>;

	/**
	 * @struct EncodingInfo
	 * @brief Structure to hold encoding information.
	 * @details This structure holds information about a specific text encoding.
	 */
	struct EncodingInfo
	{
		std::string dotNetName;  /*!< .NET encoding name */
		std::string extra_info;	 /*!< Extra information */
	};

public:

   void SetDefaultEncoding(const std::string& encoding);

	//---------------------------------------------------------------------------
	// Bom encodings @{
	//---------------------------------------------------------------------------
		/**
		 * @brief Enumeration representing different Byte Order Mark (BOM) encodings
		 *
		 * This enum class defines the various BOM (Byte Order Mark) encoding types
		 * that can be detected or used in Unicode text processing. BOM is a special
		 * marker at the beginning of a text file that indicates the encoding format
		 * and byte order of the text.
		 *
		 * @note This is a scoped enumeration (enum class) to prevent name collisions
		 *       and provide type safety.
		 */
	enum class BomEncoding {
		None,
		UTF8,
		UTF16_LE,
		UTF16_BE,
		UTF32_LE,
		UTF32_BE
	};
	//---------------------------------------------------------------------------
	// @} End of Bom encoding
	//---------------------------------------------------------------------------



	//---------------------------------------------------------------------------
	// Supported encodings @{
	//---------------------------------------------------------------------------
        /**
         * @enum Encoding
         * @brief Enumeration of supported encoding types
         * @details
         * This enumeration defines all supported encoding types.
         * Enumeration values ​​are automatically generated by including the "encodings.inc" file.
         * Each enumeration member represents an encoding method.
         * Its order and content are consistent with the "encodings.inc" file .
         * The last member count is used to indicate the number of encoding types.
         * @see https://learn.microsoft.com/zh-cn/windows/win32/intl/code-page-identifiers
        */
        enum class Encoding : int {
        #define X(name, str) name,
        #include "encodings.inc"
        #undef X
        count
        };
	//---------------------------------------------------------------------------
	//@} End of Supported encodings
	//---------------------------------------------------------------------------


	/**
	 * @struct IConvResult
	 * @brief Structure to hold the result of a conversion operation.
	 * @note IConvResult
	 */
	struct IConvResult {
		std::string        conv_result_str;  /*!< Conversion result string */
		int                error_code;       /*!< Error code               */
		std::string        error_msg;        /*!< Error message            */

		IConvResult() {
			/** Initialize the members */
			conv_result_str = "";
			error_code = 0;
			error_msg = "";
		}
		/**
		 * @brief Check if the conversion was successful.
		 * @return True if the conversion was successful, false otherwise.
		 * @return bool
		 */
		bool IsSuccess() const {
			return error_code == 0;
		}

		explicit operator bool() const {
			return IsSuccess();
		}
		bool operator!() const {
			return !IsSuccess();
		}
		bool operator==(int code) const {
			return error_code == code;
		}

		bool operator!=(int code) const {
			return error_code != code;
		}

		const char* c_str() const {
			return IsSuccess() ? conv_result_str.c_str() : error_msg.data();
		}
	};


	~UniConv() {
	}
/** Test Success */
/***************************************************************************/
/*========================= Get current encoding ==========================*/
/***************************************************************************/
	/**
	 * @brief Get current system encoding.
	 * @return A string representing the current system encoding.
	 * @retval "UTF-8" if the system encoding is UTF-8.
	 * @retval "UTF-16LE" if the system encoding is UTF-16LE.
	 * @note finished test on windows
	 */
	static std::string     GetCurrentSystemEncoding();


	/**
	 * @brief Get current system encoding code page.
	 * @return The code page of the current system encoding.
	 * @retval 0 if the code page cannot be determined.
	 * @retval -1  it not on windows or Linux
	 * @note finished test on windows
	 */
	static std::uint16_t   GetCurrentSystemEncodingCodePage();

	/**
	 * @brief Get the encoding name by code page.
	 * @param  codePage The code page to look up.
	 * @return The name of the encoding corresponding to the given code page.
	 * @retval std::string
	 * @note finished test on windows
	 */
	static std::string     GetEncodingNameByCodePage(std::uint16_t codePage);

/** Test Success */ 
/***************************************************************************/
/*=================== Locale <-> UTF-8 Conversion Interface =========================*/
/***************************************************************************/
	/**
	 * @brief Convert a string from system local encoding to UTF-8 string
	 * @param  input System local encoding input string
	 * @return Converted UTF-8 string
	 * @todo test
	 */
	std::string ToUtf8FromLocale(const std::string& input);

	/**
	 * @brief Convert a C string from system local encoding to UTF-8 string
	 * @param  input System local encoding C string
	 * @return Converted UTF-8 string
	 * @todo test
	 */
	std::string ToUtf8FromLocale(const char* input);

	/**
	 * @brief Convert UTF-8 string to system local encoding string
	 * @param  input UTF-8 encoded input string
	 * @return Converted system local encoding string
	 */
	std::string ToLocaleFromUtf8(const std::string& input);

	/**
	 * @brief Convert UTF-8 C string to system local encoding string
	 * @param  input UTF-8 encoded C string
	 * @return Converted system local encoding string
	 */
	std::string ToLocaleFromUtf8(const char* input);
/** Test Success */
/***************************************************************************/
/*=================== Locale convert to UTF-16 (LE BE) ====================*/
/***************************************************************************/
	/**
	 * @brief Convert a string from the current locale encoding to UTF-16LE.
	 * @param  input The input string to be converted.
	 * @return The converted string in UTF-16LE encoding.
	 */
	std::u16string ToUtf16LEFromLocale(const std::string& input);

	/**
	 * @brief Convert a C-style string from the current locale encoding to UTF-16LE.
	 * @param input The C-style string to be converted.
	 * @return The converted string in UTF-16LE encoding.
	 */
	std::u16string ToUtf16LEFromLocale(const char* input);
	/**
	 * @brief Convert a string from the current locale encoding to UTF-16BE.
	 * @param  input The input string to be converted.
	 * @return The converted string in UTF-16BE encoding.
	 */
	std::u16string ToUtf16BEFromLocale(const std::string& input);
	/**
	 * @brief Convert a C-style string from the current locale encoding to UTF-16BE.
	 * @param input The C-style string to be converted.
	 * @return The converted string in UTF-16BE encoding.
	 */
	std::u16string ToUtf16BEFromLocale(const char* input);

/** Test Success */
/***************************************************************************/
/*========================== UTF-16 BE To Locale ==========================*/
/***************************************************************************/
	/**
	 * @brief Convert a UTF-16BE encoded string to the current locale encoding.
	 * @param  input The UTF-16BE encoded string to be converted.
	 * @return The converted string in the current locale encoding.
	 */
	std::string ToLocaleFromUtf16BE(const std::u16string& input);

	/**
	 * @brief Convert a C-style UTF-16BE string to the current locale encoding.
	 * @param input The C-style UTF-16BE string to be converted.
	 * @return The converted string in the current locale encoding.
	 */
	std::string ToLocaleFromUtf16BE(const char16_t* input);

/** Test Success */
/***************************************************************************/
/*======================== UTF-16 (LE BE) To UTF-8 ========================*/
/***************************************************************************/
	/**
	 * @brief Convert a UTF-16LE string to UTF-8.
	 * @param input The UTF-16LE string to be converted.
	 * @return The converted string in UTF-8 encoding.
	 */
	std::string ToUtf8FromUtf16LE(const std::u16string& input);

	// Overload with length parameter
	std::string ToUtf8FromUtf16LE(const char16_t* input, size_t len);

	/**
	 * @brief Convert a C-style UTF-16LE string to UTF-8.
	 * @param input The C-style UTF-16LE string to be converted.
	 * @return The converted string in UTF-8 encoding.
	 */
	std::string ToUtf8FromUtf16LE(const char16_t* input);

	/**
	 * @brief Convert a UTF-16BE string to UTF-8.
	 * @param sInput The UTF-16BE string to be converted.
	 * @return The converted string in UTF-8 encoding.
	 * @retval val std::string
	 */
	/**
	 * @brief
	 * @param  input UTF-16BE
	 * @return
	 */
	std::string ToUtf8FromUtf16BE(const std::u16string& input);

	std::string ToUtf8FromUtf16BE(const char16_t* input, size_t len);

	/**
	 * @brief Convert a C-style UTF-16BE string to UTF-8.
	 * @param sInput The C-style UTF-16BE string to be converted.
	 * @return The converted string in UTF-8 encoding.
	 * @retval val std::string
	 */
	/**
	 * @brief UTF-16BE -8
	 * @param  input UTF-16BE C
	 * @return
	 */
	std::string ToUtf8FromUtf16BE(const char16_t* input);

/** Test Success */
/***************************************************************************/
/*========================== UTF-8 To UTF16(LE BE) ========================*/
/***************************************************************************/
	/**
	 * @brief Convert a UTF-8 string to UTF-16LE.
	 * @param sInput The UTF-8 string to be converted.
	 * @return The converted string in UTF-16LE encoding.
	 * @retval val std::u16string
	 */
	/**
	 * @brief
	 * @param  input UTF-8
	 * @return
	 */
	std::u16string ToUtf16LEFromUtf8(const std::string& input);

	/**
	 * @brief Convert a C-style UTF-8 string to UTF-16LE.
	 * @param sInput The C-style UTF-8 string to be converted.
	 * @return The converted string in UTF-16LE encoding.
	 * @retval val std::u16string
	 */
	/**
	 * @brief
	 * @param  input UTF-8 C
	 * @return
	 */
	std::u16string ToUtf16LEFromUtf8(const char* input);

	/**
	 * @brief Convert a UTF-8 string to UTF-16BE.
	 * @param sInput The UTF-8 string to be converted.
	 * @return The converted string in UTF-16BE encoding.
	 * @retval val std::u16string
	 */
	/**
	 * @brief
	 * @param  input UTF-8
	 */
	std::u16string ToUtf16BEFromUtf8(const std::string& input);

	/**
	 * @brief Convert a C-style UTF-8 string to UTF-16BE.
	 * @param sInput The C-style UTF-8 string to be converted.
	 * @return The converted string in UTF-16BE encoding.
	 * @retval val std::u16string
	 */
	/**
	 * @brief
	 * @param  input UTF-8 C
	 * @return
	 */
	std::u16string ToUtf16BEFromUtf8(const char* input);

/** Test Success */
/***************************************************************************/
/*====================== UTF16 LE BE <-> UTF16 LE BE ======================*/
/***************************************************************************/

	/**
	 * @brief Convert a UTF-16LE string to UTF-16BE.
	 * @param sInput The UTF-16LE string to be converted.
	 * @return The converted string in UTF-16BE encoding.
	 * @retval val std::u16string
	 */
	/**
	 * @brief Utf16le to utf16be
	 * @param  input UTF-16LE
	 * @return
	 */
	std::u16string ToUtf16BEFromUtf16LE(const std::u16string& input);

	/**
	 * @brief Convert a C-style UTF-16LE string to UTF-16BE.
	 * @param sInput The C-style UTF-16LE string to be converted.
	 * @return The converted string in UTF-16BE encoding.
	 * @retval val std::u16string
	 */
	/**
	 * @brief
	 * @param  input UTF-16LE C
	 * @return
	 */
	std::u16string ToUtf16BEFromUtf16LE(const char16_t* input);

	/**
	 * @brief Convert a UTF-16BE string to UTF-16LE.
	 * @param sInput The UTF-16BE string to be converted.
	 * @return The converted string in UTF-16LE encoding.
	 * @retval val std::u16string
	 */
	/**
	 * @brief
	 * @param  input UTF-16BE
	 * @return
	 */
	std::u16string ToUtf16LEFromUtf16BE(const std::u16string& input);

	/**
	 * @brief Convert a C-style UTF-16BE string to UTF-16LE.
	 * @param sInput The C-style UTF-16BE string to be converted.
	 * @return The converted string in UTF-16LE encoding.
	 * @retval val std::u16string
	 */
	/**
	 * @brief
	 * @param  input UTF-16BE C
	 * @return
	 */
	std::u16string ToUtf16LEFromUtf16BE(const char16_t* input);

/*Test Success */
/***************************************************************************/
/*========================= string <-> wstring ============================*/
/***************************************************************************/

	/**
	 * @brief Convert a string to a wide string.
	 * @param sInput The input string to be converted.
	 * @return The converted wide string.
	 * @retval val std::wstring
	 */
	std::wstring         LocaleToWideString(const std::string& sInput);

	/**
	 * @brief Convert a C-style string to a wide string.
	 * @param sInput The C-style string to be converted.
	 * @return The converted wide string.
	 * @retval val std::wstring
	 */
	std::wstring         LocaleToWideString(const char* sInput);

	/**	
	 * @brief Convert a wide string to a string in the current locale encoding.
	 * @param sInput The wide string to be converted.
	 * @return The converted string in the current locale encoding.
	 * @retval val std::string
	 */
	std::string          LocaleToNarrowString(const std::wstring& sInput);

	/**
	 * @brief Convert a C-style wide string to a string in the current locale encoding.
	 * @param sInput The C-style wide string to be converted.
	 * @return The converted string in the current locale encoding.
	 * @retval val std::string
	 */
	std::string          LocaleToNarrowString(const wchar_t* sInput);

/** Test Suceess */
/***************************************************************************/
/*===================== UTF16 <-> Local Encoding =======================*/
/***************************************************************************/
	/**
	 * @brief Convert UTF-16LE string to local encoding.
	 * @param input UTF-16LE string to convert.
	 * @return Converted string in local encoding.
	 */
	std::string ToLocalFromUtf16LE(const std::u16string& input);

	/**
	 * @brief Convert UTF-16LE C-style string to local encoding.
	 * @param input UTF-16LE C-style string to convert.
	 * @return Converted string in local encoding.
	 */
	std::string ToLocalFromUtf16LE(const char16_t* input);

/***************************************************************************/
/*======================= Wide String Helpers ===========================*/
/***************************************************************************/
	/**
	 * @brief Convert wide string to local encoding.
	 * @param sInput Wide string to convert.
	 * @return Converted string in local encoding.
	 */
	std::string WideStringToLocale(const std::wstring& sInput);

	/**
	 * @brief Convert wide C-style string to local encoding.
	 * @param sInput Wide C-style string to convert.
	 * @return Converted string in local encoding.
	 */
	std::string WideStringToLocale(const wchar_t* sInput);

/***************************************************************************/
/*=========================== UTF32 <=> other ============================*/
/***************************************************************************/
	/**
	 * @brief Convert a UTF-32 string to UTF-8.
	 * @param sInput The UTF-32 string to be converted.
	 * @return The converted UTF-8 string.
	 * @retval val std::string
	 */
	std::string          ToUtf8FromUtf32LE(const std::u32string& sInput);
	std::string          ToUtf8FromUtf32LE(const char32_t* sInput);


	/**
	 * @brief Convert a UTF-32 string to UTF-16LE.
	 * @param sInput The UTF-32 string to be converted.
	 * @return The converted UTF-16LE string.
	 * @retval val std::u16string
	 */
	std::u16string       ToUtf16LEFromUtf32LE(const std::u32string& sInput);
	std::u16string       ToUtf16LEFromUtf32LE(const char32_t* sInput);

	/**
	 * @brief Convert a UTF-32 string to UTF-16BE.
	 * @param sInput The UTF-32 string to be converted.
	 * @return The converted UTF-16BE string.
	 * @retval val std::u16string
	 */
	std::u16string       Utf32LEConvertToUtf16BE(const std::u32string& sInput);
	std::u16string       Utf32LEConvertToUtf16BE(const char32_t* sInput);

	/**
	 * @brief Convert UTF-8 string to UTF-32.
	 * @param sInput The UTF-8 string to be converted.
	 * @return The converted UTF-32 string.
	 * @retval val std::u32string
	 */
	std::u32string       Utf8ConvertToUtf32LE(const std::string& sInput);
	std::u32string       Utf8ConvertToUtf32LE(const char* sInput);

	/**
	 * @brief Convert a UTF-16LE string to UTF-32.
	 * @param sInput The UTF-16LE string to be converted.
	 * @return The converted UTF-32 string.
	 * @retval val std::u32string
	 */
	std::u32string       Utf16LEConvertToUtf32LE(const std::u16string& sInput);
	std::u32string       Utf16LEConvertToUtf32LE(const char16_t* sInput);

	/**
	 * @brief Convert a UTF-16BE string to UTF-32.
	 * @param sInput The UTF-16BE string to be converted.
	 * @return The converted UTF-32 string.
	 * @retval val std::u32string
	 */
	std::u32string       Utf16BEConvertToUtf32LE(const std::u16string& sInput);
	std::u32string       Utf16BEConvertToUtf32LE(const char16_t* sInput);

	/**
    * @brief Convert a UCS-4 (std::wstring, platform dependent) string to UTF-8 encoded std::string.
    * @details This function converts a wide string (std::wstring), which is typically UCS-4 on Linux (wchar_t = 4 bytes)
    *          and UTF-16 on Windows (wchar_t = 2 bytes), to a UTF-8 encoded std::string.
    *          The conversion assumes the input is UCS-4 (i.e., each wchar_t is a Unicode code point).
    * @param wstr The input wide string (UCS-4 encoded).
    * @return The converted UTF-8 encoded string.
    */
	std::string          Ucs4ConvertToUtf8(const std::wstring& wstr);

	std::wstring         Utf8ConvertsToUcs4(const std::string& utf8str);


	std::wstring         U16StringToWString(const std::u16string& u16str);
	std::wstring		 U16StringToWString(const char16_t* u16str);

	/**
	 * @brief Encoding to string
	 *
	 */
	static std::string ToString(UniConv::Encoding enc);

	/**
	 * @brief Convert between any two encodings using iconv
	 * @param input Input string data
	 * @param fromEncoding Source encoding name
	 * @param toEncoding Target encoding name
	 * @return Conversion result
	 */
	IConvResult             ConvertEncoding(const std::string& input, const char* fromEncoding, const char* toEncoding);

private:

	//----------------------------------------------------------------------------------------------------------------------
	// Private members @{
	//----------------------------------------------------------------------------------------------------------------------
	static const std::unordered_map<std::uint16_t,EncodingInfo>  m_encodingMap;                /*!< Encoding map           */
	static const std::unordered_map<std::string,std::uint16_t>   m_encodingToCodePageMap;      /*!< Iconv code page map    */
	mutable std::shared_mutex                                    m_iconvCacheMutex;            /*!< Iconv cache mutex      */
	static const std::unordered_map<int,std::string_view>        m_iconvErrorMap;              /*!< Iconv error messages   */
	static std::unordered_map<std::string, IconvSharedPtr>       m_iconvDescriptorCacheMapS;   /*!< Iconv descriptor cache */
	static constexpr size_t                                      MAX_CACHE_SIZE = 100;         /*!< Iconv max cache number */
	static const  std::string                                    m_encodingNames[];            /*!< Encoding map           */
	static std::string                                           m_defaultEncoding;            /*!< Current encoding       */
	//----------------------------------------------------------------------------------------------------------------------
	/// @} ! Private members
	//----------------------------------------------------------------------------------------------------------------------

private:


	/**
	 * @brief Retrieves a descriptive error string corresponding to an iconv error code.
	 *
	 * This function maps the provided error code from the iconv library to a human-readable
	 * string that describes the error. It is useful for debugging and logging purposes
	 * when working with character encoding conversions.
	 * @param err_code The error code returned by the iconv library.
	 * @return A string containing the description of the error associated with the given error code.
	 * @retval If the error code is not found in the map,return a generic error message
	 */
	static std::string                  GetIconvErrorString(int err_code);

	/**
	 * @brief Get the iconv descriptor.
	 * @param fromcode The source encoding.
	 * @param tocode The target encoding.
	 * @return The iconv descriptor as an IconvSharedPtr.
	 */
	IconvSharedPtr                            GetIconvDescriptor(const char* fromcode, const char* tocode);

	std::pair<BomEncoding, std::string_view>  DetectAndRemoveBom(const std::string_view& data);
	std::pair<BomEncoding, std::wstring_view> DetectAndRemoveBom(const std::wstring_view& data);

private:
	UniConv() { }

	UniConv(const UniConv&) = delete;
	UniConv& operator=(const UniConv&) = delete;
public:

};

#endif // UNICONV_H__

