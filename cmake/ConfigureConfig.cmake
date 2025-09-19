# Configure config.h for cross-platform compatibility
include(CheckIncludeFile)
include(CheckFunctionExists)
include(CheckSymbolExists)
include(CheckTypeSize)
include(CheckCSourceCompiles)

# Package information
set(PACKAGE_NAME "UniConv")
set(PACKAGE_VERSION "${PROJECT_VERSION}")
set(PACKAGE_STRING "${PACKAGE_NAME} ${PACKAGE_VERSION}")
set(PACKAGE_BUGREPORT "")
set(PACKAGE_URL "")
set(PACKAGE_TARNAME "uniconv")
set(PACKAGE "${PACKAGE_TARNAME}")
set(VERSION "${PROJECT_VERSION}")

# Platform detection and Windows-specific settings
if(WIN32 OR CYGWIN OR MINGW)
    set(WINDOWS_NATIVE 1)
    message(STATUS "Detected Windows platform, enabling Windows-specific configuration")
    
    # Windows standard headers are typically available
    set(HAVE_STDIO_H 1)
    set(HAVE_STDLIB_H 1)
    set(HAVE_STRING_H 1)
    set(HAVE_WCHAR_H 1)
    set(HAVE_LIMITS_H 1)
    set(HAVE_SYS_TYPES_H 1)
    set(STDC_HEADERS 1)
    
    # Modern Windows (Vista+) and modern compilers support stdint.h
    if(MSVC AND MSVC_VERSION GREATER_EQUAL 1600)  # Visual Studio 2010+
        set(HAVE_STDINT_H 1)
        set(HAVE_INTTYPES_H 1)
    elseif(MINGW OR CYGWIN)
        set(HAVE_STDINT_H 1)
        set(HAVE_INTTYPES_H 1)
    endif()
    
    # Windows doesn't have unistd.h
    set(HAVE_UNISTD_H 0)
    
    # Windows doesn't typically have langinfo
    set(HAVE_LANGINFO_CODESET 0)
    
    # Windows memory allocation behavior
    set(HAVE_MALLOC_POSIX 0)
    set(HAVE_FREE_POSIX 0)
    
    # ICONV_CONST setting for Windows
    set(ICONV_CONST "")
    
    # Windows type sizes (common values)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)  # 64-bit
        set(BITSIZEOF_SIZE_T 64)
        set(BITSIZEOF_PTRDIFF_T 64)
    else()  # 32-bit
        set(BITSIZEOF_SIZE_T 32)
        set(BITSIZEOF_PTRDIFF_T 32)
    endif()
    set(BITSIZEOF_WCHAR_T 16)
    set(BITSIZEOF_WINT_T 16)
    set(BITSIZEOF_SIG_ATOMIC_T 32)
    
    # Standard functions available on Windows
    set(HAVE_MEMMOVE 1)
    set(HAVE_SETLOCALE 1)
    
    # Wide character support on Windows
    set(HAVE_WCHAR_T 1)
    set(HAVE_MBSTATE_T 1)
    set(HAVE_MBRTOWC 1)
    set(HAVE_WCRTOMB 1)
    set(HAVE_MBSINIT 1)
    
    # Thread-safe functions (Windows doesn't have unlocked variants)
    set(HAVE_DECL_CLEARERR_UNLOCKED 0)
    set(HAVE_DECL_FEOF_UNLOCKED 0)
    set(HAVE_DECL_FERROR_UNLOCKED 0)
    set(HAVE_DECL_FFLUSH_UNLOCKED 0)
    set(HAVE_DECL_FGETS_UNLOCKED 0)
    set(HAVE_DECL_FPUTC_UNLOCKED 0)
    set(HAVE_DECL_FPUTS_UNLOCKED 0)
    set(HAVE_DECL_FREAD_UNLOCKED 0)
    set(HAVE_DECL_FWRITE_UNLOCKED 0)
    set(HAVE_DECL_GETCHAR_UNLOCKED 0)
    set(HAVE_DECL_GETC_UNLOCKED 0)
    set(HAVE_DECL_PUTCHAR_UNLOCKED 0)
    set(HAVE_DECL_PUTC_UNLOCKED 0)
    set(HAVE_GETC_UNLOCKED 0)
    
