/*****************************************************************************
*  UniConv
*  Copyright (C) 2025 hesphoros <hesphoros@gmail.com>
*
*  This file is part of UniConv.
*
*  Permission is hereby granted, free of charge, to any person obtaining a 
*  copy of this software and associated documentation files (the "Software"), 
*  to deal in the Software without restriction, including without limitation 
*  the rights to use, copy, modify, merge, publish, distribute, sublicense, 
*  and/or sell copies of the Software, and to permit persons to whom the 
*  Software is furnished to do so, subject to the following conditions:
*
*  The above copyright notice and this permission notice shall be included 
*  in all copies or substantial portions of the Software.
*
*  @file     UniConv.cpp
*  @brief    UniConv impl
*  @details  None
*
*  @author   hesphoros
*  @email    hesphoros@gmail.com
*  @version  3.1.0
*  @date     2026/01/07
*  @license  MIT License
*---------------------------------------------------------------------------*
*  Remark         : None
*---------------------------------------------------------------------------*
*  Change History :
*  <Date>     | <Version> | <Author>       | <Description>
*  2025/03/10 | 1.0.0.1   | hesphoros      | Create file
*  Change History :
*  <Date>     | <Version> | <Author>       | <Description>
*  2025/9/18  | 2.0.0.1   | hesphoros      | Add ConvertEncoding API
*  Change Histrory :
*  <Date>     | <Version> | <Author>       | <Description>
*  2025/12/29 | 3.0.0.1   | hesphoros      | Refactor error handling system
*  <Date>     | <Version> | <Author>       | <Description>
*  2026/1/4   | 3.0.0.2   | hesphoros      | Add simdutf SIMD acceleration support

*****************************************************************************/

#include "UniConv.h"

//==============================================================================
// 可选：simdutf（SIMD 加速 UTF 转换）
//==============================================================================
#ifdef UNICONV_HAS_SIMDUTF
    #include <simdutf.h>
#endif

namespace {

/**
 * @brief 不区分大小写比较两个编码名称是否相等
 * @param enc1 第一个编码名称
 * @param enc2 第二个编码名称
 * @return 如果编码名称等价则返回 true
 * @note 支持常见别名，如 "UTF-8" == "utf-8" == "UTF8"
 */
inline bool CompareEncodingNamesEqual(const char* enc1, const char* enc2) noexcept {
    if (!enc1 || !enc2) return false;
    if (enc1 == enc2) return true;  // 指针相同
    
    // 快速长度检查（允许有分隔符差异，如 UTF-8 vs UTF8）
    size_t len1 = strlen(enc1);
    size_t len2 = strlen(enc2);
    
    // 如果长度差异超过2（考虑 "-" 或 "_" 分隔符），则不相等
    if (len1 > len2 + 2 || len2 > len1 + 2) return false;
    
    // 规范化比较：忽略大小写和分隔符（'-', '_'）
    size_t i = 0, j = 0;
    while (i < len1 && j < len2) {
        char c1 = enc1[i];
        char c2 = enc2[j];
        
        // 跳过分隔符
        if (c1 == '-' || c1 == '_') { ++i; continue; }
        if (c2 == '-' || c2 == '_') { ++j; continue; }
        
        // 转换为小写比较
        if (c1 >= 'A' && c1 <= 'Z') c1 += 32;
        if (c2 >= 'A' && c2 <= 'Z') c2 += 32;
        
        if (c1 != c2) return false;
        ++i; ++j;
    }
    
    // 跳过剩余的分隔符
    while (i < len1 && (enc1[i] == '-' || enc1[i] == '_')) ++i;
    while (j < len2 && (enc2[j] == '-' || enc2[j] == '_')) ++j;
    
    return i == len1 && j == len2;
}

/**
 * @brief 检查编码是否是 ASCII 兼容的
 * @param encoding 编码名称
 * @return 如果编码的 ASCII 字符（0x00-0x7F）与标准 ASCII 一致则返回 true
 * @note ASCII 兼容编码包括：UTF-8, ISO-8859-*, Windows-125*, ASCII 等
 *       非 ASCII 兼容编码：UTF-16, UTF-32, EBCDIC 等
 */
inline bool IsAsciiCompatibleEncoding(const char* encoding) noexcept {
    if (!encoding) return false;
    
    std::string_view enc{encoding};
    
    // ASCII 兼容编码列表
    // UTF-8 是 ASCII 兼容的
    if (enc.find("UTF-8") != std::string_view::npos ||
        enc.find("utf-8") != std::string_view::npos ||
        enc.find("UTF8") != std::string_view::npos ||
        enc.find("utf8") != std::string_view::npos) {
        return true;
    }
    
    // ASCII 本身
    if (enc.find("ASCII") != std::string_view::npos ||
        enc.find("ascii") != std::string_view::npos ||
        enc.find("US-ASCII") != std::string_view::npos ||
        enc.find("ANSI_X3.4") != std::string_view::npos) {
        return true;
    }
    
    // ISO-8859 系列（Latin 系列）
    if (enc.find("ISO-8859") != std::string_view::npos ||
        enc.find("ISO8859") != std::string_view::npos ||
        enc.find("iso-8859") != std::string_view::npos ||
        enc.find("LATIN") != std::string_view::npos ||
        enc.find("latin") != std::string_view::npos) {
        return true;
    }
    
    // Windows 代码页 125x 系列
    if (enc.find("Windows-125") != std::string_view::npos ||
        enc.find("windows-125") != std::string_view::npos ||
        enc.find("CP125") != std::string_view::npos ||
        enc.find("cp125") != std::string_view::npos) {
        return true;
    }
    
    // 中文 GBK/GB2312/GB18030（ASCII 兼容）
    if (enc.find("GBK") != std::string_view::npos ||
        enc.find("gbk") != std::string_view::npos ||
        enc.find("GB2312") != std::string_view::npos ||
        enc.find("gb2312") != std::string_view::npos ||
        enc.find("GB18030") != std::string_view::npos ||
        enc.find("gb18030") != std::string_view::npos) {
        return true;
    }
    
    // Big5（ASCII 兼容）
    if (enc.find("Big5") != std::string_view::npos ||
        enc.find("BIG5") != std::string_view::npos ||
        enc.find("big5") != std::string_view::npos) {
        return true;
    }
    
    // 日文编码（ASCII 兼容）
    if (enc.find("Shift_JIS") != std::string_view::npos ||
        enc.find("SHIFT_JIS") != std::string_view::npos ||
        enc.find("shift_jis") != std::string_view::npos ||
        enc.find("SJIS") != std::string_view::npos ||
        enc.find("sjis") != std::string_view::npos ||
        enc.find("EUC-JP") != std::string_view::npos ||
        enc.find("euc-jp") != std::string_view::npos ||
        enc.find("EUCJP") != std::string_view::npos) {
        return true;
    }
    
    // 韩文编码（ASCII 兼容）
    if (enc.find("EUC-KR") != std::string_view::npos ||
        enc.find("euc-kr") != std::string_view::npos ||
        enc.find("EUCKR") != std::string_view::npos) {
        return true;
    }
    
    // KOI8 系列（ASCII 兼容）
    if (enc.find("KOI8") != std::string_view::npos ||
        enc.find("koi8") != std::string_view::npos) {
        return true;
    }
    
    // 非 ASCII 兼容编码：UTF-16, UTF-32, UCS-2, UCS-4, EBCDIC
    // 这些需要走 iconv 转换
    return false;
}

/**
 * @brief 快速检查字符串是否全是 ASCII 字符（所有字节 < 0x80）
 * @param input 输入字符串
 * @return 如果所有字节都是 ASCII（0x00-0x7F）则返回 true
 * @note 使用位运算优化，一次检查 8 个字节
 */
inline bool IsAllAscii(const std::string& input) noexcept {
    const size_t len = input.size();
    const unsigned char* data = reinterpret_cast<const unsigned char*>(input.data());
    
    // 空字符串视为 ASCII
    if (len == 0) return true;
    
    size_t i = 0;
    
    // 使用 64 位批量检查（每次检查 8 字节）
    // 如果任何字节的最高位为 1，则 OR 结果的对应位也为 1
    if (len >= 8) {
        const uint64_t* ptr64 = reinterpret_cast<const uint64_t*>(data);
        const size_t count64 = len / 8;
        
        for (size_t j = 0; j < count64; ++j) {
            // 检查是否有任何字节 >= 0x80
            // 0x8080808080808080 是每个字节最高位的掩码
            if (ptr64[j] & 0x8080808080808080ULL) {
                return false;
            }
        }
        i = count64 * 8;
    }
    
    // 处理剩余字节
    for (; i < len; ++i) {
        if (data[i] >= 0x80) {
            return false;
        }
    }
    
    return true;
}

//==============================================================================
// simdutf 快速路径辅助函数（仅在启用 simdutf 时编译）
//==============================================================================
#ifdef UNICONV_HAS_SIMDUTF

/**
 * @brief 检查是否为 UTF-8 到 UTF-16LE 的转换
 */
inline bool IsUtf8ToUtf16LE(const char* from, const char* to) noexcept {
    std::string_view from_sv{from};
    std::string_view to_sv{to};
    
    bool is_from_utf8 = (from_sv.find("UTF-8") != std::string_view::npos ||
                         from_sv.find("utf-8") != std::string_view::npos ||
                         from_sv.find("UTF8") != std::string_view::npos ||
                         from_sv.find("utf8") != std::string_view::npos);
    
    bool is_to_utf16le = (to_sv.find("UTF-16LE") != std::string_view::npos ||
                          to_sv.find("utf-16le") != std::string_view::npos ||
                          to_sv.find("UTF16LE") != std::string_view::npos);
    
    return is_from_utf8 && is_to_utf16le;
}

/**
 * @brief 检查是否为 UTF-16LE 到 UTF-8 的转换
 */
inline bool IsUtf16LEToUtf8(const char* from, const char* to) noexcept {
    std::string_view from_sv{from};
    std::string_view to_sv{to};
    
    bool is_from_utf16le = (from_sv.find("UTF-16LE") != std::string_view::npos ||
                            from_sv.find("utf-16le") != std::string_view::npos ||
                            from_sv.find("UTF16LE") != std::string_view::npos);
    
    bool is_to_utf8 = (to_sv.find("UTF-8") != std::string_view::npos ||
                       to_sv.find("utf-8") != std::string_view::npos ||
                       to_sv.find("UTF8") != std::string_view::npos ||
                       to_sv.find("utf8") != std::string_view::npos);
    
    return is_from_utf16le && is_to_utf8;
}

/**
 * @brief 检查是否为 UTF-8 到 UTF-16BE 的转换
 */
inline bool IsUtf8ToUtf16BE(const char* from, const char* to) noexcept {
    std::string_view from_sv{from};
    std::string_view to_sv{to};
    
    bool is_from_utf8 = (from_sv.find("UTF-8") != std::string_view::npos ||
                         from_sv.find("utf-8") != std::string_view::npos ||
                         from_sv.find("UTF8") != std::string_view::npos ||
                         from_sv.find("utf8") != std::string_view::npos);
    
    bool is_to_utf16be = (to_sv.find("UTF-16BE") != std::string_view::npos ||
                          to_sv.find("utf-16be") != std::string_view::npos ||
                          to_sv.find("UTF16BE") != std::string_view::npos);
    
    return is_from_utf8 && is_to_utf16be;
}

/**
 * @brief 检查是否为 UTF-16BE 到 UTF-8 的转换
 */
inline bool IsUtf16BEToUtf8(const char* from, const char* to) noexcept {
    std::string_view from_sv{from};
    std::string_view to_sv{to};
    
    bool is_from_utf16be = (from_sv.find("UTF-16BE") != std::string_view::npos ||
                            from_sv.find("utf-16be") != std::string_view::npos ||
                            from_sv.find("UTF16BE") != std::string_view::npos);
    
    bool is_to_utf8 = (to_sv.find("UTF-8") != std::string_view::npos ||
                       to_sv.find("utf-8") != std::string_view::npos ||
                       to_sv.find("UTF8") != std::string_view::npos ||
                       to_sv.find("utf8") != std::string_view::npos);
    
    return is_from_utf16be && is_to_utf8;
}

/**
 * @brief 使用 simdutf 进行 UTF-8 到 UTF-16LE 转换
 */
inline StringResult ConvertUtf8ToUtf16LE_SIMD(const std::string& input) noexcept {
    // 先验证 UTF-8 是否有效
    if (!simdutf::validate_utf8(input.data(), input.size())) {
        return StringResult::Failure(ErrorCode::InvalidSequence);
    }
    
    // 计算输出长度
    size_t utf16_len = simdutf::utf16_length_from_utf8(input.data(), input.size());
    
    // 分配输出缓冲区
    std::u16string utf16_output(utf16_len, u'\0');
    
    // 执行转换
    size_t written = simdutf::convert_utf8_to_utf16le(
        input.data(), input.size(),
        utf16_output.data());
    
    if (written == 0 && utf16_len > 0) {
        return StringResult::Failure(ErrorCode::ConversionFailed);
    }
    
    // 转换为字节字符串
    std::string result;
    result.resize(utf16_output.size() * sizeof(char16_t));
    std::memcpy(result.data(), utf16_output.data(), result.size());
    
    return StringResult::Success(std::move(result));
}

/**
 * @brief 使用 simdutf 进行 UTF-16LE 到 UTF-8 转换
 */
inline StringResult ConvertUtf16LEToUtf8_SIMD(const std::string& input) noexcept {
    // 检查输入长度是否为偶数
    if (input.size() % 2 != 0) {
        return StringResult::Failure(ErrorCode::InvalidSequence);
    }
    
    const char16_t* utf16_data = reinterpret_cast<const char16_t*>(input.data());
    size_t utf16_len = input.size() / 2;
    
    // 验证 UTF-16LE
    if (!simdutf::validate_utf16le(utf16_data, utf16_len)) {
        return StringResult::Failure(ErrorCode::InvalidSequence);
    }
    
    // 计算输出长度
    size_t utf8_len = simdutf::utf8_length_from_utf16le(utf16_data, utf16_len);
    
    // 分配输出缓冲区
    std::string result(utf8_len, '\0');
    
    // 执行转换
    size_t written = simdutf::convert_utf16le_to_utf8(
        utf16_data, utf16_len,
        result.data());
    
    if (written == 0 && utf8_len > 0) {
        return StringResult::Failure(ErrorCode::ConversionFailed);
    }
    
    result.resize(written);
    return StringResult::Success(std::move(result));
}

/**
 * @brief 使用 simdutf 进行 UTF-8 到 UTF-16BE 转换
 */
inline StringResult ConvertUtf8ToUtf16BE_SIMD(const std::string& input) noexcept {
    // 先验证 UTF-8 是否有效
    if (!simdutf::validate_utf8(input.data(), input.size())) {
        return StringResult::Failure(ErrorCode::InvalidSequence);
    }
    
    // 计算输出长度
    size_t utf16_len = simdutf::utf16_length_from_utf8(input.data(), input.size());
    
    // 分配输出缓冲区
    std::u16string utf16_output(utf16_len, u'\0');
    
    // 执行转换
    size_t written = simdutf::convert_utf8_to_utf16be(
        input.data(), input.size(),
        utf16_output.data());
    
    if (written == 0 && utf16_len > 0) {
        return StringResult::Failure(ErrorCode::ConversionFailed);
    }
    
    // 转换为字节字符串
    std::string result;
    result.resize(utf16_output.size() * sizeof(char16_t));
    std::memcpy(result.data(), utf16_output.data(), result.size());
    
    return StringResult::Success(std::move(result));
}

/**
 * @brief 使用 simdutf 进行 UTF-16BE 到 UTF-8 转换
 */
inline StringResult ConvertUtf16BEToUtf8_SIMD(const std::string& input) noexcept {
    // 检查输入长度是否为偶数
    if (input.size() % 2 != 0) {
        return StringResult::Failure(ErrorCode::InvalidSequence);
    }
    
    const char16_t* utf16_data = reinterpret_cast<const char16_t*>(input.data());
    size_t utf16_len = input.size() / 2;
    
    // 验证 UTF-16BE
    if (!simdutf::validate_utf16be(utf16_data, utf16_len)) {
        return StringResult::Failure(ErrorCode::InvalidSequence);
    }
    
    // 计算输出长度
    size_t utf8_len = simdutf::utf8_length_from_utf16be(utf16_data, utf16_len);
    
    // 分配输出缓冲区
    std::string result(utf8_len, '\0');
    
    // 执行转换
    size_t written = simdutf::convert_utf16be_to_utf8(
        utf16_data, utf16_len,
        result.data());
    
    if (written == 0 && utf8_len > 0) {
        return StringResult::Failure(ErrorCode::ConversionFailed);
    }
    
    result.resize(written);
    return StringResult::Success(std::move(result));
}

#endif // UNICONV_HAS_SIMDUTF

} // anonymous namespace


