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
// Block character encoding warnings to ensure cross-platform encoding consistency
#pragma warning(push)
#pragma warning(disable: 4819)  // Character encoding warning
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
#include <cstdlib>
#include <type_traits>
#include <array>
#include <atomic>
#include <future>
#include <thread>
#include <queue>
#include <condition_variable>
#include <chrono>

#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#include <windows.h>
#elif __linux__
#include <unistd.h>
#include <langinfo.h>
#endif


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

//----------------------------------------------------------------------------------------------------------------------
// === Compile-Time Optimizations and Branch Prediction ===
//----------------------------------------------------------------------------------------------------------------------

// Branch prediction hints for better performance
#if defined(__GNUC__) || defined(__clang__)
    #define UNICONV_LIKELY(x)    __builtin_expect(!!(x), 1)
    #define UNICONV_UNLIKELY(x)  __builtin_expect(!!(x), 0)
	// Function attributes for compiler optimization hints:
    #define UNICONV_PURE         __attribute__((pure))  					// No side effects except memory reads, result depends only on inputs.
    #define UNICONV_CONST        __attribute__((const)) 					// Stronger than pure: result depends only on arguments (no memory reads).
    #define UNICONV_NOINLINE     __attribute__((noinline)) 					// Prevent compiler from inlining this function.
    #define UNICONV_ALWAYS_INLINE __attribute__((always_inline)) inline  	// Force inlining (even with -O0).
    #define UNICONV_HOT          __attribute__((hot)) 						// Function is executed often, optimize aggressively.
    #define UNICONV_COLD         __attribute__((cold))						// Function is rarely executed, optimize for size.
    #define UNICONV_FLATTEN      __attribute__((flatten))					// Inline all calls inside, create a single flat function body.
	// Hint CPU to prefetch memory into cache before it's actually needed.
	// addr      : memory address to prefetch
	// rw        : 0 = read (default), 1 = write
	// locality  : temporal locality (0-3), higher means data will be reused soon
	//   0 = no temporal locality (one-time use, evict quickly)
	//   3 = high temporal locality (keep in cache)
	// Example: UNICONV_PREFETCH(ptr, 0, 3);
	#define UNICONV_PREFETCH(addr, rw, locality) __builtin_prefetch((addr), (rw), (locality))

#elif defined(_MSC_VER)
    #define UNICONV_LIKELY(x)    (x)
    #define UNICONV_UNLIKELY(x)  (x)
    #define UNICONV_PURE
    #define UNICONV_CONST
    #define UNICONV_NOINLINE     __declspec(noinline)
    #define UNICONV_ALWAYS_INLINE __forceinline
    #define UNICONV_HOT
    #define UNICONV_COLD
    #define UNICONV_FLATTEN
    #define UNICONV_PREFETCH(addr, rw, locality) _mm_prefetch((const char*)(addr), _MM_HINT_T0)
    // Include intrinsics for prefetch
    #include <immintrin.h>
#else
    #define UNICONV_LIKELY(x)    (x)
    #define UNICONV_UNLIKELY(x)  (x)
    #define UNICONV_PURE
    #define UNICONV_CONST
    #define UNICONV_NOINLINE
    #define UNICONV_ALWAYS_INLINE inline
    #define UNICONV_HOT
    #define UNICONV_COLD
    #define UNICONV_FLATTEN
    #define UNICONV_PREFETCH(addr, rw, locality) ((void)0)
#endif

// Fast math optimizations (use with caution)
#if defined(__GNUC__) || defined(__clang__)
    #define UNICONV_FAST_MATH __attribute__((optimize("fast-math")))
#else
    #define UNICONV_FAST_MATH
#endif

// Template specialization hints
#define UNICONV_TEMPLATE_LIKELY template<bool Condition = true> \
    std::enable_if_t<Condition, void>

// Constexpr evaluation forcing
#if defined(__cpp_consteval) && __cpp_consteval >= 201811L
    #define UNICONV_CONSTEVAL consteval
#else
    #define UNICONV_CONSTEVAL constexpr
#endif

// Loop optimization hints
#if defined(__clang__)
    #define UNICONV_UNROLL_LOOP(n) _Pragma("clang loop unroll_count(" #n ")")
    #define UNICONV_VECTORIZE_LOOP _Pragma("clang loop vectorize(enable)")
