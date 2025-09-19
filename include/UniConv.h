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
*  @version  2.0.0.1
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

#include "iconv.h"
#include <iostream>
#include <string>
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
#include <cstdlib>        // 替换 malloc.h
#include <type_traits>    // for CompactResult

#ifdef _WIN32
#include <io.h>           // 仅在 Windows 需要时包含
#include <fcntl.h>
#endif




#ifdef _WIN32
#include <windows.h>
#endif // _WIN32

#ifdef __linux__
#include <unistd.h>
#include <langinfo.h>
#endif // __linux__


#ifdef _WIN32
    #ifdef UNICONV_DLL
        #define UNICONV_EXPORT __declspec(dllexport)
    #elif defined(UNICONV_DLL_IMPORT)
        #define UNICONV_EXPORT __declspec(dllimport)
    #else
        #define UNICONV_EXPORT
    #endif
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

//----------------------------------------------------------------------------------------------------------------------
// === High-Performance Error Handling System ===
//----------------------------------------------------------------------------------------------------------------------

// 轻量级错误码枚举，仅占1字节
enum class ErrorCode : uint8_t {
    Success = 0,
    InvalidParameter = 1,
    InvalidSourceEncoding = 2,
    InvalidTargetEncoding = 3,
    ConversionFailed = 4,
    IncompleteSequence = 5,
    InvalidSequence = 6,
    OutOfMemory = 7,
    BufferTooSmall = 8,
    FileNotFound = 9,
    FileReadError = 10,
    FileWriteError = 11,
    InternalError = 12,
    EncodingNotFound = 13,
    SystemError = 14
};

// 编译时错误信息映射，零运行时开销
namespace detail {
    constexpr const char* GetErrorMessage(ErrorCode code) noexcept {
        switch (code) {
            case ErrorCode::Success: return "Success";
            case ErrorCode::InvalidParameter: return "Invalid parameter";
            case ErrorCode::InvalidSourceEncoding: return "Invalid source encoding";
            case ErrorCode::InvalidTargetEncoding: return "Invalid target encoding";
            case ErrorCode::ConversionFailed: return "Conversion failed";
            case ErrorCode::IncompleteSequence: return "Incomplete multibyte sequence";
            case ErrorCode::InvalidSequence: return "Invalid multibyte sequence";
            case ErrorCode::OutOfMemory: return "Out of memory";
            case ErrorCode::BufferTooSmall: return "Buffer too small";
            case ErrorCode::FileNotFound: return "File not found";
            case ErrorCode::FileReadError: return "File read error";
            case ErrorCode::FileWriteError: return "File write error";
            case ErrorCode::InternalError: return "Internal error";
            case ErrorCode::EncodingNotFound: return "Encoding not found";
            case ErrorCode::SystemError: return "System error";
            default: return "Unknown error";
        }
    }
}

// 高性能CompactResult模板类
template<typename T>
class [[nodiscard]] CompactResult {
private:
    union {
        T value_;
        ErrorCode error_code_;
    };
    bool has_value_;  // 状态标志
    
public:
    // 成功构造 - 零开销
    explicit CompactResult(T&& value) noexcept(std::is_nothrow_move_constructible_v<T>)
        : value_(std::move(value)), has_value_(true) {}
    
    // 从const引用构造
    explicit CompactResult(const T& value) noexcept(std::is_nothrow_copy_constructible_v<T>)
        : value_(value), has_value_(true) {}
    
    // 错误构造 - 极小开销
    explicit CompactResult(ErrorCode error) noexcept 
        : error_code_(error), has_value_(false) {}
    
    // 移动构造
    CompactResult(CompactResult&& other) noexcept 
        : has_value_(other.has_value_) {
        if (has_value_) {
            new(&value_) T(std::move(other.value_));
        } else {
            error_code_ = other.error_code_;
        }
    }
    