#if !UNICONV_NO_THREAD_LOCAL
// Thread-local cache instance definition (only when thread_local is enabled)
thread_local UniConv::ThreadLocalCache UniConv::t_cache;
#endif

const std::string UniConv::m_encodingNames[] = {
    #define X(name, str) str,
    #include "encodings.inc"
    #undef X
};

// Thread-safe default encoding management
namespace {
    std::atomic<const std::string*> g_defaultEncodingPtr{nullptr};
    std::mutex g_defaultEncodingMutex;
}

std::string UniConv::m_defaultEncoding = {}; // Legacy static member (deprecated, kept for compatibility)

/**************************  === UniConv m_encodingMap ===  ***************************/
const std::unordered_map<std::uint16_t, UniConv::EncodingInfo> UniConv::m_encodingMap = {
    {37,     {"IBM037",       "IBM EBCDIC US-Canada"}},
    {437,    {"IBM437",       "OEM United States"}},
    {850,    {"IBM850",       "OEM Multilingual Latin 1; Western European (DOS)"}},
    {852,    {"IBM852",       "OEM Latin 2; Central European (DOS)"}},
    {855,    {"IBM855",       "OEM Cyrillic (primarily Russian)"}},
    {857,    {"IBM857",       "OEM Turkish; Turkish (DOS)"}},
    {860,    {"IBM860",       "OEM Portuguese; Portuguese (DOS)"}},
    {861,    {"IBM861",       "OEM Icelandic; Icelandic (DOS)"}},
    {862,    {"DOS-862",      "OEM Hebrew; Hebrew (DOS)"}},
    {863,    {"IBM863",       "OEM French Canadian; French Canadian (DOS)"}},
    {865,    {"IBM865",       "OEM Nordic; Nordic (DOS)"}},
    {866,    {"CP866",        "OEM Russian; Cyrillic (DOS)"}},
    {874,    {"Windows-874",  "Thai (Windows)"}},
    {932,    {"Shift_JIS",    "ANSI/OEM Japanese; Japanese (Shift-JIS)"}},
    {936,    {"GB2312",       "ANSI/OEM Simplified Chinese (PRC, Singapore); Chinese Simplified (GB2312)"}},
    {949,    {"KS_C_5601-1987", "ANSI/OEM Korean (Unified Hangul Code)"}},
    {950,    {"Big5",         "ANSI/OEM Traditional Chinese (Taiwan; Hong Kong SAR, PRC); Chinese Traditional (Big5)"}},
    {1200,   {"UTF-16",       "Unicode UTF-16, little endian byte order (BMP of ISO 10646); available only to managed applications"}},
    {1201,   {"UTF-16BE",     "Unicode UTF-16, big endian byte order; available only to managed applications"}},
    {1250,   {"Windows-1250", "ANSI Central European; Central European (Windows)"}},
    {1251,   {"Windows-1251", "ANSI Cyrillic; Cyrillic (Windows)"}},
    {1252,   {"Windows-1252", "ANSI Latin 1; Western European (Windows)"}},
    {1253,   {"Windows-1253", "ANSI Greek; Greek (Windows)"}},
    {1254,   {"Windows-1254", "ANSI Turkish; Turkish (Windows)"}},
    {1255,   {"Windows-1255", "ANSI Hebrew; Hebrew (Windows)"}},
    {1256,   {"Windows-1256", "ANSI Arabic; Arabic (Windows)"}},
    {1257,   {"Windows-1257", "ANSI Baltic; Baltic (Windows)"}},
    {1258,   {"Windows-1258", "ANSI/OEM Vietnamese; Vietnamese (Windows)"}},
    {20866,  {"KOI8-R",       "Russian (KOI8-R); Cyrillic (KOI8-R)"}},
    {21866,  {"KOI8-U",       "Ukrainian (KOI8-U); Cyrillic (KOI8-U)"}},
    {28591,  {"ISO-8859-1",   "ISO 8859-1 Latin 1; Western European (ISO)"}},
    {28592,  {"ISO-8859-2",   "ISO 8859-2 Central European; Central European (ISO)"}},
    {28595,  {"ISO-8859-5",   "ISO 8859-5 Cyrillic"}},
    {28597,  {"ISO-8859-7",   "ISO 8859-7 Greek"}},
    {28599,  {"ISO-8859-9",   "ISO 8859-9 Turkish"}},
    {28605,  {"ISO-8859-15",  "ISO 8859-15 Latin 9"}},
    {50220,  {"ISO-2022-JP",  "ISO 2022 Japanese with no halfwidth Katakana; Japanese (JIS)"}},
    {50225,  {"ISO-2022-KR",  "ISO 2022 Korean"}},
    {51932,  {"EUC-JP",       "EUC Japanese"}},
    {51936,  {"EUC-CN",       "EUC Simplified Chinese; Chinese Simplified (EUC)"}},
    {51949,  {"EUC-KR",       "EUC Korean"}},
    {52936,  {"HZ-GB-2312",   "HZ-GB2312 Simplified Chinese; Chinese Simplified (HZ)"}},
    {54936,  {"GB18030",      "Windows XP and later: GB18030 Simplified Chinese (4 byte); Chinese Simplified (GB18030)"}},
    {65000,  {"UTF-7",        "Unicode (UTF-7)"}},
    {65001,  {"UTF-8",        "Unicode (UTF-8)"}}
};

/**************************  === UniConv m_encodingToCodePageMap ===  ***************************/
const std::unordered_map<std::string, std::uint16_t> UniConv::m_encodingToCodePageMap = {
    {"UTF-8",                  65001},    // Unicode (UTF-8)
    {"ANSI_X3.4-1968",         20127},    // US-ASCII
    {"ISO-8859-1",             28591},    // Latin-1
    {"ISO-8859-2",             28592},    // Latin-2
    {"ISO-8859-3",             28593},    // Latin-3
    {"ISO-8859-4",             28594},    // Latin-4 (Baltic)
    {"ISO-8859-5",             28595},    // Cyrillic
    {"ISO-8859-6",             28596},    // Arabic
    {"ISO-8859-7",             28597},    // Greek
    {"ISO-8859-8",             28598},    // Hebrew
    {"ISO-8859-9",             28599},    // Latin-5 (Turkish)
    {"ISO-8859-10",            28600},    // Latin-6 (Nordic)
    {"ISO-8859-11",            28601},    // Thai
    {"ISO-8859-13",            28603},    // Latin-7 (Baltic Rim)
    {"ISO-8859-14",            28604},    // Latin-8 (Celtic)
    {"ISO-8859-15",            28605},    // Latin-9 (Western European with Euro)
    {"ISO-8859-16",            28606},    // Latin-10 (South-Eastern European)
    {"GB2312",                 936},      // Simplified Chinese
    {"GBK",                    936},      // Simplified Chinese (GBK)
    {"GB18030",                54936},    // Simplified Chinese (GB18030)
    {"BIG5",                   950},      // Traditional Chinese
    {"EUC-JP",                 20932},    // Japanese
    {"EUC-KR",                 51949},    // Korean
    {"KOI8-R",                 20866},    // Russian
    {"KOI8-U",                 21866},    // Ukrainian
    {"Windows-1250",           1250},     // Central European (Windows)
    {"Windows-1251",           1251},     // Cyrillic (Windows)
    {"Windows-1252",           1252},     // Western European (Windows)
    {"Windows-1253",           1253},     // Greek (Windows)
    {"Windows-1254",           1254},     // Turkish (Windows)
    {"Windows-1255",           1255},     // Hebrew (Windows)
    {"Windows-1256",           1256},     // Arabic (Windows)
    {"Windows-1257",           1257},     // Baltic (Windows)
    {"Windows-1258",           1258},     // Vietnamese (Windows)
    {"Shift_JIS",              932},      // Japanese (Shift-JIS)
    {"CP932",                  932},      // Japanese (Shift-JIS, Windows)
    {"CP949",                  949},      // Korean (Unified Hangul Code, Windows)
    {"CP950",                  950},      // Traditional Chinese (Big5, Windows)
    {"CP866",                  866},      // Cyrillic (DOS)
    {"CP850",                  850},      // Western European (DOS)
    {"CP852",                  852},      // Central European (DOS)
    {"CP855",                  855},      // Cyrillic (DOS, primarily Russian)
    {"CP857",                  857},      // Turkish (DOS)
    {"CP860",                  860},      // Portuguese (DOS)
    {"CP861",                  861},      // Icelandic (DOS)
    {"CP862",                  862},      // Hebrew (DOS)
    {"CP863",                  863},      // French Canadian (DOS)
    {"CP864",                  864},      // Arabic (DOS)
    {"CP865",                  865},      // Nordic (DOS)
    {"CP869",                  869},      // Modern Greek (DOS)
    {"CP874",                  874},      // Thai (Windows)
    {"CP1250",                 1250},     // Central European (Windows)
    {"CP1251",                 1251},     // Cyrillic (Windows)
    {"CP1252",                 1252},     // Western European (Windows)
    {"CP1253",                 1253},     // Greek (Windows)
    {"CP1254",                 1254},     // Turkish (Windows)
    {"CP1255",                 1255},     // Hebrew (Windows)
    {"CP1256",                 1256},     // Arabic (Windows)
    {"CP1257",                 1257},     // Baltic (Windows)
    {"CP1258",                 1258},     // Vietnamese (Windows)
    {"MacRoman",               10000},    // Western European (Mac)
    {"MacCyrillic",            10007},    // Cyrillic (Mac)
    {"MacGreek",               10006},    // Greek (Mac)
    {"MacTurkish",             10081},    // Turkish (Mac)
    {"MacIcelandic",           10079},    // Icelandic (Mac)
    {"MacCentralEurope",       10029},    // Central European (Mac)
    {"MacThai",                10021},    // Thai (Mac)
    {"MacJapanese",            10001},    // Japanese (Mac)
    {"MacChineseTrad",         10002},    // Traditional Chinese (Mac)
    {"MacChineseSimp",         10008},    // Simplified Chinese (Mac)
    {"MacKorean",              10003},    // Korean (Mac)
    {"MacArabic",              10004},    // Arabic (Mac)
    {"MacHebrew",              10005},    // Hebrew (Mac)
    {"TIS-620",                874},      // Thai (TIS-620)
    {"ISCII-DEVANAGARI",       57002},    // ISCII Devanagari
    {"ISCII-BENGALI",          57003},    // ISCII Bangla
    {"ISCII-TAMIL",            57004},    // ISCII Tamil
    {"ISCII-TELUGU",           57005},    // ISCII Telugu
    {"ISCII-ASSAMESE",         57006},    // ISCII Assamese
    {"ISCII-ORIYA",            57007},    // ISCII Odia
    {"ISCII-KANNADA",          57008},    // ISCII Kannada
    {"ISCII-MALAYALAM",        57009},    // ISCII Malayalam
    {"ISCII-GUJARATI",         57010},    // ISCII Gujarati
    {"ISCII-PUNJABI",          57011},    // ISCII Punjabi
    {"VISCII",                 1258},     // Vietnamese (VISCII)
    {"VPS",                    1258},     // Vietnamese (VPS)
    {"UTF-16",                 1200},     // Unicode UTF-16 (Little Endian)
    {"UTF-16BE",               1201},     // Unicode UTF-16 (Big Endian)
    {"UTF-32",                 12000},    // Unicode UTF-32 (Little Endian)
    {"UTF-32BE",               12001},    // Unicode UTF-32 (Big Endian)
    {"UTF-7",                  65000},    // Unicode UTF-7
    {"HZ-GB-2312",             52936},    // HZ-GB2312 Simplified Chinese
    {"ISO-2022-JP",            50220},    // Japanese (ISO-2022-JP)
    {"ISO-2022-KR",            50225},    // Korean (ISO-2022-KR)
    {"ISO-2022-CN",            50227},    // Simplified Chinese (ISO-2022-CN)
    {"EUC-TW",                 51950},    // Traditional Chinese (EUC-TW)
    {"ARMSCII-8",              0},        // Armenian (ARMSCII-8, no Windows code page)
    {"GEORGIAN-ACADEMY",       0},        // Georgian (Academy, no Windows code page)
    {"GEORGIAN-PS",            0},        // Georgian (PS, no Windows code page)
    {"TSCII",                  0},        // Tamil (TSCII, no Windows code page)
    {"RK1048",                 0},        // Kazakh (RK1048, no Windows code page)
    {"MULELAO-1",              0},        // Lao (MULELAO-1, no Windows code page)
    {"TCVN",                   1258},     // Vietnamese (TCVN)
    {"VISCII1.1",              1258},     // Vietnamese (VISCII 1.1)
    {"VISCII1.1-HYBRID",       1258},     // Vietnamese (VISCII 1.1 Hybrid)
};



void UniConv::SetDefaultEncoding(const std::string& encoding) noexcept
{
    // Thread-safe implementation using atomic pointer and mutex
    std::lock_guard<std::mutex> lock(g_defaultEncodingMutex);
    
    // Load old pointer
    const std::string* old_ptr = g_defaultEncodingPtr.load(std::memory_order_relaxed);
    
    // Create new string
    const std::string* new_ptr = new std::string(encoding);
    
    // Atomically update pointer
    g_defaultEncodingPtr.store(new_ptr, std::memory_order_release);
    
    // Delete old string (safe because we hold the mutex)
    delete old_ptr;
    
    // Update legacy static member for backward compatibility
    this->m_defaultEncoding = encoding;
}


// ===================== System encoding related functions =====================
std::string UniConv::GetCurrentSystemEncoding() noexcept
{
#if !UNICONV_NO_THREAD_LOCAL
    // Check thread-local cache first (fast path)
    if (t_cache.system_encoding_cached) {
        return t_cache.system_encoding;
    }
#else
    // Without thread_local, always compute dynamically
    // This is less efficient but provides DLL compatibility
#endif
    
    // Check default encoding - need a static default since this is a static function
    // For static functions, we cannot access instance members like m_defaultEncoding
    // So we'll compute the system encoding each time when UNICONV_NO_THREAD_LOCAL is defined
    
	std::stringstream ss;
#ifdef _WIN32
	UINT codePage = GetACP();
	auto it = m_encodingMap.find(codePage);
	if (it != m_encodingMap.end())
        ss << it->second.dotNetName;

#endif // _WIN32


#ifdef __linux__
	setlocale(LC_ALL, "");
	char* locstr = setlocale(LC_CTYPE, NULL);
	char* encoding = nl_langinfo(CODESET);
	ss << encoding;
#endif // __linux__
	if (ss.str().empty()) ss << "Encoding not found.";
	
#if !UNICONV_NO_THREAD_LOCAL	
	// Cache the result in thread-local storage
	t_cache.system_encoding = ss.str();
	t_cache.system_encoding_cached = true;
	
	return t_cache.system_encoding;
#else
	// Return directly without caching when thread_local is disabled
	return ss.str();
#endif
}