else()
    # Unix/Linux/macOS platform detection and configuration
    set(WINDOWS_NATIVE 0)
    message(STATUS "Detected Unix-like platform, performing feature detection")
    
    # Check for standard headers
    check_include_file("stdio.h" HAVE_STDIO_H)
    check_include_file("stdlib.h" HAVE_STDLIB_H)
    check_include_file("string.h" HAVE_STRING_H)
    check_include_file("strings.h" HAVE_STRINGS_H)
    check_include_file("unistd.h" HAVE_UNISTD_H)
    check_include_file("limits.h" HAVE_LIMITS_H)
    check_include_file("wchar.h" HAVE_WCHAR_H)
    check_include_file("stdint.h" HAVE_STDINT_H)
    check_include_file("inttypes.h" HAVE_INTTYPES_H)
    check_include_file("sys/stat.h" HAVE_SYS_STAT_H)
    check_include_file("sys/types.h" HAVE_SYS_TYPES_H)
    check_include_file("minix/config.h" HAVE_MINIX_CONFIG_H)
    check_include_file("langinfo.h" HAVE_LANGINFO_H)
    
    # Check for langinfo support
    if(HAVE_LANGINFO_H)
        check_symbol_exists("nl_langinfo" "langinfo.h" HAVE_NL_LANGINFO)
        if(HAVE_NL_LANGINFO)
            check_symbol_exists("CODESET" "langinfo.h" HAVE_CODESET)
            if(HAVE_CODESET)
                set(HAVE_LANGINFO_CODESET 1)
            else()
                set(HAVE_LANGINFO_CODESET 0)
            endif()
        else()
            set(HAVE_LANGINFO_CODESET 0)
        endif()
    else()
        set(HAVE_LANGINFO_CODESET 0)
    endif()
    
    # Check for standard library functions
    check_function_exists("memmove" HAVE_MEMMOVE)
    check_function_exists("setlocale" HAVE_SETLOCALE)
    check_function_exists("mbrtowc" HAVE_MBRTOWC)
    check_function_exists("wcrtomb" HAVE_WCRTOMB)
    check_function_exists("mbsinit" HAVE_MBSINIT)
    check_function_exists("getc_unlocked" HAVE_GETC_UNLOCKED)
    
    # Check for unlocked I/O function declarations
    check_symbol_exists("clearerr_unlocked" "stdio.h" HAVE_DECL_CLEARERR_UNLOCKED)
    check_symbol_exists("feof_unlocked" "stdio.h" HAVE_DECL_FEOF_UNLOCKED)
    check_symbol_exists("ferror_unlocked" "stdio.h" HAVE_DECL_FERROR_UNLOCKED)
    check_symbol_exists("fflush_unlocked" "stdio.h" HAVE_DECL_FFLUSH_UNLOCKED)
    check_symbol_exists("fgets_unlocked" "stdio.h" HAVE_DECL_FGETS_UNLOCKED)
    check_symbol_exists("fputc_unlocked" "stdio.h" HAVE_DECL_FPUTC_UNLOCKED)
    check_symbol_exists("fputs_unlocked" "stdio.h" HAVE_DECL_FPUTS_UNLOCKED)
    check_symbol_exists("fread_unlocked" "stdio.h" HAVE_DECL_FREAD_UNLOCKED)
    check_symbol_exists("fwrite_unlocked" "stdio.h" HAVE_DECL_FWRITE_UNLOCKED)
    check_symbol_exists("getchar_unlocked" "stdio.h" HAVE_DECL_GETCHAR_UNLOCKED)
    check_symbol_exists("getc_unlocked" "stdio.h" HAVE_DECL_GETC_UNLOCKED)
    check_symbol_exists("putchar_unlocked" "stdio.h" HAVE_DECL_PUTCHAR_UNLOCKED)
    check_symbol_exists("putc_unlocked" "stdio.h" HAVE_DECL_PUTC_UNLOCKED)
    
    # Check for types and their sizes
    check_type_size("wchar_t" WCHAR_T_SIZE)
    if(WCHAR_T_SIZE)
        set(HAVE_WCHAR_T 1)
        math(EXPR BITSIZEOF_WCHAR_T "${WCHAR_T_SIZE} * 8")
    else()
        set(HAVE_WCHAR_T 0)
        set(BITSIZEOF_WCHAR_T 0)
    endif()
    
    check_type_size("wint_t" WINT_T_SIZE)
    if(WINT_T_SIZE)
        set(HAVE_WINT_T 1)
        math(EXPR BITSIZEOF_WINT_T "${WINT_T_SIZE} * 8")
    else()
        set(HAVE_WINT_T 0)
        set(BITSIZEOF_WINT_T 0)
    endif()
    
    # Check for mbstate_t
    check_c_source_compiles("
        #include <wchar.h>
        int main() { mbstate_t state; return 0; }"
        HAVE_MBSTATE_T)
    
    # Check type sizes
    check_type_size("size_t" SIZE_T_SIZE)
    if(SIZE_T_SIZE)
        math(EXPR BITSIZEOF_SIZE_T "${SIZE_T_SIZE} * 8")
    endif()
    
    check_type_size("ptrdiff_t" PTRDIFF_T_SIZE)
    if(PTRDIFF_T_SIZE)
        math(EXPR BITSIZEOF_PTRDIFF_T "${PTRDIFF_T_SIZE} * 8")
    endif()
    
    check_type_size("sig_atomic_t" SIG_ATOMIC_T_SIZE)
    if(SIG_ATOMIC_T_SIZE)
        math(EXPR BITSIZEOF_SIG_ATOMIC_T "${SIG_ATOMIC_T_SIZE} * 8")
    endif()
    
    # Unix/Linux specific settings
    set(HAVE_MALLOC_POSIX 1)
    set(HAVE_FREE_POSIX 1)
    set(ICONV_CONST "const")
    
    # Standard C headers check
    if(HAVE_STDIO_H AND HAVE_STDLIB_H AND HAVE_STRING_H)
        set(STDC_HEADERS 1)
    else()
        set(STDC_HEADERS 0)
    endif()