    // 移动赋值
    CompactResult& operator=(CompactResult&& other) noexcept {
        if (this != &other) {
            if (has_value_) {
                value_.~T();
            }
            has_value_ = other.has_value_;
            if (has_value_) {
                new(&value_) T(std::move(other.value_));
            } else {
                error_code_ = other.error_code_;
            }
        }
        return *this;
    }
    
    // 拷贝构造
    CompactResult(const CompactResult& other) noexcept(std::is_nothrow_copy_constructible_v<T>)
        : has_value_(other.has_value_) {
        if (has_value_) {
            new(&value_) T(other.value_);
        } else {
            error_code_ = other.error_code_;
        }
    }
    
    // 拷贝赋值
    CompactResult& operator=(const CompactResult& other) noexcept(std::is_nothrow_copy_constructible_v<T>) {
        if (this != &other) {
            if (has_value_) {
                value_.~T();
            }
            has_value_ = other.has_value_;
            if (has_value_) {
                new(&value_) T(other.value_);
            } else {
                error_code_ = other.error_code_;
            }
        }
        return *this;
    }
    
    ~CompactResult() {
        if (has_value_) {
            value_.~T();
        }
    }
    
    // 热路径优化：内联+分支预测
    [[nodiscard]] bool IsSuccess() const noexcept {
        return has_value_; 
    }
    
    // 显式bool转换
    explicit operator bool() const noexcept {
        return IsSuccess();
    }
    
    // 快速访问，无额外检查（性能优先）
    T&& GetValue() && noexcept { 
        return std::move(value_); 
    }
    
    const T& GetValue() const& noexcept { 
        return value_; 
    }
    
    T& GetValue() & noexcept {
        return value_;
    }
    
    // 错误信息获取（编译时字符串，零分配）
    constexpr const char* GetErrorMessage() const noexcept {
        return has_value_ ? "Success" : detail::GetErrorMessage(error_code_);
    }
    
    ErrorCode GetErrorCode() const noexcept {
        return has_value_ ? ErrorCode::Success : error_code_;
    }
    
    // 提供默认值的安全访问
    template<typename U>
    T ValueOr(U&& default_value) const& {
        return has_value_ ? value_ : static_cast<T>(std::forward<U>(default_value));
    }
    
    template<typename U>
    T ValueOr(U&& default_value) && {
        return has_value_ ? std::move(value_) : static_cast<T>(std::forward<U>(default_value));
    }
    
    // 静态工厂方法
    static CompactResult Success(T&& value) {
        return CompactResult(std::move(value));
    }
    
    static CompactResult Success(const T& value) {
        return CompactResult(value);
    }
    
    static CompactResult Failure(ErrorCode error) {
        return CompactResult(error);
    }
};

// std::string特化版本，优化内存使用
template<>
class [[nodiscard]] CompactResult<std::string> {
private:
    std::string value_;     // 使用空字符串表示可能的错误状态
    ErrorCode error_code_;  // 仅1字节
    
public:
    // 成功构造
    explicit CompactResult(std::string&& value) noexcept
        : value_(std::move(value)), error_code_(ErrorCode::Success) {}
        
    explicit CompactResult(const std::string& value) 
        : value_(value), error_code_(ErrorCode::Success) {}
        
    // 错误构造 - 使用空字符串，避免额外分配
    explicit CompactResult(ErrorCode error) noexcept 
        : error_code_(error) {}  // value_ 默认构造为空字符串
    
    // 默认移动和拷贝
    CompactResult(CompactResult&&) noexcept = default;
    CompactResult& operator=(CompactResult&&) noexcept = default;
    CompactResult(const CompactResult&) = default;
    CompactResult& operator=(const CompactResult&) = default;
    
    // 超快速成功检查
    [[nodiscard]] bool IsSuccess() const noexcept { 
        return error_code_ == ErrorCode::Success; 
    }
    
    explicit operator bool() const noexcept {
        return IsSuccess();
    }
    