std::uint16_t UniConv::GetCurrentSystemEncodingCodePage() noexcept {
#ifdef _WIN32
	UINT codePage = GetACP();
	return static_cast<std::uint16_t>(codePage);
#endif // _WIN32


#ifdef __linux__
	setlocale(LC_ALL, "");
	char* locstr = setlocale(LC_CTYPE, NULL);
	char* encoding = nl_langinfo(CODESET);
	auto it = m_encodingToCodePageMap.find(encoding);

	if (it != m_encodingToCodePageMap.end())
        return it->second;
    else
    {
        // Encoding not found in mapping table, return default code page 0
        return 0;
    }
#endif // __linux__
	return -1; // Default return value if not found
}

std::string  UniConv::GetEncodingNameByCodePage(std::uint16_t codePage) noexcept
{
	auto it = m_encodingMap.find(codePage);
	if (it != m_encodingMap.end())
		return it->second.dotNetName;
	else
		return "Encoding not found.";
}


// ===================== Refactored encoding conversion methods =====================

// System local encoding -> UTF-8
std::string UniConv::ToUtf8FromLocale(const std::string& input) {
    if(input.empty())
        return std::string{};
    std::string currentEncoding = GetCurrentSystemEncoding();
    auto result = ConvertEncodingFast(input, currentEncoding.c_str(), ToString(Encoding::utf_8).c_str());
    return result.IsSuccess() ? std::move(result).GetValue() : "";
}

std::string UniConv::ToUtf8FromLocale(const char* input) {
    if (!input) return "";
    return ToUtf8FromLocale(std::string(input));
}

// UTF-8 -> System local encoding
std::string UniConv::ToLocaleFromUtf8(const std::string& input) {
    std::string currentEncoding = GetCurrentSystemEncoding();
    auto result = ConvertEncodingFast(input, ToString(Encoding::utf_8).c_str(), currentEncoding.c_str());
    return result.IsSuccess() ? std::move(result).GetValue() : "";
}

std::string UniConv::ToLocaleFromUtf8(const char* input) {
    if (!input) return "";
    return ToLocaleFromUtf8(std::string(input));
}

// System local encoding -> UTF-16LE
std::u16string UniConv::ToUtf16LEFromLocale(const std::string& input) {
    std::string currentEncoding = GetCurrentSystemEncoding();
    auto result = ConvertEncodingFast(input, currentEncoding.c_str(), UniConv::ToString(UniConv::Encoding::utf_16le).c_str());
    if (result.IsSuccess()) {
        auto value = std::move(result).GetValue();
        if (value.size() % 2 == 0) {
            return std::u16string(reinterpret_cast<const char16_t*>(value.data()),
                                 value.size() / 2);
        }
    }
    return std::u16string();
}

std::u16string UniConv::ToUtf16LEFromLocale(const char* input) {
    if (!input) return std::u16string();
    return ToUtf16LEFromLocale(std::string(input));
}

// System local encoding -> UTF-16BE
std::u16string UniConv::ToUtf16BEFromLocale(const std::string& input) {
    std::string currentEncoding = GetCurrentSystemEncoding();
    auto result = ConvertEncodingFast(input, currentEncoding.c_str(), ToString(Encoding::utf_16be).c_str());
    if (result.IsSuccess()) {
        auto value = std::move(result).GetValue();
        if (value.size() % 2 == 0) {
            return std::u16string(reinterpret_cast<const char16_t*>(value.data()), 
                                 value.size() / 2);
        }
    }
    return std::u16string();
}

std::u16string UniConv::ToUtf16BEFromLocale(const char* input) {
    if (!input) return std::u16string();
    return ToUtf16BEFromLocale(std::string(input));
}

// UTF-16BE -> System local encoding
std::string UniConv::ToLocaleFromUtf16BE(const std::u16string& input) {
    if (input.empty()) return "";

    std::string currentEncoding = GetCurrentSystemEncoding();
    std::string input_bytes(reinterpret_cast<const char*>(input.data()), input.size() * sizeof(char16_t));
    auto result = ConvertEncodingFast(input_bytes, ToString(UniConv::Encoding::utf_16be).c_str(), currentEncoding.c_str());
    return result.IsSuccess() ? std::move(result).GetValue() : "";
}

std::string UniConv::ToLocaleFromUtf16BE(const char16_t* input) {
    if (!input) return "";
    return ToLocaleFromUtf16BE(std::u16string(input));
}

// UTF-16LE -> UTF-8
std::string UniConv::ToUtf8FromUtf16LE(const std::u16string& input) {
    if (input.empty()) return "";

    std::string input_bytes(reinterpret_cast<const char*>(input.data()), input.size() * sizeof(char16_t));
    auto result = ConvertEncodingFast(input_bytes, ToString(Encoding::utf_16le).c_str(), ToString(Encoding::utf_8).c_str());
    return result.IsSuccess() ? std::move(result).GetValue() : "";
}

// ===================== UTF-16LE with length parameter overloads =====================
std::string UniConv::ToUtf8FromUtf16LE(const char16_t* input, size_t len) {
    if (!input || len == 0) return "";

    std::string input_bytes(reinterpret_cast<const char*>(input), len * sizeof(char16_t));
    auto result = ConvertEncodingFast(input_bytes, ToString(Encoding::utf_16le).c_str(), ToString(Encoding::utf_8).c_str());
    return result.IsSuccess() ? std::move(result).GetValue() : "";
}

std::string UniConv::ToUtf8FromUtf16LE(const char16_t* input) {
    if (!input) return "";
    return ToUtf8FromUtf16LE(std::u16string(input));
}

// UTF-16BE -> UTF-8
std::string UniConv::ToUtf8FromUtf16BE(const std::u16string& input) {
    if (input.empty()) return "";
    
    std::string input_bytes(reinterpret_cast<const char*>(input.data()), input.size() * sizeof(char16_t));
    auto result = ConvertEncodingFast(input_bytes, ToString(Encoding::utf_16be).c_str(),ToString(Encoding::utf_8).c_str());
    return result.IsSuccess() ? std::move(result).GetValue() : "";
}

std::string UniConv::ToUtf8FromUtf16BE(const char16_t* input, size_t len) {
    if (!input || len == 0) return "";

    std::string input_bytes(reinterpret_cast<const char*>(input), len * sizeof(char16_t));
    auto result = ConvertEncodingFast(input_bytes, ToString(Encoding::utf_16be).c_str(), ToString(Encoding::utf_8).c_str());
    return result.IsSuccess() ? std::move(result).GetValue() : "";
}

std::string UniConv::ToUtf8FromUtf16BE(const char16_t* input) {
    if (!input) return "";
    return ToUtf8FromUtf16BE(std::u16string(input));
}

// UTF-8 -> UTF-16LE
std::u16string UniConv::ToUtf16LEFromUtf8(const std::string& input) {
    auto result = ConvertEncodingFast(input, ToString(Encoding::utf_8).c_str(), ToString(Encoding::utf_16le).c_str());
    if (result.IsSuccess()) {
        auto value = std::move(result).GetValue();
        if (value.size() % 2 == 0) {
            return std::u16string(reinterpret_cast<const char16_t*>(value.data()), 
                                 value.size() / 2);
        }
    }
    return std::u16string();
}

std::u16string UniConv::ToUtf16LEFromUtf8(const char* input) {
    if (!input) return std::u16string();
    return ToUtf16LEFromUtf8(std::string(input));
}

// UTF-8 -> UTF-16BE
std::u16string UniConv::ToUtf16BEFromUtf8(const std::string& input) {
    auto result = ConvertEncodingFast(input, ToString(Encoding::utf_8).c_str(), ToString(Encoding::utf_16be).c_str());
    if (result.IsSuccess()) {
        auto value = std::move(result).GetValue();
        if (value.size() % 2 == 0) {
            return std::u16string(reinterpret_cast<const char16_t*>(value.data()), 
                                 value.size() / 2);
        }
    }
    return std::u16string();
}

std::u16string UniConv::ToUtf16BEFromUtf8(const char* input) {
    if (!input) return std::u16string();
    return ToUtf16BEFromUtf8(std::string(input));
}

// UTF-16LE -> UTF-16BE
std::u16string UniConv::ToUtf16BEFromUtf16LE(const std::u16string& input) {
    if (input.empty()) return std::u16string();

    std::string input_bytes(reinterpret_cast<const char*>(input.data()), input.size() * sizeof(char16_t));
    auto result = ConvertEncodingFast(input_bytes, ToString(Encoding::utf_16le).c_str(), ToString(Encoding::utf_16be).c_str());
    if (result.IsSuccess()) {
        auto value = std::move(result).GetValue();
        if (value.size() % 2 == 0) {
            return std::u16string(reinterpret_cast<const char16_t*>(value.data()), 
                                 value.size() / 2);
        }
    }
    return std::u16string();
}

std::u16string UniConv::ToUtf16BEFromUtf16LE(const char16_t* input) {
    if (!input) return std::u16string();
    return ToUtf16BEFromUtf16LE(std::u16string(input));
}

// UTF-16BE -> UTF-16LE
std::u16string UniConv::ToUtf16LEFromUtf16BE(const std::u16string& input) {
    if (input.empty()) return std::u16string();
    
    std::string input_bytes(reinterpret_cast<const char*>(input.data()), input.size() * sizeof(char16_t));
    auto result = ConvertEncodingFast(input_bytes, ToString(Encoding::utf_16be).c_str(), ToString(Encoding::utf_16le).c_str());
    if (result.IsSuccess()) {
        auto value = std::move(result).GetValue();
        if (value.size() % 2 == 0) {
            return std::u16string(reinterpret_cast<const char16_t*>(value.data()), 
                                 value.size() / 2);
        }
    }
    return std::u16string();
}

std::u16string UniConv::ToUtf16LEFromUtf16BE(const char16_t* input) {
    if (!input) return std::u16string();
    size_t len = 0;
    while (input[len] != 0) ++len;
    return ToUtf16LEFromUtf16BE(std::u16string(input, len));
}

// New standardized method implementations
std::wstring UniConv::ToWideStringFromLocale(const std::string& input) {
    if (input.empty()) return std::wstring{};
    std::string currentEncoding = this->GetCurrentSystemEncoding();
    auto result = this->ConvertEncodingFast(input, currentEncoding.c_str(), ToString(Encoding::utf_16le).c_str());
    if (!result.IsSuccess()) return std::wstring{};
    // Remove the BOM (if any)
    auto value = std::move(result).GetValue();
    const char* data = value.data();
    size_t size = value.size();
    if (size >= 2 && (uint8_t)data[0] == 0xFF && (uint8_t)data[1] == 0xFE) {
        data += 2;
        size -= 2;
    }
    return std::wstring(reinterpret_cast<const wchar_t*>(data), size / sizeof(wchar_t));
}

std::wstring UniConv::ToWideStringFromLocale(const char* input) {
	if (!input) return std::wstring{};
	return this->ToWideStringFromLocale(std::string(input));
}



// New standardized method implementations
std::string UniConv::ToLocaleFromWideString(const std::wstring& input)
{
	if (input.empty()) return std::string{};
	std::string currentEncoding = this->GetCurrentSystemEncoding();
	auto result = this->ConvertEncodingFast
           (std::string(reinterpret_cast<const char*>(input.data()), input.size() * sizeof(wchar_t)),
               ToString(Encoding::wchar_t_encoding).c_str(),
               currentEncoding.c_str());
	return result.IsSuccess() ? std::move(result).GetValue() : std::string{};
}

std::string UniConv::ToLocaleFromWideString(const wchar_t* input)
{
	return input ? this->ToLocaleFromWideString(std::wstring(input)) : std::string{};
}



// UTF-16LE -> Local
std::string UniConv::ToLocaleFromUtf16LE(const std::u16string& input) {
    if (input.empty()) return "";
    
    std::string currentEncoding = this->GetCurrentSystemEncoding();
    std::string input_bytes(reinterpret_cast<const char*>(input.data()), input.size() * sizeof(char16_t));
    auto result = this->ConvertEncodingFast(input_bytes, ToString(Encoding::utf_16le).c_str(), currentEncoding.c_str());
    return result.IsSuccess() ? std::move(result).GetValue() : "";
}

std::string UniConv::ToLocaleFromUtf16LE(const char16_t* input) {
    if (!input) return "";
    return this->ToLocaleFromUtf16LE(std::u16string(input));
}