endif()

# Common settings for all platforms
set(ENABLE_RELOCATABLE 1)
set(LIBDIR "")
set(INSTALLDIR "")

# Compiler visibility support
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(HAVE_VISIBILITY 1)
else()
    set(HAVE_VISIBILITY 0)
endif()

# Byte order detection
include(TestBigEndian)
test_big_endian(IS_BIG_ENDIAN)
if(IS_BIG_ENDIAN)
    set(WORDS_BIGENDIAN 1)
else()
    set(WORDS_BIGENDIAN 0)
endif()

# Set fallback values for any undefined variables
if(NOT DEFINED HAVE_GETTEXT)
    set(HAVE_GETTEXT 0)
endif()

if(NOT DEFINED HAVE_ICONV)
    set(HAVE_ICONV 1)  # We're building our own iconv
endif()

if(NOT DEFINED C_ALLOCA)
    set(C_ALLOCA 0)
endif()

if(NOT DEFINED DOUBLE_SLASH_IS_DISTINCT_ROOT)
    set(DOUBLE_SLASH_IS_DISTINCT_ROOT 0)
endif()

# Generate config.h from template
configure_file(
    "${CMAKE_SOURCE_DIR}/include/iconv/config.h.in"
    "${CMAKE_SOURCE_DIR}/include/iconv/config.h"
    @ONLY
)

message(STATUS "Generated config.h with the following key settings:")
message(STATUS "  WINDOWS_NATIVE: ${WINDOWS_NATIVE}")
message(STATUS "  HAVE_STDINT_H: ${HAVE_STDINT_H}")
message(STATUS "  HAVE_WCHAR_H: ${HAVE_WCHAR_H}")
message(STATUS "  HAVE_LANGINFO_CODESET: ${HAVE_LANGINFO_CODESET}")
message(STATUS "  ICONV_CONST: ${ICONV_CONST}")
message(STATUS "  WORDS_BIGENDIAN: ${WORDS_BIGENDIAN}")