    // 直接返回引用，零拷贝
    std::string&& GetValue() && noexcept { 
        return std::move(value_); 
    }
    
    const std::string& GetValue() const& noexcept { 
        return value_; 
    }
    
    std::string& GetValue() & noexcept {
        return value_;
    }
    
    ErrorCode GetErrorCode() const noexcept { 
        return error_code_; 
    }
    
    constexpr const char* GetErrorMessage() const noexcept {
        return error_code_ == ErrorCode::Success ? "Success" : detail::GetErrorMessage(error_code_);
    }
    
    // 提供默认值
    template<typename U>
    std::string ValueOr(U&& default_value) const& {
        return IsSuccess() ? value_ : std::string(std::forward<U>(default_value));
    }
    
    template<typename U>
    std::string ValueOr(U&& default_value) && {
        return IsSuccess() ? std::move(value_) : std::string(std::forward<U>(default_value));
    }
    
    // 静态工厂方法
    static CompactResult Success(std::string&& value) {
        return CompactResult(std::move(value));
    }
    
    static CompactResult Success(const std::string& value) {
        return CompactResult(value);
    }
    
    static CompactResult Failure(ErrorCode error) {
        return CompactResult(error);
    }
};

// 便利的类型别名
using StringResult = CompactResult<std::string>;
using StringViewResult = CompactResult<std::string_view>;
using IntResult = CompactResult<int>;
using BoolResult = CompactResult<bool>;

//----------------------------------------------------------------------------------------------------------------------

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

