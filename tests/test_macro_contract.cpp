#include <cstddef>
#include <string>
#include <type_traits>

#include <UniConv/UniConv.h>
#include <UniConv/UniConv.h>

#if !defined(UNICONV_CXX_STANDARD)
#error "UNICONV_CXX_STANDARD must always be defined"
#endif

#if !defined(UNICONV_HAS_CHAR8_T)
#error "UNICONV_HAS_CHAR8_T must always be defined"
#endif

#if !defined(UNICONV_NO_THREAD_LOCAL)
#error "UNICONV_NO_THREAD_LOCAL must always be defined"
#endif

#if defined(CPP_STANDARD)
#error "CPP_STANDARD is not part of the UniConv 4.0 public interface"
#endif

#if defined(UNICONV_CONFIG_LEGACY_AUTO_EXECUTION_CHARSET)
#error "The UniConv 4.0 header must not expose legacy charset configuration"
#endif

#if defined(UNICONV_U8_LITERAL) || defined(UNICONV_UTF8_VIEW)
#error "Rejected UTF-8 convenience macros must not be published"
#endif

static_assert(UNICONV_CXX_STANDARD >= 201703L);
static_assert(UNICONV_HAS_CHAR8_T == 0 || UNICONV_HAS_CHAR8_T == 1);
static_assert(UNICONV_NO_THREAD_LOCAL == 0 || UNICONV_NO_THREAD_LOCAL == 1);

#if defined(_MSC_VER) && defined(_MSVC_LANG)
static_assert(UNICONV_CXX_STANDARD == _MSVC_LANG);
#else
static_assert(UNICONV_CXX_STANDARD == __cplusplus);
#endif

#if defined(__cpp_char8_t) && __cpp_char8_t >= 201811L
static_assert(UNICONV_HAS_CHAR8_T == 1);
#else
static_assert(UNICONV_HAS_CHAR8_T == 0);
#endif

static_assert(UNICONV_DETAIL_COMPILER_CLANG == 0 || UNICONV_DETAIL_COMPILER_CLANG == 1);
static_assert(UNICONV_DETAIL_COMPILER_MSVC_CL == 0 || UNICONV_DETAIL_COMPILER_MSVC_CL == 1);
static_assert(UNICONV_DETAIL_COMPILER_CLANG_CL == 0 || UNICONV_DETAIL_COMPILER_CLANG_CL == 1);
static_assert(UNICONV_DETAIL_COMPILER_GCC == 0 || UNICONV_DETAIL_COMPILER_GCC == 1);
static_assert(UNICONV_DETAIL_TOOLCHAIN_MINGW == 0 || UNICONV_DETAIL_TOOLCHAIN_MINGW == 1);
static_assert(UNICONV_DETAIL_PLATFORM_WINDOWS == 0 || UNICONV_DETAIL_PLATFORM_WINDOWS == 1);
static_assert(UNICONV_DETAIL_PLATFORM_LINUX == 0 || UNICONV_DETAIL_PLATFORM_LINUX == 1);
static_assert(UNICONV_DETAIL_PLATFORM_APPLE == 0 || UNICONV_DETAIL_PLATFORM_APPLE == 1);

#if UNICONV_DETAIL_COMPILER_CLANG_CL
static_assert(UNICONV_DETAIL_COMPILER_CLANG == 1);
static_assert(UNICONV_DETAIL_COMPILER_MSVC_CL == 0);
#endif

#if UNICONV_DETAIL_COMPILER_MSVC_CL
static_assert(UNICONV_DETAIL_COMPILER_CLANG == 0);
static_assert(UNICONV_DETAIL_COMPILER_CLANG_CL == 0);
#endif

#if UNICONV_DETAIL_COMPILER_GCC
static_assert(UNICONV_DETAIL_COMPILER_CLANG == 0);
#endif

#if UNICONV_DETAIL_TOOLCHAIN_MINGW
static_assert(UNICONV_DETAIL_PLATFORM_WINDOWS == 1);
#endif

using U8LiteralArray = std::remove_reference_t<decltype(u8"x")>;
using U8LiteralElement = std::remove_extent_t<U8LiteralArray>;

static_assert(std::is_array_v<U8LiteralArray>);
static_assert(std::extent_v<U8LiteralArray> == 2);

#if UNICONV_HAS_CHAR8_T
static_assert(std::is_same_v<U8LiteralElement, const char8_t>);
#else
static_assert(std::is_same_v<U8LiteralElement, const char>);
#endif

int main() {
    auto literal_pointer = u8"x";
    auto raw_literal_pointer = u8R"(raw text)";

#if UNICONV_HAS_CHAR8_T
    static_assert(std::is_same_v<decltype(literal_pointer), const char8_t*>);
    static_assert(std::is_same_v<decltype(raw_literal_pointer), const char8_t*>);
#else
    static_assert(std::is_same_v<decltype(literal_pointer), const char*>);
    static_assert(std::is_same_v<decltype(raw_literal_pointer), const char*>);
#endif

    (void)literal_pointer;
    (void)raw_literal_pointer;

    UniConv converter;
    std::string output;
    const std::string input = "macro-contract";
    const auto error = converter.ConvertEncodingFast(input, "UTF-8", "UTF-8", output);
    return error == ErrorCode::Success && output == "macro-contract" ? 0 : 1;
}