// ===================== Helper Functions =====================
std::string UniConv::WideStringToLocale(const std::wstring& sInput) {
#ifdef _WIN32
    if (sInput.empty()) return "";
    
    int bytes_needed = WideCharToMultiByte(CP_ACP, 0, sInput.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (bytes_needed <= 0) return "";
    
    std::string result(bytes_needed - 1, '\0');
    WideCharToMultiByte(CP_ACP, 0, sInput.c_str(), -1, &result[0], bytes_needed, nullptr, nullptr);    return result;
#else
    // Linux implementation - use iconv for conversion
    if (sInput.empty()) return std::string();
    
    // Convert wstring to UTF-32LE byte representation first
    std::string utf32_bytes;
    utf32_bytes.reserve(sInput.size() * 4);
    
    for (wchar_t wc : sInput) {
        // Convert each wchar_t to UTF-32LE bytes
        uint32_t codepoint = static_cast<uint32_t>(wc);
        utf32_bytes.push_back(static_cast<char>(codepoint & 0xFF));
        utf32_bytes.push_back(static_cast<char>((codepoint >> 8) & 0xFF));
        utf32_bytes.push_back(static_cast<char>((codepoint >> 16) & 0xFF));
        utf32_bytes.push_back(static_cast<char>((codepoint >> 24) & 0xFF));
    }
    
    // Use iconv to convert UTF-32LE to locale encoding
    std::string currentEncoding = GetCurrentSystemEncoding(); 
    auto result = ConvertEncodingFast(utf32_bytes, "UTF-32LE", currentEncoding.c_str());
    return result.IsSuccess() ? result.GetValue() : std::string{};
#endif
}

std::string UniConv::WideStringToLocale(const wchar_t* sInput) {
    if (!sInput) return "";
    return WideStringToLocale(std::wstring(sInput));
}

std::string UniConv::ToUtf8FromUtf32LE(const std::u32string& sInput)
{
	if (sInput.empty()) return std::string{};
	std::string input_bytes(reinterpret_cast<const char*>(sInput.data()), sInput.size() * sizeof(char32_t));
	auto result = this->ConvertEncodingFast(input_bytes, ToString(Encoding::utf_32le).c_str(), ToString(Encoding::utf_8).c_str());
	return result.IsSuccess() ? std::move(result).GetValue() : std::string{};
}

std::string UniConv::ToUtf8FromUtf32LE(const char32_t* sInput)
{
	if (!sInput) return std::string{};
	return ToUtf8FromUtf32LE(std::u32string(sInput));
}

std::u16string UniConv::ToUtf16LEFromUtf32LE(const std::u32string& sInput)
{
	if (sInput.empty() ) return std::u16string{};
	std::string input_bytes(reinterpret_cast<const char*>(sInput.data()), sInput.size() * sizeof(char32_t));
	auto result = this->ConvertEncodingFast(input_bytes, ToString(Encoding::utf_32le).c_str(), ToString(Encoding::utf_16le).c_str());
    if (result.IsSuccess()) {
        auto value = std::move(result).GetValue();
        if (value.size() % 2 == 0) {
            return std::u16string(reinterpret_cast<const char16_t*>(value.data()), 
               value.size() / sizeof(char16_t));
        }
    }
    return std::u16string{};
}

std::u16string UniConv::ToUtf16LEFromUtf32LE(const char32_t* sInput)
{
	if (!sInput) return std::u16string{};
	return ToUtf16LEFromUtf32LE(std::u32string(sInput));
}

// New standardized method implementations
std::u16string UniConv::ToUtf16BEFromUtf32LE(const std::u32string& input)
{
	if (input.empty()) return std::u16string{};
	std::string input_bytes(reinterpret_cast<const char*>(input.data()), input.size() * sizeof(char32_t));
    auto result = this->ConvertEncodingFast(input_bytes, ToString(Encoding::utf_32le).c_str(), ToString(Encoding::utf_16be).c_str());
    if (result.IsSuccess()) {
        auto value = std::move(result).GetValue();
        if (value.size() % 2 == 0) {
            return std::u16string(reinterpret_cast<const char16_t*>(value.data()), 
               value.size() / sizeof(char16_t));
        }
    }
    return std::u16string{};
}

std::u16string UniConv::ToUtf16BEFromUtf32LE(const char32_t* input)
{
   if (!input) return std::u16string{};
   return ToUtf16BEFromUtf32LE(std::u32string(input));
}



// New standardized method implementations
std::u32string UniConv::ToUtf32LEFromUtf8(const std::string& input)
{
	if (input.empty()) return std::u32string{};
	auto result = this->ConvertEncodingFast(input, ToString(Encoding::utf_8).c_str(), ToString(Encoding::utf_32le).c_str());
    if (result.IsSuccess()) {
        auto value = std::move(result).GetValue();
        if (value.size() % 4 == 0) {
            return std::u32string(reinterpret_cast<const char32_t*>(value.data()), 
                                 value.size() / sizeof(char32_t));
        }
    }
	return std::u32string{};
}

std::u32string UniConv::ToUtf32LEFromUtf8(const char* input)
{
	if (!input) return std::u32string{};
	return ToUtf32LEFromUtf8(std::string(input));
}



// New standardized method implementations
std::u32string UniConv::ToUtf32LEFromUtf16LE(const std::u16string& input)
{
	if (input.empty()) return std::u32string{};
	std::string input_bytes(reinterpret_cast<const char*>(input.data()), input.size() * sizeof(char16_t));
    auto result = this->ConvertEncodingFast(input_bytes, ToString(Encoding::utf_16le).c_str(), ToString(Encoding::utf_32le).c_str());
    if (result.IsSuccess()) {
        auto value = std::move(result).GetValue();
        if (value.size() % 4 == 0) {
            return std::u32string(reinterpret_cast<const char32_t*>(value.data()), 
                                 value.size() / sizeof(char32_t));
        }
	}
	return std::u32string{};
}

std::u32string UniConv::ToUtf32LEFromUtf16LE(const char16_t* input)
{
	if (input == nullptr) return std::u32string{};
	return ToUtf32LEFromUtf16LE(std::u16string(input));
}



// New standardized method implementations
std::u32string UniConv::ToUtf32LEFromUtf16BE(const std::u16string& input)
{
	if (input.empty()) return std::u32string{};
    std::string input_bytes(reinterpret_cast<const char*>(input.data()), input.size() * sizeof(char16_t));
    auto result = this->ConvertEncodingFast(input_bytes, ToString(Encoding::utf_16be).c_str(), ToString(Encoding::utf_32le).c_str());
    if (result.IsSuccess()) {
        auto value = std::move(result).GetValue();
        if (value.size() % 4 == 0) {
            return std::u32string(reinterpret_cast<const char32_t*>(value.data()), 
                                 value.size() / sizeof(char32_t));
        }
	}
	return std::u32string{};
}

std::u32string UniConv::ToUtf32LEFromUtf16BE(const char16_t* input)
{
	if (input == nullptr) return std::u32string{};
	return ToUtf32LEFromUtf16BE(std::u16string(input));
}



// New standardized method implementations
std::string UniConv::ToUtf8FromUcs4(const std::wstring& input)
{
    #if defined(_WIN32)
        // Windows platform：Use Windows API WideCharToMultiByte
        if (input.empty()) return std::string();
    
        int sizeRequired = WideCharToMultiByte(
            CP_UTF8, 
            0,
            input.data(), 
            (int)input.size(),
            nullptr, 
            0, 
            nullptr,
            nullptr);

        if (sizeRequired <= 0)
        {
    		return std::string{}; // Error or empty string
        }
        std::vector<char> utf8Buffer(sizeRequired);
        WideCharToMultiByte(CP_UTF8, 0, input.data(), (int)input.size(), utf8Buffer.data(), sizeRequired, nullptr, nullptr);
        return std::string(utf8Buffer.begin(), utf8Buffer.end());

    #elif defined(__linux__) || defined(__APPLE__)
        if (input.empty()) return std::string();
        std::string utf8_str = reinterpret_cast<const char*>(input.data());
        auto result = ConvertEncodingFast(utf8_str, ToString(Encoding::wchar_t_encoding).c_str(), ToString(Encoding::utf_8).c_str());
        return result.IsSuccess() ? std::move(result).GetValue() : std::string{};
    #endif
}

std::wstring UniConv::ToUcs4FromUtf8(const std::string& input)
{
    if (input.empty()) return std::wstring{};
    // Convert UTF-8 to UCS-4 (wide string)
    auto result = ConvertEncodingFast(input, ToString(Encoding::utf_8).c_str(), ToString(Encoding::utf_16le).c_str());
    if (result.IsSuccess()) {
        auto value = std::move(result).GetValue();
        if (value.size() % sizeof(wchar_t) == 0) {
            return std::wstring(reinterpret_cast<const wchar_t*>(value.data()),
                value.size() / sizeof(wchar_t));
        }
    }
    return std::wstring{};
}



std::wstring UniConv::U16StringToWString(const std::u16string& u16str)
{
    if (u16str.empty()) return std::wstring{};
    auto result = ConvertEncodingFast(std::string(reinterpret_cast<const char*>(u16str.data()), u16str.size() * sizeof(char16_t)),
        ToString(Encoding::utf_16le).c_str(), ToString(Encoding::wchar_t_encoding).c_str());
    if (result.IsSuccess()) {
        auto value = std::move(result).GetValue();
        if (value.size() % sizeof(wchar_t) == 0) {
            return std::wstring(reinterpret_cast<const wchar_t*>(value.data()),
                value.size() / sizeof(wchar_t));
        }
    }
    return std::wstring{};
}

std::wstring UniConv::U16StringToWString(const char16_t* u16str)
{
    if (!u16str) return std::wstring{};
    return U16StringToWString(std::u16string(u16str));
}


// ===================== Error Handling Related =====================

UniConv::IconvSharedPtr UniConv::GetIconvDescriptor(const char* fromcode, const char* tocode)
{
	// 参数有效性检查 - 预测参数通常有效
	if (UNICONV_UNLIKELY(!fromcode || !tocode)) {
        m_cacheMissCount.fetch_add(1, std::memory_order_relaxed);
        return nullptr;
    }

    // 使用预计算哈希作为缓存键 - 避免字符串拼接分配
    const size_t from_len = strlen(fromcode);
    const size_t to_len   = strlen(tocode);
    const uint64_t key = detail::MakeEncodingPairKey(fromcode, from_len, tocode, to_len);

    // Lock-free cache lookup using parallel-hashmap's concurrent operations
    // Try to find in cache (thread-safe read)
    IconvCacheEntry entry;
    if (UNICONV_LIKELY(m_iconvDescriptorCacheMap.if_contains(key, [&](const auto& item) {
        entry = item.second;
        item.second.UpdateAccess();
    }))) {
        // Cache hit - update statistics
        m_cacheHitCount.fetch_add(1, std::memory_order_relaxed);
        return entry.descriptor;
    }

    // Cache miss - need to create new descriptor
    m_cacheMissCount.fetch_add(1, std::memory_order_relaxed);
    
    // Create new iconv descriptor
    iconv_t cd = iconv_open(tocode, fromcode);
    if (UNICONV_UNLIKELY(cd == reinterpret_cast<iconv_t>(-1))) {
        #if defined(UNICONV_DEBUG_MODE) && UNICONV_DEBUG_MODE
        // std::cout << "iconv_open error for " << fromcode << ">" << tocode << std::endl;
        #endif
        return nullptr;
    }

    // Create smart pointer with custom deleter (cross-platform safe)
    // Cast iconv_t to void* for std::shared_ptr<void>
    auto iconvPtr = std::shared_ptr<void>(static_cast<void*>(cd), IconvDeleter());
    
    // LRU cache size management (lock-free)
    if (UNICONV_UNLIKELY(m_iconvDescriptorCacheMap.size() >= MAX_CACHE_SIZE)) {
        EvictLRUCacheEntries();
    }
    
    // Insert new cache entry (thread-safe insert or update)
    IconvCacheEntry new_entry(iconvPtr);
    m_iconvDescriptorCacheMap.insert_or_assign(key, std::move(new_entry));

    #if defined(UNICONV_DEBUG_MODE) && UNICONV_DEBUG_MODE
        // std::wcout << "Create and cached iconv descriptor: " << fromcode << ">" << tocode << std::endl;
    #endif

    return iconvPtr;
}

std::pair<UniConv::BomEncoding, std::string_view> UniConv::DetectAndRemoveBom(const std::string_view& data)
{
    const unsigned char* bytes = reinterpret_cast<const unsigned char*>(data.data());
    size_t len = data.size();

    // UTF-8 BOM (3 bytes)
    if (len >= 3 && bytes[0] == 0xEF && bytes[1] == 0xBB && bytes[2] == 0xBF)
        return { BomEncoding::UTF8, data.substr(3) };
    
    // UTF-32 检测必须在 UTF-16 之前！（UTF-32LE BOM 以 FF FE 开头，会被误判为 UTF-16LE）
    if (len >= 4 && bytes[0] == 0xFF && bytes[1] == 0xFE && bytes[2] == 0x00 && bytes[3] == 0x00)
        return { BomEncoding::UTF32_LE, data.substr(4) };
    if (len >= 4 && bytes[0] == 0x00 && bytes[1] == 0x00 && bytes[2] == 0xFE && bytes[3] == 0xFF)
        return { BomEncoding::UTF32_BE, data.substr(4) };
    
    // UTF-16 检测（在 UTF-32 之后）
    if (len >= 2 && bytes[0] == 0xFF && bytes[1] == 0xFE)
        return { BomEncoding::UTF16_LE, data.substr(2) };
    if (len >= 2 && bytes[0] == 0xFE && bytes[1] == 0xFF)
        return { BomEncoding::UTF16_BE, data.substr(2) };

    return { BomEncoding::None, data };
}

std::pair<UniConv::BomEncoding, std::wstring_view> UniConv::DetectAndRemoveBom(const std::wstring_view& data)
{
    if (data.empty()) {
        return { BomEncoding::None, data };
    }

    if constexpr (sizeof(wchar_t) == 2) {
        // Windows ：UTF-16
        if (data[0] == 0xFEFF)
            return { BomEncoding::UTF16_BE, data.substr(1) };
        if (data[0] == 0xFFFE)
            return { BomEncoding::UTF16_LE, data.substr(1) };
    }
    else if constexpr (sizeof(wchar_t) == 4) {
        // Linux/macOS：UTF-32
        if (data.size() >= 1 && data[0] == 0x0000FEFF)
            return { BomEncoding::UTF32_BE, data.substr(1) };
        if (data.size() >= 1 && data[0] == 0xFFFE0000)
            return { BomEncoding::UTF32_LE, data.substr(1) };
    }

    return { BomEncoding::None, data };
}


void UniConv::CleanupIconvCache() {
    // Lock-free clear using phmap's thread-safe operations
    m_iconvDescriptorCacheMap.clear();
    
    #if defined(UNICONV_DEBUG_MODE) && UNICONV_DEBUG_MODE
        // std::cout << "iconv descriptor cache cleared" << std::endl;
    #endif
}

std::string UniConv::ToString(UniConv::Encoding  enc) noexcept {
    int idx = static_cast<int>(enc);
    if (idx >= 0 && idx < static_cast<int>(Encoding::count))
        return m_encodingNames[idx];
    return {};
}

//----------------------------------------------------------------------------------------------------------------------
// === High-Performance Methods Implementation ===
//----------------------------------------------------------------------------------------------------------------------

StringResult UniConv::ConvertEncodingFast(const std::string& input,const char* fromEncoding, const char* toEncoding) noexcept {
    // 快速参数检查 - 预测参数通常有效
    if (UNICONV_UNLIKELY(!fromEncoding || !toEncoding)) {
        return StringResult::Failure(ErrorCode::InvalidParameter);
    }
    
    // 快速编码名称有效性检查
    if (UNICONV_UNLIKELY(!IsValidEncodingName(fromEncoding))) {
        return StringResult::Failure(ErrorCode::InvalidSourceEncoding);
    }
    if (UNICONV_UNLIKELY(!IsValidEncodingName(toEncoding))) {
        return StringResult::Failure(ErrorCode::InvalidTargetEncoding);
    }

    // 预测输入通常不为空
    if (UNICONV_UNLIKELY(input.empty())) {
        return StringResult::Success(std::string{});
    }

    // 同编码直接返回（零转换开销）
    // 使用不区分大小写的比较，因为编码名称可能有大小写差异
    if (UNICONV_LIKELY(CompareEncodingNamesEqual(fromEncoding, toEncoding))) {
        return StringResult::Success(std::string(input));
    }
    
    // ASCII 内容的快速处理
    // 对于 ASCII 兼容编码（UTF-8, ISO-8859-*, Windows-125*, ASCII 等），
    // 如果内容全是 ASCII（所有字节 < 0x80），可以直接复制
    if (IsAsciiCompatibleEncoding(fromEncoding) && IsAsciiCompatibleEncoding(toEncoding)) {
        if (UNICONV_LIKELY(IsAllAscii(input))) {
            return StringResult::Success(std::string(input));
        }
    }
    
    //==========================================================================
    //  simdutf SIMD 加速快速路径（可选）
    //==========================================================================
#ifdef UNICONV_HAS_SIMDUTF
    // UTF-8 → UTF-16LE
    if (IsUtf8ToUtf16LE(fromEncoding, toEncoding)) {
        return ConvertUtf8ToUtf16LE_SIMD(input);
    }
    // UTF-16LE → UTF-8
    if (IsUtf16LEToUtf8(fromEncoding, toEncoding)) {
        return ConvertUtf16LEToUtf8_SIMD(input);
    }
    // UTF-8 → UTF-16BE
    if (IsUtf8ToUtf16BE(fromEncoding, toEncoding)) {
        return ConvertUtf8ToUtf16BE_SIMD(input);
    }
    // UTF-16BE → UTF-8
    if (IsUtf16BEToUtf8(fromEncoding, toEncoding)) {
        return ConvertUtf16BEToUtf8_SIMD(input);
    }
#endif // UNICONV_HAS_SIMDUTF
    //==========================================================================

    // 预取输入数据到缓存
    UNICONV_PREFETCH(input.data(), 0, 3);

    // 使用LRU缓存的iconv描述符
    auto descriptor = GetIconvDescriptor(fromEncoding, toEncoding);
    if (UNICONV_UNLIKELY(!descriptor)) {
        return StringResult::Failure(ErrorCode::ConversionFailed);
    }

    // Cast from shared_ptr<void> to iconv_t (cross-platform safe)
    iconv_t cd = static_cast<iconv_t>(descriptor.get());

    // 输入缓冲区设置
    const char* inbuf_ptr = input.data();
    std::size_t inbuf_left = input.size();

    // 使用智能估算预分配输出缓冲区
    std::string result;
    size_t estimated_size = EstimateOutputSize(input.size(), fromEncoding, toEncoding);
    result.reserve(estimated_size);

    // 使用临时缓冲区进行转换，基于智能估算调整大小
    std::size_t buffer_size = (std::max)(static_cast<std::size_t>(4096), estimated_size);
    std::vector<char> temp_buffer(buffer_size);

    // 转换循环，限制最大迭代次数防止死循环 - 预测通常一次转换成功
    constexpr int max_iterations = 100;
    int iteration_count = 0;

    // 预测大多数情况下单次转换即可完成
    while (UNICONV_LIKELY(inbuf_left > 0) && UNICONV_LIKELY(iteration_count++ < max_iterations)) {
        char* outbuf_ptr = temp_buffer.data();
        std::size_t outbuf_left = temp_buffer.size();

        // 预取临时缓冲区到缓存
        UNICONV_PREFETCH(temp_buffer.data(), 1, 2);

        // 执行转换
        std::size_t ret = iconv(cd, &inbuf_ptr, &inbuf_left, &outbuf_ptr, &outbuf_left);

        // 计算转换的字节数 - 预测通常有数据被转换
        std::size_t converted_bytes = temp_buffer.size() - outbuf_left;
        if (UNICONV_LIKELY(converted_bytes > 0)) {
            result.append(temp_buffer.data(), converted_bytes);
        }

        // 错误处理 - 预测大多数情况下转换成功
        if (UNICONV_UNLIKELY(static_cast<std::size_t>(-1) == ret)) {
            int current_errno = errno;
            switch (current_errno) {
                case E2BIG:
                    // 输出缓冲区太小，扩容继续 - 预测不会超过限制
                    if (UNICONV_UNLIKELY(temp_buffer.size() >= 1048576 * 10)) { // 10MB限制
                        return StringResult::Failure(ErrorCode::BufferTooSmall);
                    }
                    temp_buffer.resize(temp_buffer.size() * 2);
                    continue;
                case EILSEQ:
                    return StringResult::Failure(ErrorCode::InvalidSequence);
                case EINVAL:
                    return StringResult::Failure(ErrorCode::IncompleteSequence);
                default:
                    return StringResult::Failure(ErrorCode::ConversionFailed);
            }
        } else {
            // 转换成功完成 - 最常见的情况
            break;
        }
    }

    // 预测很少会达到最大迭代次数
    if (UNICONV_UNLIKELY(iteration_count >= max_iterations)) {
        return StringResult::Failure(ErrorCode::InternalError);
    }

    return StringResult::Success(std::move(result));
}

const char* UniConv::GetEncodingNamePtr(int codepage) noexcept {
    // 预测codepage存在于映射中
    auto it = m_encodingMap.find(static_cast<std::uint16_t>(codepage));
    return UNICONV_LIKELY(it != m_encodingMap.end()) ? it->second.dotNetName.c_str() : nullptr;
}

StringViewResult UniConv::GetEncodingNameFast(int codepage) noexcept {
    // 预测codepage存在于映射中
    auto it = m_encodingMap.find(static_cast<std::uint16_t>(codepage));
    if (UNICONV_LIKELY(it != m_encodingMap.end())) {
        return StringViewResult::Success(std::string_view{it->second.dotNetName});
    } else {
        return StringViewResult::Failure(ErrorCode::EncodingNotFound);
    }
}

IntResult UniConv::GetSystemCodePageFast() noexcept {
#ifdef _WIN32
    UINT codepage = GetACP();
    return IntResult::Success(static_cast<int>(codepage));
#elif defined(__linux__)
    // Linux下的实现
    setlocale(LC_ALL, "");
    char* encoding = nl_langinfo(CODESET);
    if (encoding) {
        auto it = m_encodingToCodePageMap.find(encoding);
        if (it != m_encodingToCodePageMap.end()) {
            return IntResult::Success(static_cast<int>(it->second));
        }
    }
    return IntResult::Failure(ErrorCode::SystemError);
#else
    return IntResult::Failure(ErrorCode::SystemError);
#endif
}

//----------------------------------------------------------------------------------------------------------------------
// === High-Performance Internal Methods Implementation ===
//----------------------------------------------------------------------------------------------------------------------

// ===================================================================================================================
// Encoding-Aware Output Size Estimation (P3 Optimization)
// ===================================================================================================================

namespace {
    // Encoding ID for fast lookup (avoid string comparisons in hot path)
    enum class EncodingId : uint8_t {
        Unknown = 0,
        UTF8,
        UTF16,
        UTF16LE,
        UTF16BE,
        UTF32,
        UTF32LE,
        UTF32BE,
        ASCII,
        GBK,
        GB2312,
        GB18030,
        BIG5,
        ShiftJIS,
        ISO8859_1,
        Windows1252,
        EUC_JP,
        EUC_KR
    };
    
    // Fast encoding name to ID lookup using hash
    EncodingId GetEncodingId(const char* encoding) noexcept {
        if (!encoding) return EncodingId::Unknown;
        
        // Normalize to uppercase for comparison
        char normalized[32];
        size_t len = 0;
        for (; encoding[len] && len < 31; ++len) {
            char c = encoding[len];
            normalized[len] = (c >= 'a' && c <= 'z') ? (c - 32) : c;
        }
        normalized[len] = '\0';
        
        // Use hash for fast lookup
        uint32_t hash = detail::fnv1a_hash(normalized, len);
        
        // Common encodings (ordered by frequency)
        switch (hash) {
            case detail::fnv1a_hash("UTF-8", 5):
            case detail::fnv1a_hash("UTF8", 4):
                return EncodingId::UTF8;
            case detail::fnv1a_hash("UTF-16", 6):
            case detail::fnv1a_hash("UTF16", 5):
                return EncodingId::UTF16;
            case detail::fnv1a_hash("UTF-16LE", 8):
            case detail::fnv1a_hash("UTF16LE", 7):
                return EncodingId::UTF16LE;
            case detail::fnv1a_hash("UTF-16BE", 8):
            case detail::fnv1a_hash("UTF16BE", 7):
                return EncodingId::UTF16BE;
            case detail::fnv1a_hash("UTF-32", 6):
            case detail::fnv1a_hash("UTF32", 5):
                return EncodingId::UTF32;
            case detail::fnv1a_hash("UTF-32LE", 8):
            case detail::fnv1a_hash("UTF32LE", 7):
                return EncodingId::UTF32LE;
            case detail::fnv1a_hash("UTF-32BE", 8):
            case detail::fnv1a_hash("UTF32BE", 7):
                return EncodingId::UTF32BE;
            case detail::fnv1a_hash("ASCII", 5):
            case detail::fnv1a_hash("US-ASCII", 8):
                return EncodingId::ASCII;
            case detail::fnv1a_hash("GBK", 3):
                return EncodingId::GBK;
            case detail::fnv1a_hash("GB2312", 6):
                return EncodingId::GB2312;
            case detail::fnv1a_hash("GB18030", 7):
                return EncodingId::GB18030;
            case detail::fnv1a_hash("BIG5", 4):
            case detail::fnv1a_hash("BIG-5", 5):
                return EncodingId::BIG5;
            case detail::fnv1a_hash("SHIFT_JIS", 9):
            case detail::fnv1a_hash("SHIFT-JIS", 9):
            case detail::fnv1a_hash("SJIS", 4):
                return EncodingId::ShiftJIS;
            case detail::fnv1a_hash("ISO-8859-1", 10):
            case detail::fnv1a_hash("LATIN1", 6):
                return EncodingId::ISO8859_1;
            case detail::fnv1a_hash("WINDOWS-1252", 12):
            case detail::fnv1a_hash("CP1252", 6):
                return EncodingId::Windows1252;
            case detail::fnv1a_hash("EUC-JP", 6):
            case detail::fnv1a_hash("EUCJP", 5):
                return EncodingId::EUC_JP;
            case detail::fnv1a_hash("EUC-KR", 6):
            case detail::fnv1a_hash("EUCKR", 5):
                return EncodingId::EUC_KR;
            default:
                // Fallback: check for common substrings
                if (strstr(normalized, "UTF-8") || strstr(normalized, "UTF8")) return EncodingId::UTF8;
                if (strstr(normalized, "UTF-16") || strstr(normalized, "UTF16")) return EncodingId::UTF16;
                if (strstr(normalized, "UTF-32") || strstr(normalized, "UTF32")) return EncodingId::UTF32;
                if (strstr(normalized, "GBK")) return EncodingId::GBK;
                if (strstr(normalized, "GB18030")) return EncodingId::GB18030;
                if (strstr(normalized, "GB2312")) return EncodingId::GB2312;
                return EncodingId::Unknown;
        }
    }
    
    // Encoding expansion factor table (from_id, to_id) -> factor
    // Factor < 1.0 means output is smaller, > 1.0 means output is larger
    constexpr double GetExpansionFactor(EncodingId from, EncodingId to) noexcept {
        // Same encoding family optimizations
        if (from == to) return 1.0;
        
        // UTF-8 conversions
        if (from == EncodingId::UTF8) {
            switch (to) {
                case EncodingId::UTF16:
                case EncodingId::UTF16LE:
                case EncodingId::UTF16BE:
                    return 2.0;  // UTF-8 can expand to 2x in UTF-16
                case EncodingId::UTF32:
                case EncodingId::UTF32LE:
                case EncodingId::UTF32BE:
                    return 4.0;  // UTF-8 can expand to 4x in UTF-32
                case EncodingId::ASCII:
                    return 1.0;  // ASCII subset stays same
                case EncodingId::GBK:
                case EncodingId::GB2312:
                case EncodingId::GB18030:
                case EncodingId::BIG5:
                    return 1.0;  // Similar size for CJK
                default:
                    return 1.5;
            }
        }
        
        // UTF-16 conversions
        if (from == EncodingId::UTF16 || from == EncodingId::UTF16LE || from == EncodingId::UTF16BE) {
            switch (to) {
                case EncodingId::UTF8:
                    return 1.5;  // UTF-16 to UTF-8 usually similar or smaller
                case EncodingId::UTF32:
                case EncodingId::UTF32LE:
                case EncodingId::UTF32BE:
                    return 2.0;  // Each UTF-16 unit becomes 4 bytes
                case EncodingId::ASCII:
                    return 0.5;  // 2 bytes -> 1 byte for ASCII range
                case EncodingId::GBK:
                case EncodingId::GB2312:
                case EncodingId::GB18030:
                    return 1.0;  // Similar size
                default:
                    return 1.0;
            }
        }
        
        // UTF-32 conversions
        if (from == EncodingId::UTF32 || from == EncodingId::UTF32LE || from == EncodingId::UTF32BE) {
            switch (to) {
                case EncodingId::UTF8:
                    return 1.0;  // UTF-32 to UTF-8 usually shrinks
                case EncodingId::UTF16:
                case EncodingId::UTF16LE:
                case EncodingId::UTF16BE:
                    return 0.5;  // 4 bytes -> 2 bytes typical
                default:
                    return 0.5;
            }
        }
        
        // CJK encodings (GBK, GB2312, GB18030, BIG5)
        if (from == EncodingId::GBK || from == EncodingId::GB2312 || 
            from == EncodingId::GB18030 || from == EncodingId::BIG5) {
            switch (to) {
                case EncodingId::UTF8:
                    return 1.5;  // CJK characters: 2 bytes -> 3 bytes
                case EncodingId::UTF16:
                case EncodingId::UTF16LE:
                case EncodingId::UTF16BE:
                    return 1.0;  // Similar size
                case EncodingId::UTF32:
                case EncodingId::UTF32LE:
                case EncodingId::UTF32BE:
                    return 2.0;  // 2 bytes -> 4 bytes
                default:
                    return 1.2;
            }
        }
        
        // ASCII/Latin1 conversions
        if (from == EncodingId::ASCII || from == EncodingId::ISO8859_1 || from == EncodingId::Windows1252) {
            switch (to) {
                case EncodingId::UTF8:
                    return 1.5;  // Extended ASCII chars become multi-byte
                case EncodingId::UTF16:
                case EncodingId::UTF16LE:
                case EncodingId::UTF16BE:
                    return 2.0;  // 1 byte -> 2 bytes
                case EncodingId::UTF32:
                case EncodingId::UTF32LE:
                case EncodingId::UTF32BE:
                    return 4.0;  // 1 byte -> 4 bytes
                default:
                    return 1.2;
            }
        }
        
        // Default: conservative estimate
        return 2.0;
    }
}

size_t UniConv::EstimateOutputSize(size_t input_size, const char* from_encoding, const char* to_encoding) noexcept {
    // Fast path: empty input
    if (input_size == 0) return 512;  // Minimum buffer
    
    // Fast path: same encoding
    if (from_encoding && to_encoding && CompareEncodingNamesEqual(from_encoding, to_encoding)) {
        return input_size + 16;  // Just add small safety margin
    }
    
    // Get encoding IDs for fast lookup
    EncodingId from_id = GetEncodingId(from_encoding);
    EncodingId to_id = GetEncodingId(to_encoding);
    
    // Get expansion factor from table
    double expansion_factor = GetExpansionFactor(from_id, to_id);
    
    // Add safety margin (15%)
    double estimated = static_cast<double>(input_size) * expansion_factor * 1.15;
    
    // Clamp to reasonable bounds
    constexpr size_t MIN_BUFFER_SIZE = 512;
    constexpr size_t MAX_REASONABLE_SIZE = 32 * 1024 * 1024; // 32MB
    
    return std::clamp(static_cast<size_t>(estimated), MIN_BUFFER_SIZE, MAX_REASONABLE_SIZE);
}

bool UniConv::IsValidEncodingName(const char* encoding) noexcept {
    if (UNICONV_UNLIKELY(!encoding || strlen(encoding) == 0)) {
        return false;
    }
    
    // 使用静态集合进行 O(1) 查找（线程安全的静态初始化）
    // 使用 lambda 初始化以访问类的私有静态成员
    static const std::unordered_set<std::string> valid_encodings = []() {
        std::unordered_set<std::string> valid_set;
        valid_set.reserve(256);  // 预留足够空间
        
        // 从 m_encodingMap 提取标准编码名称
        for (const auto& [cp, info] : m_encodingMap) {
            valid_set.insert(info.dotNetName);
        }
        
        // 从 m_encodingToCodePageMap 提取所有已知编码
        for (const auto& [name, cp] : m_encodingToCodePageMap) {
            valid_set.insert(name);
        }
        
        // 添加常见别名和大小写变体
        const char* common_aliases[] = {
            // UTF 系列
            "utf-8", "UTF8", "utf8",
            "utf-16", "UTF16", "utf16",
            "utf-32", "UTF32", "utf32",
            "utf-16le", "UTF-16LE", "utf16le",
            "utf-16be", "UTF-16BE", "utf16be",
            "utf-32le", "UTF-32LE", "utf32le",
            "utf-32be", "UTF-32BE", "utf32be",
            // 中文编码
            "gb2312", "GB2312", "gbk", "GBK",
            "gb18030", "GB18030",
            "big5", "BIG5", "Big5",
            // 其他常见编码
            "ascii", "ASCII", "us-ascii", "US-ASCII",
            "iso-8859-1", "ISO-8859-1", "latin1", "LATIN1",
            "windows-1252", "WINDOWS-1252", "cp1252", "CP1252",
            "shift_jis", "SHIFT_JIS", "sjis", "SJIS",
            "euc-jp", "EUC-JP", "eucjp", "EUCJP",
            "euc-kr", "EUC-KR", "euckr", "EUCKR",
            "euc-cn", "EUC-CN", "euccn", "EUCCN"
        };
        
        for (const char* alias : common_aliases) {
            valid_set.insert(alias);
        }
        
        return valid_set;
    }();
    
    // 直接查找，避免前缀匹配和字符验证的开销
    return valid_encodings.find(encoding) != valid_encodings.end();
}

constexpr int UniConv::GetEncodingMultiplier(const char* encoding) noexcept {
    if (!encoding) return 4;

    std::string_view enc{encoding};

    // UTF编码
    if (enc.find("UTF-32") != std::string_view::npos ||
        enc.find("UCS-4") != std::string_view::npos) return 4;

    if (enc.find("UTF-16") != std::string_view::npos ||
        enc.find("UCS-2") != std::string_view::npos) return 4;  // 考虑代理对

    if (enc.find("UTF-8") != std::string_view::npos) return 4;

    // 中文编码
    if (enc.find("GBK") != std::string_view::npos ||
        enc.find("GB2312") != std::string_view::npos ||
        enc.find("GB18030") != std::string_view::npos) return 4;  // GB18030可达4字节

    if (enc.find("Big5") != std::string_view::npos) return 2;

    // 欧洲编码
    if (enc.find("ISO-8859") != std::string_view::npos) return 1;

    if (enc.find("CP1252") != std::string_view::npos) return 1;
    
    // 日文编码
    if (enc.find("SHIFT_JIS") != std::string_view::npos ||
        enc.find("EUC-JP") != std::string_view::npos) return 3;
    
    // 默认保守估计
    return 4;
}

StringResult UniConv::ConvertEncodingInternal(const std::string& input,const char* fromEncoding,const char* toEncoding,StringBufferPool::BufferLease& buffer_lease,size_t estimated_size) noexcept {
    // 更新统计
    m_totalConversions.fetch_add(1, std::memory_order_relaxed);

    // 获取缓冲区
    std::string& result = buffer_lease.get();
    result.clear();

    // 预分配容量
    try {
        result.reserve(estimated_size);
    } catch (...) {
        return StringResult::Failure(ErrorCode::OutOfMemory);
    }

    // 获取iconv描述符
    auto descriptor = GetIconvDescriptor(fromEncoding, toEncoding);
    if (!descriptor) {
        return StringResult::Failure(ErrorCode::InvalidSourceEncoding);
    }

    // 准备转换参数
    const char* inbuf = input.data();
    size_t inbytesleft = input.size();

    // 使用栈分配的临时缓冲区进行分块转换
    constexpr size_t CHUNK_SIZE = 8192;  // 8KB块大小
    char temp_chunk[CHUNK_SIZE];

    // 转换循环
    constexpr int MAX_ITERATIONS = 1000;  // 防止无限循环
    int iteration_count = 0;

    while (inbytesleft > 0 && iteration_count < MAX_ITERATIONS) {
        char* outbuf = temp_chunk;
        size_t outbytesleft = CHUNK_SIZE;
        
        size_t converted = iconv(descriptor.get(), &inbuf, &inbytesleft,&outbuf, &outbytesleft);
        
        // 添加已转换的数据到结果
        size_t chunk_converted = CHUNK_SIZE - outbytesleft;
        if (chunk_converted > 0) {
            try {
                result.append(temp_chunk, chunk_converted);
            } catch (...) {
                return StringResult::Failure(ErrorCode::OutOfMemory);
            }
        }
        
        if (converted == (size_t)-1) {
            switch (errno) {
                case E2BIG:
                    // 输出缓冲区满，继续下一轮
                    continue;
                case EILSEQ:
                    return StringResult::Failure(ErrorCode::InvalidSequence);
                case EINVAL:
                    return StringResult::Failure(ErrorCode::IncompleteSequence);
                default:
                    return StringResult::Failure(ErrorCode::ConversionFailed);
            }
        }
        
        ++iteration_count;
    }
    
    if (iteration_count >= MAX_ITERATIONS) {
        return StringResult::Failure(ErrorCode::InternalError);
    }
    
    // 成功，返回结果
    return StringResult::Success(std::move(result));
}

//----------------------------------------------------------------------------------------------------------------------
// === Batch Processing Methods Implementation ===
//----------------------------------------------------------------------------------------------------------------------

std::vector<StringResult> UniConv::ConvertEncodingBatch(const std::vector<std::string>& inputs,const char* fromEncoding,const char* toEncoding) noexcept {
    std::vector<StringResult> results;
    results.reserve(inputs.size());
    
    // 参数验证 - 预测参数通常有效
    if (UNICONV_UNLIKELY(!fromEncoding || !toEncoding)) {
        // 为所有输入返回错误结果
        for (size_t i = 0; i < inputs.size(); ++i) {
            results.emplace_back(StringResult::Failure(ErrorCode::InvalidParameter));
        }
        return results;
    }
    
    // 快速编码名称有效性检查
    if (UNICONV_UNLIKELY(!IsValidEncodingName(fromEncoding))) {
        for (size_t i = 0; i < inputs.size(); ++i) {
            results.emplace_back(StringResult::Failure(ErrorCode::InvalidSourceEncoding));
        }
        return results;
    }
    if (UNICONV_UNLIKELY(!IsValidEncodingName(toEncoding))) {
        for (size_t i = 0; i < inputs.size(); ++i) {
            results.emplace_back(StringResult::Failure(ErrorCode::InvalidTargetEncoding));
        }
        return results;
    }
    
    // 预取inputs数据到缓存
    if (UNICONV_LIKELY(!inputs.empty())) {
        UNICONV_PREFETCH(inputs.data(), 0, 2);
    }
    
    // 预先获取iconv描述符以避免重复创建 - 预测通常能成功获取
    auto descriptor = GetIconvDescriptor(fromEncoding, toEncoding);
    if (UNICONV_UNLIKELY(!descriptor)) {
        // 为所有输入返回错误结果
        for (size_t i = 0; i < inputs.size(); ++i) {
            results.emplace_back(StringResult::Failure(ErrorCode::InvalidSourceEncoding));
        }
        return results;
    }
    
    // 批量处理每个输入
    for (const auto& input : inputs) {
        if (input.empty()) {
            results.emplace_back(StringResult::Success(std::string{}));
            continue;
        }
        
        // 估算输出大小
        size_t estimated = EstimateOutputSize(input.size(), fromEncoding, toEncoding);
        
        // 获取缓冲区 - 使用估算大小选择合适的缓冲区层级
        auto buffer_lease = m_stringBufferPool.acquire(estimated);
        if (!buffer_lease.valid()) {
            results.emplace_back(StringResult::Failure(ErrorCode::OutOfMemory));
            continue;
        }
        
        // 转换并添加结果
        results.emplace_back(ConvertEncodingInternal(input, fromEncoding, toEncoding, buffer_lease, estimated));
    }
    
    return results;
}

void UniConv::EvictLRUCacheEntries()
{
    // Lock-free eviction using phmap's concurrent operations
    if (m_iconvDescriptorCacheMap.empty()) {
        return;
    }
    
    constexpr size_t TARGET_SIZE = MAX_CACHE_SIZE * 3 / 4;  // 清理到75%容量
    
    // 收集所有条目及其访问时间和键（现在是uint64_t哈希值）
    std::vector<std::pair<uint64_t, uint64_t>> entries;  // (timestamp, hash_key)
    entries.reserve(m_iconvDescriptorCacheMap.size());
    
    // Thread-safe iteration using phmap
    m_iconvDescriptorCacheMap.for_each([&](const auto& item) {
        entries.emplace_back(item.second.last_used.load(std::memory_order_relaxed), item.first);
    });
    
    // 按访问时间排序（最旧的在前）
    std::sort(entries.begin(), entries.end());
    
    // 删除最旧的条目直到达到目标大小
    size_t to_remove = m_iconvDescriptorCacheMap.size() - TARGET_SIZE;
    for (size_t i = 0; i < to_remove && i < entries.size(); ++i) {
        if (m_iconvDescriptorCacheMap.erase(entries[i].second)) {
            m_cacheEvictionCount.fetch_add(1, std::memory_order_relaxed);
        }
    }
}

UniConv::PoolStats UniConv::GetPoolStatistics() const {
    PoolStats stats;
    stats.active_buffers = m_stringBufferPool.GetActiveBuffers();
    stats.total_conversions = m_totalConversions.load(std::memory_order_relaxed);
    stats.cache_hits = m_poolCacheHits.load(std::memory_order_relaxed);
    
    // 计算缓存命中率
    if (stats.total_conversions > 0) {
        stats.hit_rate = static_cast<double>(stats.cache_hits) / stats.total_conversions;
    } else {
        stats.hit_rate = 0.0;
    }
    
    // Lock-free iconv cache statistics using phmap
    stats.iconv_cache_size = m_iconvDescriptorCacheMap.size();
    stats.iconv_cache_hits = m_cacheHitCount.load(std::memory_order_relaxed);
    stats.iconv_cache_misses = m_cacheMissCount.load(std::memory_order_relaxed);
    stats.iconv_cache_evictions = m_cacheEvictionCount.load(std::memory_order_relaxed);
    
    // 计算iconv命中率
    const uint64_t total_iconv_requests = stats.iconv_cache_hits + stats.iconv_cache_misses;
    stats.iconv_cache_hit_rate = (total_iconv_requests > 0) ? 
        (static_cast<double>(stats.iconv_cache_hits) / total_iconv_requests) : 0.0;
    
    // 计算平均命中次数
    if (stats.iconv_cache_size > 0) {
        uint64_t total_hit_count = 0;
        for (const auto& [key, entry] : m_iconvDescriptorCacheMap) {
            total_hit_count += entry.hit_count.load(std::memory_order_relaxed);
        }
        stats.iconv_avg_hit_count = static_cast<double>(total_hit_count) / stats.iconv_cache_size;
    } else {
        stats.iconv_avg_hit_count = 0.0;
    }
    
    return stats;
}

//----------------------------------------------------------------------------------------------------------------------
// === Enhanced convenience methods with detailed error handling ===
//----------------------------------------------------------------------------------------------------------------------

CompactResult<std::string> UniConv::ToUtf8FromLocaleEx(const std::string& input) {
    if (input.empty()) {
        return CompactResult<std::string>::Success(std::string{});
    }
    
    std::string currentEncoding = GetCurrentSystemEncoding();
    return ConvertEncodingFast(input, currentEncoding.c_str(), ToString(Encoding::utf_8).c_str());
}

CompactResult<std::string> UniConv::ToLocaleFromUtf8Ex(const std::string& input) {
    if (input.empty()) {
        return CompactResult<std::string>::Success(std::string{});
    }
    
    std::string currentEncoding = GetCurrentSystemEncoding();
    return ConvertEncodingFast(input, ToString(Encoding::utf_8).c_str(), currentEncoding.c_str());
}

CompactResult<std::u16string> UniConv::ToUtf16LEFromLocaleEx(const std::string& input) {
    if (input.empty()) {
        return CompactResult<std::u16string>::Success(std::u16string{});
    }
    
    std::string currentEncoding = GetCurrentSystemEncoding();
    auto result = ConvertEncodingFast(input, currentEncoding.c_str(), ToString(Encoding::utf_16le).c_str());
    
    if (!result.IsSuccess()) {
        return CompactResult<std::u16string>::Failure(result.GetErrorCode());
    }
    
    // Convert std::string to std::u16string
    const std::string& str = result.GetValue();
    if (str.size() % 2 != 0) {
        return CompactResult<std::u16string>::Failure(ErrorCode::InvalidSequence);
    }
    
    std::u16string u16str;
    u16str.reserve(str.size() / 2);
    
    for (size_t i = 0; i < str.size(); i += 2) {
        char16_t ch = static_cast<char16_t>(static_cast<unsigned char>(str[i])) |
                      (static_cast<char16_t>(static_cast<unsigned char>(str[i + 1])) << 8);
        u16str.push_back(ch);
    }
    
    return CompactResult<std::u16string>::Success(std::move(u16str));
}

CompactResult<std::u16string> UniConv::ToUtf16BEFromLocaleEx(const std::string& input) {
    if (input.empty()) {
        return CompactResult<std::u16string>::Success(std::u16string{});
    }
    
    std::string currentEncoding = GetCurrentSystemEncoding();
    auto result = ConvertEncodingFast(input, currentEncoding.c_str(), ToString(Encoding::utf_16be).c_str());
    
    if (!result.IsSuccess()) {
        return CompactResult<std::u16string>::Failure(result.GetErrorCode());
    }
    
    // Convert std::string to std::u16string (big-endian)
    const std::string& str = result.GetValue();
    if (str.size() % 2 != 0) {
        return CompactResult<std::u16string>::Failure(ErrorCode::InvalidSequence);
    }
    
    std::u16string u16str;
    u16str.reserve(str.size() / 2);
    
    for (size_t i = 0; i < str.size(); i += 2) {
        char16_t ch = (static_cast<char16_t>(static_cast<unsigned char>(str[i])) << 8) |
                      static_cast<char16_t>(static_cast<unsigned char>(str[i + 1]));
        u16str.push_back(ch);
    }
    
    return CompactResult<std::u16string>::Success(std::move(u16str));
}

CompactResult<std::string> UniConv::ToUtf8FromUtf16LEEx(const std::u16string& input) {
    if (input.empty()) {
        return CompactResult<std::string>::Success(std::string{});
    }
    
    // Convert std::u16string to std::string for iconv processing
    std::string inputStr;
    inputStr.reserve(input.size() * 2);
    
    for (char16_t ch : input) {
        inputStr.push_back(static_cast<char>(ch & 0xFF));
        inputStr.push_back(static_cast<char>((ch >> 8) & 0xFF));
    }
    
    return ConvertEncodingFast(inputStr, ToString(Encoding::utf_16le).c_str(), ToString(Encoding::utf_8).c_str());
}

CompactResult<std::string> UniConv::ToUtf8FromUtf16BEEx(const std::u16string& input) {
    if (input.empty()) {
        return CompactResult<std::string>::Success(std::string{});
    }
    
    // Convert std::u16string to std::string for iconv processing (big-endian)
    std::string inputStr;
    inputStr.reserve(input.size() * 2);
    
    for (char16_t ch : input) {
        inputStr.push_back(static_cast<char>((ch >> 8) & 0xFF));
        inputStr.push_back(static_cast<char>(ch & 0xFF));
    }
    
    return ConvertEncodingFast(inputStr, ToString(Encoding::utf_16be).c_str(), ToString(Encoding::utf_8).c_str());
}

//----------------------------------------------------------------------------------------------------------------------
// === Zero-Copy Output Parameter API Implementation ===
//----------------------------------------------------------------------------------------------------------------------

bool UniConv::ConvertEncoding(const std::string& input, const char* fromEncoding, const char* toEncoding, std::string& output) noexcept {
    ErrorCode result = ConvertEncodingFast(input, fromEncoding, toEncoding, output);
    return result == ErrorCode::Success;
}

ErrorCode UniConv::ConvertEncodingFast(const std::string& input, const char* fromEncoding, const char* toEncoding, std::string& output) noexcept {
    // Clear output and reserve space
    output.clear();
    
    // Fast parameter check
    if (UNICONV_UNLIKELY(!fromEncoding || !toEncoding)) {
        return ErrorCode::InvalidParameter;
    }
    
    // 快速编码名称有效性检查
    if (UNICONV_UNLIKELY(!IsValidEncodingName(fromEncoding))) {
        return ErrorCode::InvalidSourceEncoding;
    }
    if (UNICONV_UNLIKELY(!IsValidEncodingName(toEncoding))) {
        return ErrorCode::InvalidTargetEncoding;
    }
    
    // Fast empty input check
    if (UNICONV_UNLIKELY(input.empty())) {
        return ErrorCode::Success;
    }
    

    
    // 同编码直接返回（零转换开销）
    if (UNICONV_LIKELY(CompareEncodingNamesEqual(fromEncoding, toEncoding))) {
        output = input;
        return ErrorCode::Success;
    }
    
    // 纯 ASCII 内容的快速处理
    if (IsAsciiCompatibleEncoding(fromEncoding) && IsAsciiCompatibleEncoding(toEncoding)) {
        if (UNICONV_LIKELY(IsAllAscii(input))) {
            output = input;
            return ErrorCode::Success;
        }
    }
    
    //==========================================================================
    // P1: simdutf SIMD 加速快速路径（可选）
    //==========================================================================
#ifdef UNICONV_HAS_SIMDUTF
    // UTF-8 → UTF-16LE
    if (IsUtf8ToUtf16LE(fromEncoding, toEncoding)) {
        auto result = ConvertUtf8ToUtf16LE_SIMD(input);
        if (result.IsSuccess()) {
            output = std::move(result).GetValue();
            return ErrorCode::Success;
        }
        return result.GetErrorCode();
    }
    // UTF-16LE → UTF-8
    if (IsUtf16LEToUtf8(fromEncoding, toEncoding)) {
        auto result = ConvertUtf16LEToUtf8_SIMD(input);
        if (result.IsSuccess()) {
            output = std::move(result).GetValue();
            return ErrorCode::Success;
        }
        return result.GetErrorCode();
    }
    // UTF-8 → UTF-16BE
    if (IsUtf8ToUtf16BE(fromEncoding, toEncoding)) {
        auto result = ConvertUtf8ToUtf16BE_SIMD(input);
        if (result.IsSuccess()) {
            output = std::move(result).GetValue();
            return ErrorCode::Success;
        }
        return result.GetErrorCode();
    }
    // UTF-16BE → UTF-8
    if (IsUtf16BEToUtf8(fromEncoding, toEncoding)) {
        auto result = ConvertUtf16BEToUtf8_SIMD(input);
        if (result.IsSuccess()) {
            output = std::move(result).GetValue();
            return ErrorCode::Success;
        }
        return result.GetErrorCode();
    }
#endif // UNICONV_HAS_SIMDUTF
    //==========================================================================
    
    // Prefetch input data to cache
    UNICONV_PREFETCH(input.data(), 0, 3);
    
    // 使用预计算哈希作为缓存键 - 避免字符串拼接分配
    const size_t from_len = strlen(fromEncoding);
    const size_t to_len   = strlen(toEncoding);
    const uint64_t key = detail::MakeEncodingPairKey(fromEncoding, from_len, toEncoding, to_len);
    
    // Try cache first (thread-safe in both modes)
#if UNICONV_NO_THREAD_LOCAL
    // DLL mode: lock the instance cache
    auto cache_lock = GetCacheLock();
#endif
    auto descriptor = GetCache().GetOrCreateIconvDescriptor(key, fromEncoding, toEncoding);
    if (UNICONV_UNLIKELY(!descriptor)) {
        // Fallback to global cache
        descriptor = GetIconvDescriptor(fromEncoding, toEncoding);
        if (UNICONV_UNLIKELY(!descriptor)) {
            return ErrorCode::ConversionFailed;
        }
    }
    
    // Cast from shared_ptr<void> to iconv_t (cross-platform safe)
    iconv_t cd = static_cast<iconv_t>(descriptor.get());
    
    // Input buffer setup
    const char* inbuf_ptr = input.data();
    std::size_t inbuf_left = input.size();
    
    // Estimate output size and reserve
    size_t estimated_size = EstimateOutputSize(input.size(), fromEncoding, toEncoding);
    try {
        output.reserve(estimated_size);
    } catch (...) {
        return ErrorCode::OutOfMemory;
    }
    
    // Use temp buffer for conversion
    std::size_t buffer_size = (std::max)(static_cast<std::size_t>(4096), estimated_size);
    std::vector<char>& temp_buffer = GetCache().temp_conversion_buffer;
    
    // Reuse thread-local temp buffer
    if (temp_buffer.size() < buffer_size) {
        try {
            temp_buffer.resize(buffer_size);
        } catch (...) {
            return ErrorCode::OutOfMemory;
        }
    }
    
    // Conversion loop
    constexpr int max_iterations = 100;
    int iteration_count = 0;
    
    while (UNICONV_LIKELY(inbuf_left > 0) && UNICONV_LIKELY(iteration_count++ < max_iterations)) {
        char* outbuf_ptr = temp_buffer.data();
        std::size_t outbuf_left = temp_buffer.size();
        
        // Prefetch temp buffer
        UNICONV_PREFETCH(temp_buffer.data(), 1, 2);
        
        // Execute conversion
        std::size_t ret = iconv(cd, &inbuf_ptr, &inbuf_left, &outbuf_ptr, &outbuf_left);
        
        // Append converted bytes
        std::size_t converted_bytes = temp_buffer.size() - outbuf_left;
        if (UNICONV_LIKELY(converted_bytes > 0)) {
            try {
                output.append(temp_buffer.data(), converted_bytes);
            } catch (...) {
                return ErrorCode::OutOfMemory;
            }
        }
        
        // Error handling
        if (UNICONV_UNLIKELY(static_cast<std::size_t>(-1) == ret)) {
            int current_errno = errno;
            switch (current_errno) {
                case E2BIG:
                    if (UNICONV_UNLIKELY(temp_buffer.size() >= 1048576 * 10)) { // 10MB limit
                        return ErrorCode::BufferTooSmall;
                    }
                    temp_buffer.resize(temp_buffer.size() * 2);
                    continue;
                case EILSEQ:
                    return ErrorCode::InvalidSequence;
                case EINVAL:
                    return ErrorCode::IncompleteSequence;
                default:
                    return ErrorCode::ConversionFailed;
            }
        } else {
            break;
        }
    }
    
    if (UNICONV_UNLIKELY(iteration_count >= max_iterations)) {
        return ErrorCode::InternalError;
    }
    
    return ErrorCode::Success;
}

// UTF-8 Conversion Series (output parameter versions)
bool UniConv::ToUtf8FromLocale(const std::string& input, std::string& output) noexcept {
    if (input.empty()) {
        output.clear();
        return true;
    }
    std::string currentEncoding = GetCurrentSystemEncoding();
    return ErrorCode::Success == ConvertEncodingFast(input, currentEncoding.c_str(), ToString(Encoding::utf_8).c_str(), output);
}

bool UniConv::ToLocaleFromUtf8(const std::string& input, std::string& output) noexcept {
    if (input.empty()) {
        output.clear();
        return true;
    }
    std::string currentEncoding = GetCurrentSystemEncoding();
    return ErrorCode::Success == ConvertEncodingFast(input, ToString(Encoding::utf_8).c_str(), currentEncoding.c_str(), output);
}

bool UniConv::ToUtf8FromUtf16LE(const std::u16string& input, std::string& output) noexcept {
    if (input.empty()) {
        output.clear();
        return true;
    }
    
    std::string input_bytes;
    input_bytes.reserve(input.size() * 2);
    for (char16_t ch : input) {
        input_bytes.push_back(static_cast<char>(ch & 0xFF));
        input_bytes.push_back(static_cast<char>((ch >> 8) & 0xFF));
    }
    
    return ErrorCode::Success == ConvertEncodingFast(input_bytes, ToString(Encoding::utf_16le).c_str(), ToString(Encoding::utf_8).c_str(), output);
}

bool UniConv::ToUtf8FromUtf16BE(const std::u16string& input, std::string& output) noexcept {
    if (input.empty()) {
        output.clear();
        return true;
    }
    
    std::string input_bytes;
    input_bytes.reserve(input.size() * 2);
    for (char16_t ch : input) {
        input_bytes.push_back(static_cast<char>((ch >> 8) & 0xFF));
        input_bytes.push_back(static_cast<char>(ch & 0xFF));
    }
    
    return ErrorCode::Success == ConvertEncodingFast(input_bytes, ToString(Encoding::utf_16be).c_str(), ToString(Encoding::utf_8).c_str(), output);
}

bool UniConv::ToUtf8FromUtf32LE(const std::u32string& input, std::string& output) noexcept {
    if (input.empty()) {
        output.clear();
        return true;
    }
    
    std::string input_bytes;
    input_bytes.reserve(input.size() * 4);
    for (char32_t ch : input) {
        input_bytes.push_back(static_cast<char>(ch & 0xFF));
        input_bytes.push_back(static_cast<char>((ch >> 8) & 0xFF));
        input_bytes.push_back(static_cast<char>((ch >> 16) & 0xFF));
        input_bytes.push_back(static_cast<char>((ch >> 24) & 0xFF));
    }
    
    return ErrorCode::Success == ConvertEncodingFast(input_bytes, ToString(Encoding::utf_32le).c_str(), ToString(Encoding::utf_8).c_str(), output);
}

// UTF-16 Conversion Series (output parameter versions)
bool UniConv::ToUtf16LEFromUtf8(const std::string& input, std::u16string& output) noexcept {
    if (input.empty()) {
        output.clear();
        return true;
    }
    
    std::string temp_output;
    ErrorCode result = ConvertEncodingFast(input, ToString(Encoding::utf_8).c_str(), ToString(Encoding::utf_16le).c_str(), temp_output);
    
    if (result == ErrorCode::Success && temp_output.size() % 2 == 0) {
        output.clear();
        output.reserve(temp_output.size() / 2);
        for (size_t i = 0; i < temp_output.size(); i += 2) {
            char16_t ch = static_cast<char16_t>(static_cast<unsigned char>(temp_output[i])) |
                          (static_cast<char16_t>(static_cast<unsigned char>(temp_output[i + 1])) << 8);
            output.push_back(ch);
        }
        return true;
    }
    return false;
}

bool UniConv::ToUtf16BEFromUtf8(const std::string& input, std::u16string& output) noexcept {
    if (input.empty()) {
        output.clear();
        return true;
    }
    
    std::string temp_output;
    ErrorCode result = ConvertEncodingFast(input, ToString(Encoding::utf_8).c_str(), ToString(Encoding::utf_16be).c_str(), temp_output);
    
    if (result == ErrorCode::Success && temp_output.size() % 2 == 0) {
        output.clear();
        output.reserve(temp_output.size() / 2);
        for (size_t i = 0; i < temp_output.size(); i += 2) {
            char16_t ch = (static_cast<char16_t>(static_cast<unsigned char>(temp_output[i])) << 8) |
                          static_cast<char16_t>(static_cast<unsigned char>(temp_output[i + 1]));
            output.push_back(ch);
        }
        return true;
    }
    return false;
}

bool UniConv::ToUtf16LEFromLocale(const std::string& input, std::u16string& output) noexcept {
    if (input.empty()) {
        output.clear();
        return true;
    }
    
    std::string currentEncoding = GetCurrentSystemEncoding();
    std::string temp_output;
    ErrorCode result = ConvertEncodingFast(input, currentEncoding.c_str(), ToString(Encoding::utf_16le).c_str(), temp_output);
    
    if (result == ErrorCode::Success && temp_output.size() % 2 == 0) {
        output.clear();
        output.reserve(temp_output.size() / 2);
        for (size_t i = 0; i < temp_output.size(); i += 2) {
            char16_t ch = static_cast<char16_t>(static_cast<unsigned char>(temp_output[i])) |
                          (static_cast<char16_t>(static_cast<unsigned char>(temp_output[i + 1])) << 8);
            output.push_back(ch);
        }
        return true;
    }
    return false;
}

bool UniConv::ToUtf16BEFromLocale(const std::string& input, std::u16string& output) noexcept {
    if (input.empty()) {
        output.clear();
        return true;
    }
    
    std::string currentEncoding = GetCurrentSystemEncoding();
    std::string temp_output;
    ErrorCode result = ConvertEncodingFast(input, currentEncoding.c_str(), ToString(Encoding::utf_16be).c_str(), temp_output);
    
    if (result == ErrorCode::Success && temp_output.size() % 2 == 0) {
        output.clear();
        output.reserve(temp_output.size() / 2);
        for (size_t i = 0; i < temp_output.size(); i += 2) {
            char16_t ch = (static_cast<char16_t>(static_cast<unsigned char>(temp_output[i])) << 8) |
                          static_cast<char16_t>(static_cast<unsigned char>(temp_output[i + 1]));
            output.push_back(ch);
        }
        return true;
    }
    return false;
}

bool UniConv::ToUtf16BEFromUtf16LE(const std::u16string& input, std::u16string& output) noexcept {
    if (input.empty()) {
        output.clear();
        return true;
    }
    
    std::string input_bytes;
    input_bytes.reserve(input.size() * 2);
    for (char16_t ch : input) {
        input_bytes.push_back(static_cast<char>(ch & 0xFF));
        input_bytes.push_back(static_cast<char>((ch >> 8) & 0xFF));
    }
    
    std::string temp_output;
    ErrorCode result = ConvertEncodingFast(input_bytes, ToString(Encoding::utf_16le).c_str(), ToString(Encoding::utf_16be).c_str(), temp_output);
    
    if (result == ErrorCode::Success && temp_output.size() % 2 == 0) {
        output.clear();
        output.reserve(temp_output.size() / 2);
        for (size_t i = 0; i < temp_output.size(); i += 2) {
            char16_t ch = (static_cast<char16_t>(static_cast<unsigned char>(temp_output[i])) << 8) |
                          static_cast<char16_t>(static_cast<unsigned char>(temp_output[i + 1]));
            output.push_back(ch);
        }
        return true;
    }
    return false;
}

bool UniConv::ToUtf16LEFromUtf16BE(const std::u16string& input, std::u16string& output) noexcept {
    if (input.empty()) {
        output.clear();
        return true;
    }
    
    std::string input_bytes;
    input_bytes.reserve(input.size() * 2);
    for (char16_t ch : input) {
        input_bytes.push_back(static_cast<char>((ch >> 8) & 0xFF));
        input_bytes.push_back(static_cast<char>(ch & 0xFF));
    }
    
    std::string temp_output;
    ErrorCode result = ConvertEncodingFast(input_bytes, ToString(Encoding::utf_16be).c_str(), ToString(Encoding::utf_16le).c_str(), temp_output);
    
    if (result == ErrorCode::Success && temp_output.size() % 2 == 0) {
        output.clear();
        output.reserve(temp_output.size() / 2);
        for (size_t i = 0; i < temp_output.size(); i += 2) {
            char16_t ch = static_cast<char16_t>(static_cast<unsigned char>(temp_output[i])) |
                          (static_cast<char16_t>(static_cast<unsigned char>(temp_output[i + 1])) << 8);
            output.push_back(ch);
        }
        return true;
    }
    return false;
}

// UTF-32 Conversion Series (output parameter versions)
bool UniConv::ToUtf32LEFromUtf8(const std::string& input, std::u32string& output) noexcept {
    if (input.empty()) {
        output.clear();
        return true;
    }
    
    std::string temp_output;
    ErrorCode result = ConvertEncodingFast(input, ToString(Encoding::utf_8).c_str(), ToString(Encoding::utf_32le).c_str(), temp_output);
    
    if (result == ErrorCode::Success && temp_output.size() % 4 == 0) {
        output.clear();
        output.reserve(temp_output.size() / 4);
        for (size_t i = 0; i < temp_output.size(); i += 4) {
            char32_t ch = static_cast<char32_t>(static_cast<unsigned char>(temp_output[i])) |
                          (static_cast<char32_t>(static_cast<unsigned char>(temp_output[i + 1])) << 8) |
                          (static_cast<char32_t>(static_cast<unsigned char>(temp_output[i + 2])) << 16) |
                          (static_cast<char32_t>(static_cast<unsigned char>(temp_output[i + 3])) << 24);
            output.push_back(ch);
        }
        return true;
    }
    return false;
}

bool UniConv::ToUtf32LEFromUtf16LE(const std::u16string& input, std::u32string& output) noexcept {
    if (input.empty()) {
        output.clear();
        return true;
    }
    
    std::string input_bytes;
    input_bytes.reserve(input.size() * 2);
    for (char16_t ch : input) {
        input_bytes.push_back(static_cast<char>(ch & 0xFF));
        input_bytes.push_back(static_cast<char>((ch >> 8) & 0xFF));
    }
    
    std::string temp_output;
    ErrorCode result = ConvertEncodingFast(input_bytes, ToString(Encoding::utf_16le).c_str(), ToString(Encoding::utf_32le).c_str(), temp_output);
    
    if (result == ErrorCode::Success && temp_output.size() % 4 == 0) {
        output.clear();
        output.reserve(temp_output.size() / 4);
        for (size_t i = 0; i < temp_output.size(); i += 4) {
            char32_t ch = static_cast<char32_t>(static_cast<unsigned char>(temp_output[i])) |
                          (static_cast<char32_t>(static_cast<unsigned char>(temp_output[i + 1])) << 8) |
                          (static_cast<char32_t>(static_cast<unsigned char>(temp_output[i + 2])) << 16) |
                          (static_cast<char32_t>(static_cast<unsigned char>(temp_output[i + 3])) << 24);
            output.push_back(ch);
        }
        return true;
    }
    return false;
}

bool UniConv::ToUtf32LEFromUtf16BE(const std::u16string& input, std::u32string& output) noexcept {
    if (input.empty()) {
        output.clear();
        return true;
    }
    
    std::string input_bytes;
    input_bytes.reserve(input.size() * 2);
    for (char16_t ch : input) {
        input_bytes.push_back(static_cast<char>((ch >> 8) & 0xFF));
        input_bytes.push_back(static_cast<char>(ch & 0xFF));
    }
    
    std::string temp_output;
    ErrorCode result = ConvertEncodingFast(input_bytes, ToString(Encoding::utf_16be).c_str(), ToString(Encoding::utf_32le).c_str(), temp_output);
    
    if (result == ErrorCode::Success && temp_output.size() % 4 == 0) {
        output.clear();
        output.reserve(temp_output.size() / 4);
        for (size_t i = 0; i < temp_output.size(); i += 4) {
            char32_t ch = static_cast<char32_t>(static_cast<unsigned char>(temp_output[i])) |
                          (static_cast<char32_t>(static_cast<unsigned char>(temp_output[i + 1])) << 8) |
                          (static_cast<char32_t>(static_cast<unsigned char>(temp_output[i + 2])) << 16) |
                          (static_cast<char32_t>(static_cast<unsigned char>(temp_output[i + 3])) << 24);
            output.push_back(ch);
        }
        return true;
    }
    return false;
}

// Locale Conversion Series (output parameter versions)
bool UniConv::ToLocaleFromUtf16LE(const std::u16string& input, std::string& output) noexcept {
    if (input.empty()) {
        output.clear();
        return true;
    }
    
    std::string currentEncoding = GetCurrentSystemEncoding();
    std::string input_bytes;
    input_bytes.reserve(input.size() * 2);
    for (char16_t ch : input) {
        input_bytes.push_back(static_cast<char>(ch & 0xFF));
        input_bytes.push_back(static_cast<char>((ch >> 8) & 0xFF));
    }
    
    return ErrorCode::Success == ConvertEncodingFast(input_bytes, ToString(Encoding::utf_16le).c_str(), currentEncoding.c_str(), output);
}

bool UniConv::ToLocaleFromUtf16BE(const std::u16string& input, std::string& output) noexcept {
    if (input.empty()) {
        output.clear();
        return true;
    }
    
    std::string currentEncoding = GetCurrentSystemEncoding();
    std::string input_bytes;
    input_bytes.reserve(input.size() * 2);
    for (char16_t ch : input) {
        input_bytes.push_back(static_cast<char>((ch >> 8) & 0xFF));
        input_bytes.push_back(static_cast<char>(ch & 0xFF));
    }
    
    return ErrorCode::Success == ConvertEncodingFast(input_bytes, ToString(Encoding::utf_16be).c_str(), currentEncoding.c_str(), output);
}

bool UniConv::ToLocaleFromWideString(const std::wstring& input, std::string& output) noexcept {
    if (input.empty()) {
        output.clear();
        return true;
    }
    
    std::string currentEncoding = GetCurrentSystemEncoding();
    std::string input_bytes(reinterpret_cast<const char*>(input.data()), input.size() * sizeof(wchar_t));
    
    return ErrorCode::Success == ConvertEncodingFast(input_bytes, ToString(Encoding::wchar_t_encoding).c_str(), currentEncoding.c_str(), output);
}

// Wide String Series (output parameter versions)
bool UniConv::ToWideStringFromLocale(const std::string& input, std::wstring& output) noexcept {
    if (input.empty()) {
        output.clear();
        return true;
    }
    
    std::string currentEncoding = GetCurrentSystemEncoding();
    std::string temp_output;
    ErrorCode result = ConvertEncodingFast(input, currentEncoding.c_str(), ToString(Encoding::utf_16le).c_str(), temp_output);
    
    if (result == ErrorCode::Success && temp_output.size() % sizeof(wchar_t) == 0) {
        // Remove BOM if present
        const char* data = temp_output.data();
        size_t size = temp_output.size();
        if (size >= 2 && (uint8_t)data[0] == 0xFF && (uint8_t)data[1] == 0xFE) {
            data += 2;
            size -= 2;
        }
        output = std::wstring(reinterpret_cast<const wchar_t*>(data), size / sizeof(wchar_t));
        return true;
    }
    return false;
}

bool UniConv::U16StringToWString(const std::u16string& input, std::wstring& output) noexcept {
    if (input.empty()) {
        output.clear();
        return true;
    }
    
    std::string input_bytes(reinterpret_cast<const char*>(input.data()), input.size() * sizeof(char16_t));
    std::string temp_output;
    ErrorCode result = ConvertEncodingFast(input_bytes, ToString(Encoding::utf_16le).c_str(), ToString(Encoding::wchar_t_encoding).c_str(), temp_output);
    
    if (result == ErrorCode::Success && temp_output.size() % sizeof(wchar_t) == 0) {
        output = std::wstring(reinterpret_cast<const wchar_t*>(temp_output.data()), temp_output.size() / sizeof(wchar_t));
        return true;
    }
    return false;
}

//----------------------------------------------------------------------------------------------------------------------
// === Batch Conversion with Output Parameter (Phase 2) ===
//----------------------------------------------------------------------------------------------------------------------

bool UniConv::ConvertEncodingBatch(
    const std::vector<std::string>& inputs,
    const char* fromEncoding,
    const char* toEncoding,
    std::vector<std::string>& outputs) noexcept {
    
    // Parameter validation
    if (UNICONV_UNLIKELY(!fromEncoding || !toEncoding)) {
        outputs.clear();
        return false;
    }
    
    // 快速编码名称有效性检查
    if (UNICONV_UNLIKELY(!IsValidEncodingName(fromEncoding) || !IsValidEncodingName(toEncoding))) {
        outputs.clear();
        return false;
    }
    
    // Resize outputs to match inputs
    try {
        outputs.resize(inputs.size());
    } catch (...) {
        return false;
    }
    
    // Early return for empty inputs
    if (inputs.empty()) {
        return true;
    }
    
    // Prefetch inputs data
    if (UNICONV_LIKELY(!inputs.empty())) {
        UNICONV_PREFETCH(inputs.data(), 0, 2);
    }
    
    // 使用预计算哈希作为缓存键 - 批量转换只计算一次
    const size_t from_len = strlen(fromEncoding);
    const size_t to_len   = strlen(toEncoding);
    const uint64_t key = detail::MakeEncodingPairKey(fromEncoding, from_len, toEncoding, to_len);
    
    // Try cache first (thread-safe in both modes)
#if UNICONV_NO_THREAD_LOCAL
    // DLL mode: lock the instance cache
    auto cache_lock = GetCacheLock();
#endif
    auto descriptor = GetCache().GetOrCreateIconvDescriptor(key, fromEncoding, toEncoding);
    if (UNICONV_UNLIKELY(!descriptor)) {
        // Fallback to global cache
        descriptor = GetIconvDescriptor(fromEncoding, toEncoding);
        if (UNICONV_UNLIKELY(!descriptor)) {
            outputs.clear();
            return false;
        }
    }
    
    bool all_success = true;
    
    // Process each input
    for (size_t i = 0; i < inputs.size(); ++i) {
        const std::string& input = inputs[i];
        std::string& output = outputs[i];
        
        // Clear output for this conversion
        output.clear();
        
        // Handle empty input
        if (input.empty()) {
            continue;
        }
        
        // Estimate and reserve output size
        size_t estimated_size = EstimateOutputSize(input.size(), fromEncoding, toEncoding);
        try {
            output.reserve(estimated_size);
        } catch (...) {
            all_success = false;
            continue;
        }
        
        // Setup conversion parameters
        const char* inbuf_ptr = input.data();
        std::size_t inbuf_left = input.size();
        
        // Use temp buffer (protected by cache lock in DLL mode)
        std::vector<char>& temp_buffer = GetCache().temp_conversion_buffer;
        std::size_t buffer_size = (std::max)(static_cast<std::size_t>(4096), estimated_size);
        
        if (temp_buffer.size() < buffer_size) {
            try {
                temp_buffer.resize(buffer_size);
            } catch (...) {
                all_success = false;
                continue;
            }
        }
        
        // Conversion loop
        constexpr int max_iterations = 100;
        int iteration_count = 0;
        bool conversion_success = true;
        
        while (inbuf_left > 0 && iteration_count++ < max_iterations) {
            char* outbuf_ptr = temp_buffer.data();
            std::size_t outbuf_left = temp_buffer.size();
            
            std::size_t ret = iconv(descriptor.get(), &inbuf_ptr, &inbuf_left, &outbuf_ptr, &outbuf_left);
            
            std::size_t converted_bytes = temp_buffer.size() - outbuf_left;
            if (converted_bytes > 0) {
                try {
                    output.append(temp_buffer.data(), converted_bytes);
                } catch (...) {
                    conversion_success = false;
                    break;
                }
            }
            
            if (static_cast<std::size_t>(-1) == ret) {
                int current_errno = errno;
                switch (current_errno) {
                    case E2BIG:
                        if (temp_buffer.size() >= 1048576 * 10) {
                            conversion_success = false;
                            break;
                        }
                        temp_buffer.resize(temp_buffer.size() * 2);
                        continue;
                    case EILSEQ:
                    case EINVAL:
                    default:
                        conversion_success = false;
                        break;
                }
                if (!conversion_success) break;
            } else {
                break;
            }
        }
        
        if (!conversion_success || iteration_count >= max_iterations) {
            all_success = false;
            output.clear();  // Clear failed output
        }
    }
    
    return all_success;
}



std::vector<StringResult> UniConv::ConvertEncodingBatchParallel(
    const std::vector<std::string>& inputs,
    const char* fromEncoding,
    const char* toEncoding,
    size_t numThreads) noexcept {
    
    std::vector<StringResult> results;
    results.reserve(inputs.size());
    
    // Initialize with empty success results
    for (size_t i = 0; i < inputs.size(); ++i) {
        results.emplace_back(StringResult::Success(std::string{}));
    }
    
    // Parameter validation
    if (UNICONV_UNLIKELY(!fromEncoding || !toEncoding)) {
        for (size_t i = 0; i < inputs.size(); ++i) {
            results[i] = StringResult::Failure(ErrorCode::InvalidParameter);
        }
        return results;
    }
    
    // 快速编码名称有效性检查
    if (UNICONV_UNLIKELY(!IsValidEncodingName(fromEncoding))) {
        for (size_t i = 0; i < inputs.size(); ++i) {
            results[i] = StringResult::Failure(ErrorCode::InvalidSourceEncoding);
        }
        return results;
    }
    if (UNICONV_UNLIKELY(!IsValidEncodingName(toEncoding))) {
        for (size_t i = 0; i < inputs.size(); ++i) {
            results[i] = StringResult::Failure(ErrorCode::InvalidTargetEncoding);
        }
        return results;
    }
    
    // Calculate total bytes for adaptive strategy
    size_t total_bytes = 0;
    for (const auto& input : inputs) {
        total_bytes += input.size();
    }
    
    // Use adaptive parallel policy to determine strategy
    ThreadPool& pool = UniConvThreadPool::GetInstance();
    size_t max_threads = (numThreads > 0) ? numThreads : pool.GetThreadCount();
    size_t recommended_threads = AdaptiveParallelPolicy::GetRecommendedThreads(
        inputs.size(), total_bytes, max_threads);
    
    // Use serial processing if recommended
    if (recommended_threads == 0) {
        return ConvertEncodingBatch(inputs, fromEncoding, toEncoding);
    }
    
    // Use thread pool for parallel processing
    auto* self = this;  // Capture for lambda
    
    pool.ParallelFor(inputs.size(), 
        [self, &inputs, &results, fromEncoding, toEncoding](size_t start, size_t end) {
            for (size_t i = start; i < end; ++i) {
                const auto& input = inputs[i];
                
                if (input.empty()) {
                    results[i] = StringResult::Success(std::string{});
                    continue;
                }
                
                // Convert using ConvertEncodingFast
                results[i] = self->ConvertEncodingFast(input, fromEncoding, toEncoding);
            }
        }, 
        1  // min_chunk_size
    );
    
    return results;
}

bool UniConv::ConvertEncodingBatchParallel(
    const std::vector<std::string>& inputs,
    const char* fromEncoding,
    const char* toEncoding,
    std::vector<std::string>& outputs,
    size_t numThreads) noexcept {
    
    outputs.clear();
    outputs.resize(inputs.size());
    
    // Parameter validation
    if (UNICONV_UNLIKELY(!fromEncoding || !toEncoding)) {
        return false;
    }
    
    // 快速编码名称有效性检查
    if (UNICONV_UNLIKELY(!IsValidEncodingName(fromEncoding) || !IsValidEncodingName(toEncoding))) {
        return false;
    }
    
    // Calculate total bytes for adaptive strategy
    size_t total_bytes = 0;
    for (const auto& input : inputs) {
        total_bytes += input.size();
    }
    
    // Use adaptive parallel policy to determine strategy
    ThreadPool& pool = UniConvThreadPool::GetInstance();
    size_t max_threads = (numThreads > 0) ? numThreads : pool.GetThreadCount();
    size_t recommended_threads = AdaptiveParallelPolicy::GetRecommendedThreads(
        inputs.size(), total_bytes, max_threads);
    
    // Use serial processing if recommended
    if (recommended_threads == 0) {
        return ConvertEncodingBatch(inputs, fromEncoding, toEncoding, outputs);
    }
    
    // Track success across threads
    std::atomic<bool> all_success{true};
    auto* self = this;  // Capture for lambda
    
    // Use thread pool for parallel processing
    pool.ParallelFor(inputs.size(),
        [self, &inputs, &outputs, &all_success, fromEncoding, toEncoding](size_t start, size_t end) {
            bool chunk_success = true;
            
            for (size_t i = start; i < end; ++i) {
                const auto& input = inputs[i];
                
                if (input.empty()) {
                    outputs[i].clear();
                    continue;
                }
                
                // Use ConvertEncodingFast output parameter version
                bool success = (self->ConvertEncodingFast(input, fromEncoding, toEncoding, outputs[i]) == ErrorCode::Success);
                if (!success) {
                    chunk_success = false;
                }
            }
            
            if (!chunk_success) {
                all_success.store(false, std::memory_order_relaxed);
            }
        },
        1  // min_chunk_size
    );
    
    return all_success.load(std::memory_order_relaxed);
}

// ===================================================================================================================
// string_view Input Overloads 
// ===================================================================================================================

bool UniConv::ConvertEncoding(std::string_view input, const char* fromEncoding, const char* toEncoding, std::string& output) noexcept {
    // Wrap string_view as std::string for now (could be optimized to avoid copy in future)
    std::string temp_input(input);
    return ConvertEncoding(temp_input, fromEncoding, toEncoding, output);
}

ErrorCode UniConv::ConvertEncodingFast(std::string_view input, const char* fromEncoding, const char* toEncoding, std::string& output) noexcept {
    std::string temp_input(input);
    return ConvertEncodingFast(temp_input, fromEncoding, toEncoding, output);
}

bool UniConv::ToUtf8FromLocale(std::string_view input, std::string& output) noexcept {
    std::string temp_input(input);
    return ToUtf8FromLocale(temp_input, output);
}

bool UniConv::ToLocaleFromUtf8(std::string_view input, std::string& output) noexcept {
    std::string temp_input(input);
    return ToLocaleFromUtf8(temp_input, output);
}

bool UniConv::ToUtf16LEFromUtf8(std::string_view input, std::u16string& output) noexcept {
    std::string temp_input(input);
    return ToUtf16LEFromUtf8(temp_input, output);
}

bool UniConv::ToUtf16BEFromUtf8(std::string_view input, std::u16string& output) noexcept {
    std::string temp_input(input);
    return ToUtf16BEFromUtf8(temp_input, output);
}