// 前向声明
enum class ErrorCode : uint8_t;
template<typename T> class CompactResult;

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
			// std::cerr << "Closing iconv_t: " << cd << std::endl;
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

   void SetDefaultEncoding(const std::string& encoding) noexcept;

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

		IConvResult() noexcept 
			: conv_result_str{}, error_code{0}, error_msg{} 
		{}
		
		// 移动构造函数和赋值操作符
		IConvResult(IConvResult&&) noexcept = default;
		IConvResult& operator=(IConvResult&&) noexcept = default;
		
		// 复制构造函数和赋值操作符
		IConvResult(const IConvResult&) = default;
		IConvResult& operator=(const IConvResult&) = default;
		/**
		 * @brief Check if the conversion was successful.
		 * @return True if the conversion was successful, false otherwise.
		 * @return bool
		 */
		bool IsSuccess() const noexcept {
			return error_code == 0;
		}

		explicit operator bool() const noexcept {
			return IsSuccess();
		}
		bool operator!() const noexcept {
			return !IsSuccess();
		}
		bool operator==(int code) const noexcept {
			return error_code == code;
		}

		bool operator!=(int code) const noexcept {
			return error_code != code;
		}

		const char* c_str() const noexcept {
			return IsSuccess() ? conv_result_str.c_str() : error_msg.data();
		}
	};


	~UniConv() {
		// 清理资源
		CleanupIconvCache();
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
	static std::string     GetCurrentSystemEncoding() noexcept;


	/**
	 * @brief Get current system encoding code page.
	 * @return The code page of the current system encoding.
	 * @retval 0 if the code page cannot be determined.
	 * @retval -1  it not on windows or Linux
	 * @note finished test on windows
	 */
	static std::uint16_t   GetCurrentSystemEncodingCodePage() noexcept;

	/**
	 * @brief Get the encoding name by code page.
	 * @param  codePage The code page to look up.
	 * @return The name of the encoding corresponding to the given code page.
	 * @retval std::string
	 * @note finished test on windows
	 */
	static std::string     GetEncodingNameByCodePage(std::uint16_t codePage) noexcept;

/** Test Success */ 
/***************************************************************************/
/*=================== Locale <-> UTF-8 Conversion Interface =========================*/
/***************************************************************************/
	/**
	 * @brief Convert a string from system locale encoding to UTF-8 string
	 * @param  input System locale encoding input string
	 * @return Converted UTF-8 string
	 * @todo test
	 */
	std::string ToUtf8FromLocale(const std::string& input);

	/**
	 * @brief Convert a C string from system locale encoding to UTF-8 string
	 * @param  input System locale encoding C string
	 * @return Converted UTF-8 string
	 * @todo test
	 */
	std::string ToUtf8FromLocale(const char* input);

	/**
	 * @brief Convert UTF-8 string to system locale encoding string
	 * @param  input UTF-8 encoded input string
	 * @return Converted system locale encoding string
	 */
	std::string ToLocaleFromUtf8(const std::string& input);

	/**
	 * @brief Convert UTF-8 C string to system locale encoding string
	 * @param  input UTF-8 encoded C string
	 * @return Converted system locale encoding string
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
	 * @brief Convert a locale string to a wide string.
	 * @param input The input locale string to be converted.
	 * @return The converted wide string.
	 */
	std::wstring         ToWideStringFromLocale(const std::string& input);

	/**
	 * @brief Convert a C-style locale string to a wide string.
	 * @param input The C-style locale string to be converted.
	 * @return The converted wide string.
	 */
	std::wstring         ToWideStringFromLocale(const char* input);

	/**
	 * @brief Convert a wide string to a string in the current locale encoding.
	 * @param input The wide string to be converted.
	 * @return The converted string in the current locale encoding.
	 */
	std::string          ToLocaleFromWideString(const std::wstring& input);

	/**
	 * @brief Convert a C-style wide string to a string in the current locale encoding.
	 * @param input The C-style wide string to be converted.
	 * @return The converted string in the current locale encoding.
	 */
	std::string          ToLocaleFromWideString(const wchar_t* input);



/** Test Suceess */
/***************************************************************************/
/*===================== UTF16 <-> Local Encoding =======================*/
/***************************************************************************/
	/**
	 * @brief Convert UTF-16LE string to local encoding.
	 * @param input UTF-16LE string to convert.
	 * @return Converted string in local encoding.
	 */
	std::string ToLocaleFromUtf16LE(const std::u16string& input);

	/**
	 * @brief Convert UTF-16LE C-style string to local encoding.
	 * @param input UTF-16LE C-style string to convert.
	 * @return Converted string in local encoding.
	 */
	std::string ToLocaleFromUtf16LE(const char16_t* input);

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
	 * @brief Convert a UTF-32LE string to UTF-16BE.
	 * @param input The UTF-32LE string to be converted.
	 * @return The converted UTF-16BE string.
	 */
	std::u16string       ToUtf16BEFromUtf32LE(const std::u32string& input);
	std::u16string       ToUtf16BEFromUtf32LE(const char32_t* input);



	/**
	 * @brief Convert UTF-8 string to UTF-32LE.
	 * @param input The UTF-8 string to be converted.
	 * @return The converted UTF-32LE string.
	 */
	std::u32string       ToUtf32LEFromUtf8(const std::string& input);
	std::u32string       ToUtf32LEFromUtf8(const char* input);



	/**
	 * @brief Convert a UTF-16LE string to UTF-32LE.
	 * @param input The UTF-16LE string to be converted.
	 * @return The converted UTF-32LE string.
	 */
	std::u32string       ToUtf32LEFromUtf16LE(const std::u16string& input);
	std::u32string       ToUtf32LEFromUtf16LE(const char16_t* input);



	/**
	 * @brief Convert a UTF-16BE string to UTF-32LE.
	 * @param input The UTF-16BE string to be converted.
	 * @return The converted UTF-32LE string.
	 */
	std::u32string       ToUtf32LEFromUtf16BE(const std::u16string& input);
	std::u32string       ToUtf32LEFromUtf16BE(const char16_t* input);



    /**
     * @brief Convert a UCS-4 (std::wstring, platform dependent) string to UTF-8 encoded std::string.
     * @details This function converts a wide string (std::wstring), which is typically UCS-4 on Linux (wchar_t = 4 bytes)
     *          and UTF-16 on Windows (wchar_t = 2 bytes), to a UTF-8 encoded std::string.
     *          The conversion assumes the input is UCS-4 (i.e., each wchar_t is a Unicode code point).
     * @param input The input wide string (UCS-4 encoded).
     * @return The converted UTF-8 encoded string.
     */
	std::string          ToUtf8FromUcs4(const std::wstring& input);

	/**
	 * @brief Convert UTF-8 string to UCS-4 (std::wstring, platform dependent).
	 * @param input The input UTF-8 encoded string.
	 * @return The converted wide string (UCS-4 encoded).
	 */
	std::wstring         ToUcs4FromUtf8(const std::string& input);


	std::wstring         U16StringToWString(const std::u16string& u16str);
	std::wstring		 U16StringToWString(const char16_t* u16str);

	/**
	 * @brief Encoding to string
	 *
	 */
	static std::string ToString(UniConv::Encoding enc) noexcept;

	/**
	 * @brief Convert between any two encodings using iconv
	 * @param input Input string data
	 * @param fromEncoding Source encoding name
	 * @param toEncoding Target encoding name
	 * @return Conversion result
	 */
	IConvResult             ConvertEncoding(const std::string& input, const char* fromEncoding, const char* toEncoding);

	//----------------------------------------------------------------------------------------------------------------------
	// === High-Performance Methods using CompactResult ===
	//----------------------------------------------------------------------------------------------------------------------
	
	/**
	 * @brief High-performance encoding conversion using CompactResult
	 * @param input Input string data
	 * @param fromEncoding Source encoding name
	 * @param toEncoding Target encoding name  
	 * @return CompactResult containing converted string or error
	 */
	StringResult ConvertEncodingFast(const std::string& input, 
	                                const char* fromEncoding, 
	                                const char* toEncoding) noexcept;
	
	/**
	 * @brief Fast encoding name lookup by codepage (zero-allocation)
	 * @param codepage The codepage to lookup
	 * @return Pointer to encoding name, or nullptr if not found
	 */
	const char* GetEncodingNamePtr(int codepage) noexcept;
	
	/**
	 * @brief Fast encoding name lookup using CompactResult
	 * @param codepage The codepage to lookup
	 * @return CompactResult containing encoding name or error
	 */
	StringViewResult GetEncodingNameFast(int codepage) noexcept;
	
	/**
	 * @brief Fast system codepage retrieval
	 * @return CompactResult containing system codepage or error
	 */
	IntResult GetSystemCodePageFast() noexcept;

private:

	//----------------------------------------------------------------------------------------------------------------------
	// Private members @{
	//----------------------------------------------------------------------------------------------------------------------
	static const std::unordered_map<std::uint16_t,EncodingInfo>  m_encodingMap;                /*!< Encoding map           */
	static const std::unordered_map<std::string,std::uint16_t>   m_encodingToCodePageMap;      /*!< Iconv code page map    */
	mutable std::shared_mutex                                    m_iconvCacheMutex;            /*!< Iconv cache mutex      */
	static const std::unordered_map<int,std::string_view>        m_iconvErrorMap;              /*!< Iconv error messages   */
	mutable std::unordered_map<std::string, IconvSharedPtr>      m_iconvDescriptorCacheMap;    /*!< Iconv descriptor cache */
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
	void                                      CleanupIconvCache();

	std::pair<BomEncoding, std::string_view>  DetectAndRemoveBom(const std::string_view& data);
	std::pair<BomEncoding, std::wstring_view> DetectAndRemoveBom(const std::wstring_view& data);

private:
	UniConv() { }

	UniConv(const UniConv&) = delete;
	UniConv& operator=(const UniConv&) = delete;
};



#endif // UNICONV_H__