#elif defined(__GNUC__)
    #define UNICONV_STRINGIFY(x) #x
    #define UNICONV_PRAGMA(x) _Pragma(UNICONV_STRINGIFY(x))
    #define UNICONV_UNROLL_LOOP(n) UNICONV_PRAGMA(GCC unroll n)
    #define UNICONV_VECTORIZE_LOOP UNICONV_PRAGMA(GCC ivdep)
#else
    #define UNICONV_UNROLL_LOOP(n)
    #define UNICONV_VECTORIZE_LOOP
#endif

// Cache-friendly alignment
#define UNICONV_CACHE_ALIGNED alignas(64)
#define UNICONV_PAGE_ALIGNED alignas(4096)

// Compile-time string hashing for fast encoding lookup
namespace detail {

	/**
	 * @brief FNV-1a hash function for compile-time string hashing.
	 * @details This function used FNV-1a algorithm to compute a hash value for a given string at compile time.
	 * *        If the compiler supports "consteval", this function will be evaluated at compile time.
	 * @param str The input string to hash. Don't need "\0" terminator.
	 * @param len The length of the input string.
	 * @return 32-bit hash value.
	 * @note
	 * * Initial FNV offset basis: 2166136261
	 * * FNV prime: 16777619
	 * * algorithm reference: https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
	 * @see operator""_hash
	 *
	 * @code
	 * constexpr uint32_t hash1 = detail::fnv1a_hash("hello", 5); // Compute hash at compile time
	 * constexpr uint32_t hash2 = detail::fnv1a_hash(runtime_str.c_str(), runtime_str.size()); // Compute hash at runtime
	 * @endcode
	 */
    UNICONV_CONSTEVAL uint32_t fnv1a_hash(const char* str, size_t len) noexcept {
        uint32_t hash = 2166136261u;  // FNV-1a offset basis
        for (size_t i = 0; i < len; ++i) {
            hash ^= static_cast<uint32_t>(str[i]);  // XOR each byte
            hash *= 16777619u;  // Multiply by FNV prime
        }
        return hash;
    }
	/**
     * @brief User-defined literal operator for computing FNV-1a hash values.
     *
     * Allows string literals to be converted into compile-time constant hash values
     * using the `_hash` suffix.
     *
     * @param str Pointer to the string literal data.
     * @param len Length of the string literal (excluding the null terminator).
     * @return A 32-bit unsigned integer representing the FNV-1a hash.
     *
     * @note
     * - Only works with string literals (evaluated at compile time).
     * - Useful in `switch` statements or constant expressions for mapping strings to IDs.
     *
     * @see fnv1a_hash
     *
     * @code
     * using namespace detail;
     *
     * Compile-time string hashing
     * constexpr uint32_t id = "hello"_hash;
     *
     *  Example: string-based switch using hashed cases
     * switch (command_id) {
     *     case "login"_hash:
     *         handle login
     *         break;
     *     case "logout"_hash:
     *         handle logout
     *         break;
     * }
     * @endcode
     */
    UNICONV_CONSTEVAL uint32_t operator""_hash(const char* str, size_t len) noexcept {
        return fnv1a_hash(str, len);
    }
}

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

// Lightweight error code enumeration, only 1 byte
enum class ErrorCode : uint8_t {
    Success					 	= 0,  /*!< Success */
    InvalidParameter 			= 1,  /*!< Invalid parameter */
    InvalidSourceEncoding 		= 2,  /*!< Invalid source encoding */
    InvalidTargetEncoding 		= 3,  /*!< Invalid target encoding */
    ConversionFailed 			= 4,  /*!< Conversion failed */
    IncompleteSequence 			= 5,  /*!< Incomplete multibyte sequence */
    InvalidSequence 			= 6,  /*!< Invalid multibyte sequence */
    OutOfMemory 				= 7,  /*!< Out of memory */
    BufferTooSmall 				= 8,  /*!< Buffer too small */
    FileNotFound 				= 9,  /*!< File not found */
    FileReadError				= 10, /*!< File read error */
    FileWriteError				= 11, /*!< File write error */
    InternalError				= 12, /*!< Internal error */
    EncodingNotFound			= 13, /*!< Encoding not found */
    SystemError					= 14  /*!< System error */
};

// Compile-time error message mapping, zero runtime overhead
namespace detail {
    constexpr const char* GetErrorMessage(ErrorCode code) noexcept {
        switch (code) {
            case ErrorCode::Success: 					return "Success";
            case ErrorCode::InvalidParameter: 			return "Invalid parameter";
            case ErrorCode::InvalidSourceEncoding: 		return "Invalid source encoding";
            case ErrorCode::InvalidTargetEncoding: 		return "Invalid target encoding";
            case ErrorCode::ConversionFailed: 			return "Conversion failed";
            case ErrorCode::IncompleteSequence: 		return "Incomplete multibyte sequence";
            case ErrorCode::InvalidSequence: 			return "Invalid multibyte sequence";
            case ErrorCode::OutOfMemory: 				return "Out of memory";
            case ErrorCode::BufferTooSmall: 			return "Buffer too small";
            case ErrorCode::FileNotFound: 				return "File not found";
            case ErrorCode::FileReadError: 				return "File read error";
            case ErrorCode::FileWriteError: 		    return "File write error";
            case ErrorCode::InternalError: 				return "Internal error";
            case ErrorCode::EncodingNotFound: 			return "Encoding not found";
            case ErrorCode::SystemError: 				return "System error";
            default: 									return "Unknown error";
        }
    }
}

// 高性能CompactResult模板类
template<typename T>
class [[nodiscard]] CompactResult {
private:
    union {
        T 				 value_;
        ErrorCode 	error_code_;
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
    [[nodiscard]] UNICONV_ALWAYS_INLINE UNICONV_HOT
    bool IsSuccess() const noexcept {
        return UNICONV_LIKELY(has_value_);
    }
    
    // 显式bool转换 - 热路径优化
    UNICONV_ALWAYS_INLINE UNICONV_HOT
    explicit operator bool() const noexcept {
        return IsSuccess();
    }

    // 快速访问，无额外检查（性能优先）- 热路径优化
    UNICONV_ALWAYS_INLINE UNICONV_HOT
    T&& GetValue() && noexcept {
        return std::move(value_);
    }

    UNICONV_ALWAYS_INLINE UNICONV_HOT
    const T& GetValue() const& noexcept {
        return value_; 
    }

    UNICONV_ALWAYS_INLINE UNICONV_HOT
    T& GetValue() & noexcept {
        return value_;
    }

    // 错误信息获取（编译时字符串，零分配）- 冷路径
    UNICONV_COLD constexpr const char* GetErrorMessage() const noexcept {
        return UNICONV_LIKELY(has_value_) ? "Success" : detail::GetErrorMessage(error_code_);
    }
    
    UNICONV_ALWAYS_INLINE ErrorCode GetErrorCode() const noexcept {
        return UNICONV_LIKELY(has_value_) ? ErrorCode::Success : error_code_;
    }
    
    // 提供默认值的安全访问 - 成功路径优化
    template<typename U>
    UNICONV_ALWAYS_INLINE T ValueOr(U&& default_value) const& {
        return UNICONV_LIKELY(has_value_) ? value_ : static_cast<T>(std::forward<U>(default_value));
    }
    
    template<typename U>
    UNICONV_ALWAYS_INLINE T ValueOr(U&& default_value) && {
        return UNICONV_LIKELY(has_value_) ? std::move(value_) : static_cast<T>(std::forward<U>(default_value));
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

// 高性能字符串缓冲池类
class StringBufferPool {
private:
    struct Buffer {
        std::string data;
        std::atomic<bool> in_use{false};

        Buffer() {
            data.reserve(4096);  // 预分配4KB，减少动态扩容
        }
    };

    static constexpr size_t POOL_SIZE = 16;  // 池大小，平衡内存使用和并发性能
    std::array<Buffer, POOL_SIZE> buffers_;  // 固定大小缓冲区数组
    std::atomic<size_t> next_index_{0};      // 下一个可用缓冲区索引

public:
    // RAII缓冲区租用器
    class BufferLease {
        Buffer* buffer_;
        StringBufferPool* pool_;

    public:
        BufferLease(Buffer* buf, StringBufferPool* pool) noexcept
            : buffer_(buf), pool_(pool) {}

        ~BufferLease() noexcept {
            if (buffer_) {
                buffer_->in_use.store(false, std::memory_order_release);
            }
        }

        // 移动构造和赋值
        BufferLease(BufferLease&& other) noexcept
            : buffer_(other.buffer_), pool_(other.pool_) {
            other.buffer_ = nullptr;
        }

        BufferLease& operator=(BufferLease&& other) noexcept {
            if (this != &other) {
                if (buffer_) {
                    buffer_->in_use.store(false, std::memory_order_release);
                }
                buffer_ = other.buffer_;
                pool_ = other.pool_;
                other.buffer_ = nullptr;
            }
            return *this;
        }

        // 禁用拷贝
        BufferLease(const BufferLease&) = delete;
        BufferLease& operator=(const BufferLease&) = delete;

        // 获取缓冲区引用
        [[nodiscard]] std::string& get() noexcept {
            return buffer_->data;
        }

        [[nodiscard]] const std::string& get() const noexcept {
            return buffer_->data;
        }

        // 检查是否有效
        [[nodiscard]] bool valid() const noexcept {
            return buffer_ != nullptr;
        }
    };

    // 获取缓冲区租用器 - 热路径优化，分支预测
    [[nodiscard]] UNICONV_HOT UNICONV_FLATTEN BufferLease acquire() noexcept {
        constexpr int max_attempts = POOL_SIZE * 2;

        // 预取下一个可能的索引位置，提升缓存性能
        size_t current_index = next_index_.load(std::memory_order_relaxed);
        UNICONV_PREFETCH(&buffers_[current_index % POOL_SIZE], 0, 1);
        
        // 尝试获取缓冲区
        for (int attempt = 0; UNICONV_LIKELY(attempt < max_attempts); ++attempt) {
            size_t index = next_index_.fetch_add(1, std::memory_order_relaxed) % POOL_SIZE;
            
            bool expected = false;
            if (UNICONV_LIKELY(buffers_[index].in_use.compare_exchange_weak(
                    expected, true, std::memory_order_acquire))) {
                // 获取成功，清理缓冲区准备使用
                buffers_[index].data.clear();
                return BufferLease(&buffers_[index], this);
            }

            // 预取下一个可能的缓冲区位置
            if (UNICONV_LIKELY(attempt + 1 < max_attempts)) {
                size_t next_index = (index + 1) % POOL_SIZE;
                UNICONV_PREFETCH(&buffers_[next_index], 0, 1);
            }
        }

        // 池满时的回退策略：使用线程本地临时缓冲区（罕见情况）
        static thread_local Buffer temp_buffer;
        temp_buffer.data.clear();
        temp_buffer.in_use.store(true);
        return BufferLease(&temp_buffer, nullptr);  // nullptr表示临时缓冲区
    }

    // 获取池统计信息（调试用）
    [[nodiscard]] size_t GetActiveBuffers() const noexcept {
        size_t count = 0;
        for (const auto& buffer : buffers_) {
            if (buffer.in_use.load(std::memory_order_relaxed)) {
                ++count;
            }
        }
        return count;
    }
};

// std::string特化版本，进一步优化内存使用和性能
template<>
class [[nodiscard]] CompactResult<std::string> {
private:
    std::string                   value_;
    ErrorCode                     error_code_;

    // SSO优化：利用标准库的小字符串优化
    static constexpr size_t SSO_THRESHOLD = 23;  // 大多数标准库的SSO阈值

public:
    // 基础构造函数
    explicit CompactResult(std::string&& value) noexcept
        : value_(std::move(value)), error_code_(ErrorCode::Success) {}

    explicit CompactResult(const std::string& value) 
        : value_(value), error_code_(ErrorCode::Success) {}

    explicit CompactResult(ErrorCode error) noexcept 
        : error_code_(error) {}

    // 预留容量构造（避免后续扩容）
    static CompactResult WithReservedCapacity(size_t capacity) noexcept {
        CompactResult result(ErrorCode::Success);
        try {
            result.value_.reserve(capacity);
        } catch (...) {
            return CompactResult(ErrorCode::OutOfMemory);
        }
        return result;
    }

    // 就地构造，避免临时对象
    template<typename... Args>
    static CompactResult EmplaceSuccess(Args&&... args) noexcept {
        try {
            CompactResult result(ErrorCode::Success);
            // Construct string by concatenating all arguments
            std::ostringstream oss;
            (oss << ... << args);
            result.value_ = oss.str();
            return result;
        } catch (...) {
            return CompactResult(ErrorCode::OutOfMemory);
        }
    }

    // 从C字符串快速构造
    static CompactResult FromCString(const char* str, size_t len) noexcept {
        if (!str) {
            return CompactResult(ErrorCode::InvalidParameter);
        }
        try {
            return CompactResult(std::string(str, len));
        } catch (...) {
            return CompactResult(ErrorCode::OutOfMemory);
        }
    }

    // 默认移动和拷贝（编译器优化友好）
    CompactResult(CompactResult&&) noexcept = default;
    CompactResult& operator=(CompactResult&&) noexcept = default;
    CompactResult(const CompactResult&) = default;
    CompactResult& operator=(const CompactResult&) = default;

    // 超快速成功检查 - 热路径优化，预测成功情况更常见
    [[nodiscard]] UNICONV_ALWAYS_INLINE UNICONV_HOT
    bool IsSuccess() const noexcept {
        return UNICONV_LIKELY(error_code_ == ErrorCode::Success);
    }

    UNICONV_ALWAYS_INLINE UNICONV_HOT
    explicit operator bool() const noexcept {
        return IsSuccess();
    }

    // 零拷贝访问方法 - 热路径优化
    [[nodiscard]] UNICONV_ALWAYS_INLINE UNICONV_HOT
    std::string&& GetValue() && noexcept {
        return std::move(value_);
    }

    [[nodiscard]] UNICONV_ALWAYS_INLINE UNICONV_HOT
    const std::string& GetValue() const& noexcept {
        return value_;
    }

    [[nodiscard]] UNICONV_ALWAYS_INLINE UNICONV_HOT
    std::string& GetValue() & noexcept {
        return value_;
    }
    
    // 高性能追加操作（避免多次分配）- 热路径优化
    UNICONV_ALWAYS_INLINE UNICONV_HOT
    void AppendUnsafe(const char* data, size_t len) noexcept {
        // 注意：调用者必须确保对象处于成功状态
        // 预取数据以提升缓存性能
        UNICONV_PREFETCH(data, 0, 1);
        value_.append(data, len);
    }
    
    UNICONV_ALWAYS_INLINE UNICONV_HOT
    void AppendUnsafe(std::string_view sv) noexcept {
        UNICONV_PREFETCH(sv.data(), 0, 1);
        value_.append(sv);
    }
    
    // 获取可写缓冲区（高级用法）- 预测成功路径
    UNICONV_HOT char* GetWritableBuffer(size_t size) {
        if (UNICONV_UNLIKELY(error_code_ != ErrorCode::Success)) {
            return nullptr;
        }
        try {
            value_.resize(size);
            return value_.data();
        } catch (...) {
            error_code_ = ErrorCode::OutOfMemory;
            return nullptr;
        }
    }
    
    // 预留更多容量 - 预测成功情况
    UNICONV_ALWAYS_INLINE bool Reserve(size_t capacity) noexcept {
        if (UNICONV_UNLIKELY(error_code_ != ErrorCode::Success)) {
            return false;
        }
        try {
            value_.reserve(capacity);
            return true;
        } catch (...) {
            error_code_ = ErrorCode::OutOfMemory;
            return false;
        }
    }

    // 获取当前容量和大小 - 热路径优化
    [[nodiscard]] UNICONV_ALWAYS_INLINE UNICONV_HOT
    size_t GetCapacity() const noexcept {
        return value_.capacity();
    }

    [[nodiscard]] UNICONV_ALWAYS_INLINE UNICONV_HOT
    size_t GetSize() const noexcept {
        return value_.size();
    }
    
    // 检查是否使用了SSO
    [[nodiscard]] bool IsSmallString() const noexcept {
        return value_.size() <= SSO_THRESHOLD;
    }
    
    // 错误处理
    [[nodiscard]] ErrorCode GetErrorCode() const noexcept { 
        return error_code_; 
    }
    
    [[nodiscard]] constexpr const char* GetErrorMessage() const noexcept {
        return error_code_ == ErrorCode::Success ? "Success" : detail::GetErrorMessage(error_code_);
    }
    
    // 提供默认值的安全访问
    template<typename U>
    [[nodiscard]] std::string ValueOr(U&& default_value) const& {
        return IsSuccess() ? value_ : std::string(std::forward<U>(default_value));
    }
    
    template<typename U>
    [[nodiscard]] std::string ValueOr(U&& default_value) && {
        return IsSuccess() ? std::move(value_) : std::string(std::forward<U>(default_value));
    }
    
    // 静态工厂方法
    static CompactResult Success(std::string&& value) noexcept {
        return CompactResult(std::move(value));
    }
    
    static CompactResult Success(const std::string& value) {
        return CompactResult(value);
    }
    
    static CompactResult Failure(ErrorCode error) noexcept {
        return CompactResult(error);
    }
};

// 便利的类型别名
using StringResult 		= CompactResult<std::string>;
using StringViewResult 	= CompactResult<std::string_view>;
using IntResult 		= CompactResult<int>;
using BoolResult 		= CompactResult<bool>;

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

	/**
	 * @brief Type alias for a shared pointer to iconv_t with custom deleter.
	 * * This alias simplifies the usage of iconv_t with automatic resource management.
	 * @see IconvDeleter
	 * @see GetIconvDescriptor
	 */
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

	/**
	 * @brief Set the default encoding for conversions.
	 * @param encoding The encoding to set as default (e.g., "UTF-8", "ISO-8859-1").
	 * @note If not set, the system's default encoding will be used.
	 */
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

	// ===================== Error handling adapters =====================
	/**
	 * @brief Convert StringResult to IConvResult for backward compatibility
	 * @param stringResult The StringResult to convert
	 * @return Equivalent IConvResult
	 */
	static IConvResult StringResultToIConvResult(const CompactResult<std::string>& stringResult);

	/**
	 * @brief Convert IConvResult to StringResult for unified internal processing
	 * @param iconvResult The IConvResult to convert
	 * @return Equivalent StringResult
	 */
	static CompactResult<std::string> IConvResultToStringResult(const IConvResult& iconvResult);

	~UniConv() {
		// clean iconv cache
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

	//----------------------------------------------------------------------------------------------------------------------
	// === Enhanced convenience methods with detailed error handling ===
	//----------------------------------------------------------------------------------------------------------------------

	/**
	 * @brief Convert from system locale encoding to UTF-8 with detailed error handling
	 * @param input System locale encoded input string
	 * @return CompactResult<std::string> with conversion result or error details
	 */
	CompactResult<std::string> ToUtf8FromLocaleEx(const std::string& input);

	/**
	 * @brief Convert from UTF-8 to system locale encoding with detailed error handling
	 * @param input UTF-8 encoded input string
	 * @return CompactResult<std::string> with conversion result or error details
	 */
	CompactResult<std::string> ToLocaleFromUtf8Ex(const std::string& input);

	/**
	 * @brief Convert from system locale to UTF-16LE with detailed error handling
	 * @param input System locale encoded input string
	 * @return CompactResult<std::u16string> with conversion result or error details
	 */
	CompactResult<std::u16string> ToUtf16LEFromLocaleEx(const std::string& input);

	/**
	 * @brief Convert from system locale to UTF-16BE with detailed error handling
	 * @param input System locale encoded input string
	 * @return CompactResult<std::u16string> with conversion result or error details
	 */
	CompactResult<std::u16string> ToUtf16BEFromLocaleEx(const std::string& input);

	/**
	 * @brief Convert from UTF-16LE to UTF-8 with detailed error handling
	 * @param input UTF-16LE encoded input string
	 * @return CompactResult<std::string> with conversion result or error details
	 */
	CompactResult<std::string> ToUtf8FromUtf16LEEx(const std::u16string& input);

	/**
	 * @brief Convert from UTF-16BE to UTF-8 with detailed error handling
	 * @param input UTF-16BE encoded input string
	 * @return CompactResult<std::string> with conversion result or error details
	 */
	CompactResult<std::string> ToUtf8FromUtf16BEEx(const std::u16string& input);

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
	UNICONV_HOT StringResult ConvertEncodingFast(const std::string& input,const char* fromEncoding,const char* toEncoding) noexcept;

	/**
	 * @brief Fast encoding name lookup by codepage (zero-allocation)
	 * @param codepage The codepage to lookup
	 * @return Pointer to encoding name, or nullptr if not found
	 */
	UNICONV_HOT UNICONV_PURE const char* GetEncodingNamePtr(int codepage) noexcept;

	/**
	 * @brief Fast encoding name lookup using CompactResult
	 * @param codepage The codepage to lookup
	 * @return CompactResult containing encoding name or error
	 */
	UNICONV_HOT UNICONV_PURE StringViewResult GetEncodingNameFast(int codepage) noexcept;

	/**
	 * @brief Fast system codepage retrieval
	 * @return CompactResult containing system codepage or error
	 */
	UNICONV_HOT IntResult GetSystemCodePageFast() noexcept;

	//----------------------------------------------------------------------------------------------------------------------
	// === Advanced High-Performance Methods ===
	//----------------------------------------------------------------------------------------------------------------------

	/**
	 * @brief High-performance encoding conversion with estimated size hint
	 * @param input Input string data
	 * @param fromEncoding Source encoding name
	 * @param toEncoding Target encoding name
	 * @param estimatedSize Estimated output size (optional, for allocation optimization)
	 * @return CompactResult containing conversion result
	 */
	UNICONV_HOT StringResult ConvertEncodingFastWithHint(const std::string& input,const char* fromEncoding,const char* toEncoding,size_t estimatedSize = 0) noexcept;

	/**
	 * @brief Batch encoding conversion with detailed error handling
	 * @param inputs Input string list
	 * @param fromEncoding Source encoding name
	 * @param toEncoding Target encoding name
	 * @return Conversion result list
	 */
	UNICONV_FLATTEN std::vector<StringResult> ConvertEncodingBatch(const std::vector<std::string>& inputs,const char* fromEncoding,const char* toEncoding) noexcept;

	//---------------------------------------------------------------------------
	// Pool Statistics @{
	//---------------------------------------------------------------------------
		/**
		 * @brief Get statistics about the internal buffer pool and iconv cache.
		 * @return Pool statistics
		 * @struct PoolStats
		 */
		struct PoolStats {
			size_t	 			      active_buffers;  // StringBufferPool active buffer count
			size_t 				   total_conversions;  // Total conversion count
			size_t 						  cache_hits;  // Buffer pool cache hit count
			double 						  	hit_rate;  // Buffer pool hit rate
			size_t 					iconv_cache_size;  // iconv descriptor cache size
			size_t 					iconv_cache_hits;  // iconv cache hit count
			size_t 				  iconv_cache_misses;  // iconv cache miss count
			size_t             iconv_cache_evictions;  // iconv cache eviction count
			double              iconv_cache_hit_rate;  // iconv cache hit rate
			double               iconv_avg_hit_count;  // iconv average hit count
		};
		PoolStats GetPoolStatistics() const;
	//---------------------------------------------------------------------------
	// @} End of Pool Statistics
	//---------------------------------------------------------------------------
private:

	//----------------------------------------------------------------------------------------------------------------------
	// Private members @{
	//----------------------------------------------------------------------------------------------------------------------
	static const std::unordered_map<std::uint16_t,EncodingInfo>  m_encodingMap;                /*!< Encoding map           */
	static const std::unordered_map<std::string,std::uint16_t>   m_encodingToCodePageMap;      /*!< Iconv code page map    */
	mutable std::shared_mutex                                    m_iconvCacheMutex;            /*!< Iconv cache mutex      */

	// implement LRU cache for iconv descriptors
	struct IconvCacheEntry {
		IconvSharedPtr 					descriptor;
		mutable std::atomic<uint64_t> last_used{0};     // 最后使用时间戳
		mutable std::atomic<uint32_t> hit_count{0};     // 命中次数

		IconvCacheEntry() = default;

		IconvCacheEntry(IconvSharedPtr desc) : descriptor(std::move(desc)) {
			last_used.store(GetCurrentTimestamp(), std::memory_order_relaxed);
		}

		// 复制构造函数
		IconvCacheEntry(const IconvCacheEntry& other) : descriptor(other.descriptor) {
			last_used.store(other.last_used.load(), std::memory_order_relaxed);
			hit_count.store(other.hit_count.load(), std::memory_order_relaxed);
		}

		// 移动构造函数
		IconvCacheEntry(IconvCacheEntry&& other) noexcept : descriptor(std::move(other.descriptor)) {
			last_used.store(other.last_used.load(), std::memory_order_relaxed);
			hit_count.store(other.hit_count.load(), std::memory_order_relaxed);
		}

		// 复制赋值运算符
		IconvCacheEntry& operator=(const IconvCacheEntry& other) {
			if (this != &other) {
				descriptor = other.descriptor;
				last_used.store(other.last_used.load(), std::memory_order_relaxed);
				hit_count.store(other.hit_count.load(), std::memory_order_relaxed);
			}
			return *this;
		}

		// 移动赋值运算符
		IconvCacheEntry& operator=(IconvCacheEntry&& other) noexcept {
			if (this != &other) {
				descriptor = std::move(other.descriptor);
				last_used.store(other.last_used.load(), std::memory_order_relaxed);
				hit_count.store(other.hit_count.load(), std::memory_order_relaxed);
			}
			return *this;
		}

		static uint64_t GetCurrentTimestamp() noexcept {
			return static_cast<uint64_t>(std::chrono::steady_clock::now().time_since_epoch().count());
		}

		void UpdateAccess() const noexcept {
			last_used.store(GetCurrentTimestamp(), std::memory_order_relaxed);
			hit_count.fetch_add(1, std::memory_order_relaxed);
		}
	};

	mutable std::unordered_map<std::string, IconvCacheEntry>     m_iconvDescriptorCacheMap;    /*!< Iconv descriptor cache with LRU */
	static constexpr size_t                                      MAX_CACHE_SIZE = 128;         /*!< Increased cache size for better hit rate */
	mutable std::atomic<uint64_t>                                m_cacheHitCount{0};           /*!< Cache hit statistics */
	mutable std::atomic<uint64_t>                                m_cacheMissCount{0};          /*!< Cache miss statistics */
	mutable std::atomic<uint64_t>                                m_cacheEvictionCount{0};      /*!< Cache eviction statistics */
	static const  std::string                                    m_encodingNames[];            /*!< Encoding map           */
	static std::string                                           m_defaultEncoding;            /*!< Current encoding       */
	// 高性能字符串缓冲池
	mutable StringBufferPool                                     m_stringBufferPool;           /*!< String buffer pool for fast conversions */
	// 性能统计（调试和优化用）
	mutable std::atomic<size_t>                                  m_totalConversions{0};        /*!< Total conversion count */
	mutable std::atomic<size_t>                                  m_poolCacheHits{0};           /*!< Pool cache hit count   */
	//----------------------------------------------------------------------------------------------------------------------
	/// @} ! Private members
	//----------------------------------------------------------------------------------------------------------------------

private:

	//----------------------------------------------------------------------------------------------------------------------
	// === High-Performance Internal Methods ===
	//----------------------------------------------------------------------------------------------------------------------

	/**
	 * @brief 智能估算输出缓冲区大小
	 * @param input_size 输入数据大小
	 * @param from_encoding 源编码
	 * @param to_encoding 目标编码
	 * @return 估算的输出大小
	 */
	static size_t EstimateOutputSize(size_t input_size, const char* from_encoding, const char* to_encoding) noexcept;

	/**
	 * @brief 获取编码的字节倍数系数
	 * @param encoding 编码名称
	 * @return 最大字节倍数
	 */
	static constexpr int GetEncodingMultiplier(const char* encoding) noexcept;

	/**
	 * @brief 内部高性能转换实现
	 * @param input 输入数据
	 * @param fromEncoding 源编码
	 * @param toEncoding 目标编码
	 * @param buffer_lease 缓冲区租用器
	 * @param estimated_size 预估大小
	 * @return 转换结果
	 */
	UNICONV_HOT UNICONV_FLATTEN StringResult ConvertEncodingInternal(const std::string& input,const char* fromEncoding,const char* toEncoding,StringBufferPool::BufferLease& buffer_lease,size_t estimated_size) noexcept;

	/**
	 * @brief Get the iconv descriptor.
	 * @param fromcode The source encoding.
	 * @param tocode The target encoding.
	 * @return The iconv descriptor as an IconvSharedPtr.
	 */
	UNICONV_HOT IconvSharedPtr                GetIconvDescriptor(const char* fromcode, const char* tocode);
	void                                      EvictLRUCacheEntries();
	void                                      CleanupIconvCache();

	std::pair<BomEncoding, std::string_view>  DetectAndRemoveBom(const std::string_view& data);
	std::pair<BomEncoding, std::wstring_view> DetectAndRemoveBom(const std::wstring_view& data);

private:
	UniConv() { }

	UniConv(const UniConv&) = delete;
	UniConv& operator=(const UniConv&) = delete;
};

#if _MSC_VER >= 1600
#pragma warning(pop)  // 恢复警告设置
#endif

#endif // UNICONV_H__

