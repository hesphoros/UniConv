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
*  @file     UniConv.cpp
*  @brief    UniConv impl
*  @details  None
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
*  Change History :
*  <Date>     | <Version> | <Author>       | <Description>
*  2025/9/18  | 1.0.0.2   | hesphoros      | Add ConvertEncoding API
*****************************************************************************/

#include "UniConv.h"
#include <algorithm>





const std::string UniConv::m_encodingNames[] = {
    #define X(name, str) str,
    #include "encodings.inc"
    #undef X
};


const std::unordered_map<int, std::string_view> UniConv::m_iconvErrorMap = {
	{EILSEQ, "Invalid multibyte sequence"},
	{EINVAL, "Incomplete multibyte sequence"},
	{E2BIG,  "Output buffer too small"},
	{EBADF,  "Invalid conversion descriptor"},
	{EFAULT, "Invalid buffer address"},
	{EINTR,  "Conversion interrupted by signal"},
	{ENOMEM, "Out of memory"}
};

std::string UniConv::m_defaultEncoding = {}; // Default encoding, can be set by user

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
    this->m_defaultEncoding = encoding;
}


// ===================== System encoding related functions =====================
std::string UniConv::GetCurrentSystemEncoding() noexcept
{
    if (!m_defaultEncoding.empty()) {
        return m_defaultEncoding;
    }
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
	return ss.str();
}

// ===================== General convert functions =====================
UniConv::IConvResult UniConv::ConvertEncoding(const std::string& input, const char* fromEncoding, const char* toEncoding) {
    // Convert result
    IConvResult iconv_result;

    if (!fromEncoding || !toEncoding) {
        iconv_result.error_code = EINVAL;
        iconv_result.error_msg = "Invalid encoding parameters";
        return iconv_result;
    }

    if (input.empty())
    {
        iconv_result.error_code = 0;
        iconv_result.error_msg = "Input is empty";
        iconv_result.conv_result_str = "";
        return iconv_result;
    }

	// Get iconv convert descriptors
	auto cd = GetIconvDescriptor(fromEncoding, toEncoding);

    // check the iconv_t descriptors
	if (!cd || (cd.get() == reinterpret_cast<iconv_t>(-1))) {
		iconv_result.error_code = errno;
		iconv_result.error_msg = GetIconvErrorString(iconv_result.error_code);
		return iconv_result;
	}

    // Input
	const char* inbuf_ptr = input.data();  // input ptr
	std::size_t inbuf_left = input.size(); // Remaining length of the input cache

	// Out put
    std::size_t initial_buffer_size = (std::max)(static_cast<std::size_t>(4096), input.size() * 2);

	std::vector<char> out_buffer(initial_buffer_size);
	std::string converted_result;  // Convert result string

    // Pre allocation
    std::size_t estimated_size = input.size() * 4;
    if (estimated_size > 1048576) estimated_size = 1048576; // 限制最大预分配
    converted_result.reserve(estimated_size);

    // The maximum number of cycles prevents dead loops
    constexpr int max_iterations = 100;
    int iteration_count  = 0;

	while ( inbuf_left > 0 && iteration_count++ < max_iterations ) {
		char*       out_ptr  = out_buffer.data();
		std::size_t out_left = out_buffer.size();

		// Perform the conversion
		std::size_t ret = iconv(cd.get(), &inbuf_ptr, &inbuf_left, &out_ptr, &out_left);

        // Write the converted data
        std::size_t converted_bytes = out_buffer.size() - out_left;

        if (converted_bytes > 0) {
            converted_result.append(out_buffer.data(), converted_bytes);
        }

        // Step3. Error handling
        if ( static_cast<std::size_t>(-1) == ret ) {
            int current_errno = errno;
            /**
             *  For the E2BIG error,
             * * continue processing instead of returning the error immediately
             */
            if ( current_errno == E2BIG ) {
                // The out_buffer is too small and needs to be expanded
                if ( out_buffer.size() >= 1048576*10 ) // MAX for 10MB limit
                {
                    iconv_result.error_code = E2BIG;
                    iconv_result.error_msg = "Output buffer size limit exceeded";
                    break;
                }
                // 普通 E2BIG 扩容场景
                iconv_result.error_code = 0;
                iconv_result.error_msg.clear();
                out_buffer.resize(out_buffer.size() *2);
                continue;
            } else {
                // Other error
                iconv_result.error_code = current_errno;
                std::cerr << "iconv failed, errno = " << current_errno << " (" << strerror(current_errno) << ")\n";
                
                iconv_result.error_msg = GetIconvErrorString(current_errno);
                break;
            }
        }

        // If output buffer is getting full, expand it
        if ( out_left < 128 && inbuf_left > 0 ) {
            if (out_buffer.size() >= 1048576*10) { // 最大10MB限制
                // 如果还有输入未处理但缓冲区已达上限，这是异常情况
                iconv_result.error_code = E2BIG;
                iconv_result.error_msg = "Output buffer size limit exceeded";
                break;
            }
            out_buffer.resize(out_buffer.size() * 2);
            continue;
        }


	}

    //  Check whether the input has been processed
    if ( inbuf_left == 0 && iconv_result.error_code == 0 ) {
        // Refresh the internal state of the iconv descriptor
        char* out_ptr = out_buffer.data();
        std::size_t out_left = out_buffer.size();

        std::size_t ret = iconv(cd.get(), nullptr, nullptr, &out_ptr, &out_left);
        std::size_t final_converted = out_buffer.size() - out_left;

        if (final_converted > 0) {
            converted_result.append(out_buffer.data(), final_converted);
        }

        if (static_cast<std::size_t>(-1) == ret) {
            iconv_result.error_code = errno;
            iconv_result.error_msg = GetIconvErrorString(iconv_result.error_code);
        }

    }


        // 10. 检查是否因为循环次数限制而退出
    if ( iteration_count >= max_iterations && inbuf_left > 0 ) {
        iconv_result.error_code = ELOOP;
        iconv_result.error_msg = "Maximum conversion iterations exceeded";
        #if defined(UNICONV_DEBUG_MODE) && UNICONV_DEBUG_MODE
        std::cerr << "Too many cycles in thr iconv convert" << std::endl;
        #endif //UNICONV_DEBUG_MODE
    }

	// 返回转换结果
	if (iconv_result.error_code == 0) {
		converted_result.shrink_to_fit();
        iconv_result.conv_result_str = std::move(converted_result);
	}

	return iconv_result;
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
	auto it = m_encodingToCodePage.find(encoding);

	if (it != m_encodingToCodePage.end())
        return it->second;
    else
    {
        // If the encoding is not found, log a warning and return a default code page
        std::cerr << "Warning: Encoding '" << encoding << "' not found in mapping table. Defaulting to 0." << std::endl;
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
    auto result = ConvertEncoding(input, currentEncoding.c_str(), ToString(Encoding::utf_8).c_str());
    return result.IsSuccess() ? result.conv_result_str : "";
}

std::string UniConv::ToUtf8FromLocale(const char* input) {
    if (!input) return "";
    return ToUtf8FromLocale(std::string(input));
}

// UTF-8 -> System local encoding
std::string UniConv::ToLocaleFromUtf8(const std::string& input) {
    std::string currentEncoding = GetCurrentSystemEncoding();
    auto result = ConvertEncoding(input, ToString(Encoding::utf_8).c_str(), currentEncoding.c_str());
    return result.IsSuccess() ? result.conv_result_str : "";
}

std::string UniConv::ToLocaleFromUtf8(const char* input) {
    if (!input) return "";
    return ToLocaleFromUtf8(std::string(input));
}

// System local encoding -> UTF-16LE
std::u16string UniConv::ToUtf16LEFromLocale(const std::string& input) {
    std::string currentEncoding = GetCurrentSystemEncoding();
    auto result = ConvertEncoding(input, currentEncoding.c_str(), UniConv::ToString(UniConv::Encoding::utf_16le).c_str());
    if (result.IsSuccess() && result.conv_result_str.size() % 2 == 0) {
        return std::u16string(reinterpret_cast<const char16_t*>(result.conv_result_str.data()), 
                             result.conv_result_str.size() / 2);
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
    auto result = ConvertEncoding(input, currentEncoding.c_str(), ToString(Encoding::utf_16be).c_str());
    if (result.IsSuccess() && result.conv_result_str.size() % 2 == 0) {
        return std::u16string(reinterpret_cast<const char16_t*>(result.conv_result_str.data()), 
                             result.conv_result_str.size() / 2);
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
    auto result = ConvertEncoding(input_bytes, ToString(UniConv::Encoding::utf_16be).c_str(), currentEncoding.c_str());
    return result.IsSuccess() ? result.conv_result_str : "";
}

std::string UniConv::ToLocaleFromUtf16BE(const char16_t* input) {
    if (!input) return "";
    return ToLocaleFromUtf16BE(std::u16string(input));
}

// UTF-16LE -> UTF-8
std::string UniConv::ToUtf8FromUtf16LE(const std::u16string& input) {
    if (input.empty()) return "";

    std::string input_bytes(reinterpret_cast<const char*>(input.data()), input.size() * sizeof(char16_t));
    auto result = ConvertEncoding(input_bytes, ToString(Encoding::utf_16le).c_str(), ToString(Encoding::utf_8).c_str());
    return result.IsSuccess() ? result.conv_result_str : "";
}

// ===================== UTF-16LE with length parameter overloads =====================
std::string UniConv::ToUtf8FromUtf16LE(const char16_t* input, size_t len) {
    if (!input || len == 0) return "";

    std::string input_bytes(reinterpret_cast<const char*>(input), len * sizeof(char16_t));
    auto result = ConvertEncoding(input_bytes, ToString(Encoding::utf_16le).c_str(), ToString(Encoding::utf_8).c_str());
    return result.IsSuccess() ? result.conv_result_str : "";
}

std::string UniConv::ToUtf8FromUtf16LE(const char16_t* input) {
    if (!input) return "";
    return ToUtf8FromUtf16LE(std::u16string(input));
}

// UTF-16BE -> UTF-8
std::string UniConv::ToUtf8FromUtf16BE(const std::u16string& input) {
    if (input.empty()) return "";
    
    std::string input_bytes(reinterpret_cast<const char*>(input.data()), input.size() * sizeof(char16_t));
    auto result = ConvertEncoding(input_bytes, ToString(Encoding::utf_16be).c_str(),ToString(Encoding::utf_8).c_str());
    return result.IsSuccess() ? result.conv_result_str : "";
}

std::string UniConv::ToUtf8FromUtf16BE(const char16_t* input, size_t len) {
    if (!input || len == 0) return "";

    std::string input_bytes(reinterpret_cast<const char*>(input), len * sizeof(char16_t));
    auto result = ConvertEncoding(input_bytes, ToString(Encoding::utf_16be).c_str(), ToString(Encoding::utf_8).c_str());
    return result.IsSuccess() ? result.conv_result_str : "";
}

std::string UniConv::ToUtf8FromUtf16BE(const char16_t* input) {
    if (!input) return "";
    return ToUtf8FromUtf16BE(std::u16string(input));
}

// UTF-8 -> UTF-16LE
std::u16string UniConv::ToUtf16LEFromUtf8(const std::string& input) {
    auto result = ConvertEncoding(input, ToString(Encoding::utf_8).c_str(), ToString(Encoding::utf_16le).c_str());
    if (result.IsSuccess() && result.conv_result_str.size() % 2 == 0) {
        return std::u16string(reinterpret_cast<const char16_t*>(result.conv_result_str.data()), 
                             result.conv_result_str.size() / 2);
    }
    return std::u16string();
}

std::u16string UniConv::ToUtf16LEFromUtf8(const char* input) {
    if (!input) return std::u16string();
    return ToUtf16LEFromUtf8(std::string(input));
}

// UTF-8 -> UTF-16BE
std::u16string UniConv::ToUtf16BEFromUtf8(const std::string& input) {
    auto result = ConvertEncoding(input, ToString(Encoding::utf_8).c_str(), ToString(Encoding::utf_16be).c_str());
    if (result.IsSuccess() && result.conv_result_str.size() % 2 == 0) {
        return std::u16string(reinterpret_cast<const char16_t*>(result.conv_result_str.data()), 
                             result.conv_result_str.size() / 2);
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
    auto result = ConvertEncoding(input_bytes, ToString(Encoding::utf_16le).c_str(), ToString(Encoding::utf_16be).c_str());
    if (result.IsSuccess() && result.conv_result_str.size() % 2 == 0) {
        return std::u16string(reinterpret_cast<const char16_t*>(result.conv_result_str.data()), 
                             result.conv_result_str.size() / 2);
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
    auto result = ConvertEncoding(input_bytes, ToString(Encoding::utf_16be).c_str(), ToString(Encoding::utf_16le).c_str());
    if (result.IsSuccess() && result.conv_result_str.size() % 2 == 0) {
        return std::u16string(reinterpret_cast<const char16_t*>(result.conv_result_str.data()), 
                             result.conv_result_str.size() / 2);
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
    auto result = this->ConvertEncoding(input, currentEncoding.c_str(), ToString(Encoding::utf_16le).c_str());
    if (!result.IsSuccess()) return std::wstring{};
    // Remove the BOM (if any)
    const char* data = result.conv_result_str.data();
    size_t size = result.conv_result_str.size();
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
	auto result = this->ConvertEncoding
           (std::string(reinterpret_cast<const char*>(input.data()), input.size() * sizeof(wchar_t)),
               ToString(Encoding::wchar_t_encoding).c_str(),
               currentEncoding.c_str());
	return result.IsSuccess() ? result.conv_result_str : std::string{};
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
    auto result = this->ConvertEncoding(input_bytes, ToString(Encoding::utf_16le).c_str(), currentEncoding.c_str());
    return result.IsSuccess() ? result.conv_result_str : "";
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
    // Linux implementation
    std::wstring_convert<std::codecvt_byname<wchar_t, char, std::mbstate_t>> converter(new std::codecvt_byname<wchar_t, char, std::mbstate_t>(""));
    return converter.to_bytes(sInput);
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
	auto result = this->ConvertEncoding(input_bytes, ToString(Encoding::utf_32le).c_str(), ToString(Encoding::utf_8).c_str());
	return result.IsSuccess() ? result.conv_result_str : std::string{};
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
	auto result = this->ConvertEncoding(input_bytes, ToString(Encoding::utf_32le).c_str(), ToString(Encoding::utf_16le).c_str());
    return result.IsSuccess() && result.conv_result_str.size() % 2 == 0 ? 
           std::u16string(reinterpret_cast<const char16_t*>(result.conv_result_str.data()), 
			   result.conv_result_str.size() / sizeof(char16_t)) : std::u16string{};
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
    auto result = this->ConvertEncoding(input_bytes, ToString(Encoding::utf_32le).c_str(), ToString(Encoding::utf_16be).c_str());
    return result.IsSuccess() && result.conv_result_str.size() % 2 == 0 ? 
           std::u16string(reinterpret_cast<const char16_t*>(result.conv_result_str.data()), 
			   result.conv_result_str.size() / sizeof(char16_t)) : std::u16string{};
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
	auto result = this->ConvertEncoding(input, ToString(Encoding::utf_8).c_str(), ToString(Encoding::utf_32le).c_str());
    if (result.IsSuccess() && result.conv_result_str.size() % 4 == 0) {
        return std::u32string(reinterpret_cast<const char32_t*>(result.conv_result_str.data()), 
                             result.conv_result_str.size() / sizeof(char32_t));
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
    auto result = this->ConvertEncoding(input_bytes, ToString(Encoding::utf_16le).c_str(), ToString(Encoding::utf_32le).c_str());
    if (result.IsSuccess() && result.conv_result_str.size() % 4 == 0) {
        return std::u32string(reinterpret_cast<const char32_t*>(result.conv_result_str.data()), 
                             result.conv_result_str.size() / sizeof(char32_t));
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
    auto result = this->ConvertEncoding(input_bytes, ToString(Encoding::utf_16be).c_str(), ToString(Encoding::utf_32le).c_str());
    if (result.IsSuccess() && result.conv_result_str.size() % 4 == 0) {
        return std::u32string(reinterpret_cast<const char32_t*>(result.conv_result_str.data()), 
                             result.conv_result_str.size() / sizeof(char32_t));
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

    #else defined(__linux__) || defined(__APPLE__)
        if (input.empty()) return std::string();
        std::string utf8_str = reinterpret_cast<const char*>(input.data());
        auto result = ConvertEncoding(utf8_str, ToString(Encoding::wchar_t_encoding).c_str(), ToString(Encoding::utf_8).c_str());
        return result.IsSuccess() ? result.conv_result_str : std::string{};
    #endif
}

std::wstring UniConv::ToUcs4FromUtf8(const std::string& input)
{
    if (input.empty()) return std::wstring{};
    // Convert UTF-8 to UCS-4 (wide string)
    auto result = ConvertEncoding(input, ToString(Encoding::utf_8).c_str(), ToString(Encoding::utf_16le).c_str());
    if (result.IsSuccess() && result.conv_result_str.size() % sizeof(wchar_t) == 0) {
        return std::wstring(reinterpret_cast<const wchar_t*>(result.conv_result_str.data()),
            result.conv_result_str.size() / sizeof(wchar_t));
    }
    return std::wstring{};
}



std::wstring UniConv::U16StringToWString(const std::u16string& u16str)
{
    if (u16str.empty()) return std::wstring{};
    auto result = ConvertEncoding(std::string(reinterpret_cast<const char*>(u16str.data()), u16str.size() * sizeof(char16_t)),
        ToString(Encoding::utf_16le).c_str(), ToString(Encoding::wchar_t_encoding).c_str());
    if (result.IsSuccess() && result.conv_result_str.size() % sizeof(wchar_t) == 0) {
        return std::wstring(reinterpret_cast<const wchar_t*>(result.conv_result_str.data()),
            result.conv_result_str.size() / sizeof(wchar_t));
    }
    return std::wstring{};
}

std::wstring UniConv::U16StringToWString(const char16_t* u16str)
{
    if (!u16str) return std::wstring{};
    return U16StringToWString(std::u16string(u16str));
}


// ===================== Error Handling Related =====================
std::string UniConv::GetIconvErrorString(int err_code) {
    auto it = m_iconvErrorMap.find(err_code);
	if (it != m_iconvErrorMap.end()) {
		return std::string(it->second);
	}
    // If the error code is not found in the map, return a generic error message
	return std::string(std::generic_category().message(err_code));
}


UniConv::IconvSharedPtr UniConv::GetIconvDescriptor(const char* fromcode, const char* tocode)
{
	if(!fromcode || !tocode)
        return nullptr;

    std::string key;
    key.reserve((strlen(fromcode) + strlen(tocode) + 1));
    key = fromcode, key += ">", key += tocode;

    // Read lock (only find the cached descriptor)
    {
        std::shared_lock<std::shared_mutex> read_lock(m_iconvCacheMutex);
        auto it = m_iconvDescriptorCacheMap.find(key);
        if (it != m_iconvDescriptorCacheMap.end()) {
            return it->second; // return cached descriptor
        }
    }

    // Write lock (create a new descriptor if not found)
    std::unique_lock<std::shared_mutex> write_lock(m_iconvCacheMutex);

	auto it = m_iconvDescriptorCacheMap.find(key);
	if (it != m_iconvDescriptorCacheMap.end()) {
		return it->second; // return cached descriptor
	}

    // Create a new iconv descriptor
    // Use iconv_open to create a new conversion descriptor
	iconv_t cd = iconv_open(tocode, fromcode);
	if (cd == reinterpret_cast<iconv_t>(-1)) {
        #if defined(UNICONV_DEBUG_MODE) && UNICONV_DEBUG_MODE
		std::cout << "iconv_open error" << std::endl;
        #endif
		return nullptr;
	}
    // Create a shared pointer to manage the iconv descriptor
    // Use a custom deleter to close the iconv descriptor when the shared pointer is destroyed
	auto iconvPtr = std::shared_ptr<std::remove_pointer_t<iconv_t>>(cd, IconvDeleter());
	
	// 缓存大小管理
	if (m_iconvDescriptorCacheMap.size() >= MAX_CACHE_SIZE) {
		// 简单的清理策略：清理一半缓存
		auto half_point = std::next(m_iconvDescriptorCacheMap.begin(), MAX_CACHE_SIZE / 2);
		m_iconvDescriptorCacheMap.erase(m_iconvDescriptorCacheMap.begin(), half_point);
	}
	
	m_iconvDescriptorCacheMap.emplace(key, iconvPtr);

    #if defined(UNICONV_DEBUG_MODE) && UNICONV_DEBUG_MODE
        std::cout << "Create and cached iconv descriptor: " << key << std::endl;
    #endif

    return iconvPtr;
}

std::pair<UniConv::BomEncoding, std::string_view> UniConv::DetectAndRemoveBom(const std::string_view& data)
{
    const unsigned char* bytes = reinterpret_cast<const unsigned char*>(data.data());
    size_t len = data.size();

    if (len >= 3 && bytes[0] == 0xEF && bytes[1] == 0xBB && bytes[2] == 0xBF)
        return { BomEncoding::UTF8, data.substr(3) };
    if (len >= 2 && bytes[0] == 0xFF && bytes[1] == 0xFE)
        return { BomEncoding::UTF16_LE, data.substr(2) };
    if (len >= 2 && bytes[0] == 0xFE && bytes[1] == 0xFF)
        return { BomEncoding::UTF16_BE, data.substr(2) };
    if (len >= 4 && bytes[0] == 0xFF && bytes[1] == 0xFE && bytes[2] == 0x00 && bytes[3] == 0x00)
        return { BomEncoding::UTF32_LE, data.substr(4) };
    if (len >= 4 && bytes[0] == 0x00 && bytes[1] == 0x00 && bytes[2] == 0xFE && bytes[3] == 0xFF)
        return { BomEncoding::UTF32_BE, data.substr(4) };

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
    std::unique_lock<std::shared_mutex> write_lock(m_iconvCacheMutex);
    m_iconvDescriptorCacheMap.clear();
    
    #if defined(UNICONV_DEBUG_MODE) && UNICONV_DEBUG_MODE
        std::cout << "iconv descriptor cache cleared" << std::endl;
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

StringResult UniConv::ConvertEncodingFast(const std::string& input, 
                                  const char* fromEncoding, 
                                  const char* toEncoding) noexcept {
    // 快速参数检查
    if (!fromEncoding || !toEncoding) {
        return StringResult::Failure(ErrorCode::InvalidParameter);
    }
    
    if (input.empty()) {
        return StringResult::Success(std::string{});
    }
    
    // 直接创建iconv描述符，避免缓存复杂性
    iconv_t cd = iconv_open(toEncoding, fromEncoding);
    if (cd == reinterpret_cast<iconv_t>(-1)) {
        return StringResult::Failure(ErrorCode::ConversionFailed);
    }
    
    // RAII管理iconv描述符
    struct IconvGuard {
        iconv_t cd_;
        ~IconvGuard() { if (cd_ != reinterpret_cast<iconv_t>(-1)) iconv_close(cd_); }
    } guard{cd};
    
    // 输入缓冲区设置
    const char* inbuf_ptr = input.data();
    std::size_t inbuf_left = input.size();
    
    // 预分配输出缓冲区，经验预估避免多次重分配
    std::string result;
    result.reserve(input.size() * 2);
    
    // 使用临时缓冲区进行转换
    std::size_t buffer_size = (std::max)(static_cast<std::size_t>(4096), input.size() * 2);
    std::vector<char> temp_buffer(buffer_size);
    
    // 转换循环，限制最大迭代次数防止死循环
    constexpr int max_iterations = 100;
    int iteration_count = 0;
    
    while (inbuf_left > 0 && iteration_count++ < max_iterations) {
        char* outbuf_ptr = temp_buffer.data();
        std::size_t outbuf_left = temp_buffer.size();
        
        // 执行转换
        std::size_t ret = iconv(cd, &inbuf_ptr, &inbuf_left, &outbuf_ptr, &outbuf_left);
        
        // 计算转换的字节数
        std::size_t converted_bytes = temp_buffer.size() - outbuf_left;
        if (converted_bytes > 0) {
            result.append(temp_buffer.data(), converted_bytes);
        }
        
        // 错误处理
        if (static_cast<std::size_t>(-1) == ret) {
            int current_errno = errno;
            switch (current_errno) {
                case E2BIG:
                    // 输出缓冲区太小，扩容继续
                    if (temp_buffer.size() >= 1048576 * 10) { // 10MB限制
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
            // 转换成功完成
            break;
        }
    }
    
    if (iteration_count >= max_iterations) {
        return StringResult::Failure(ErrorCode::InternalError);
    }
    
    return StringResult::Success(std::move(result));
}

const char* UniConv::GetEncodingNamePtr(int codepage) noexcept {
    auto it = m_encodingMap.find(static_cast<std::uint16_t>(codepage));
    return it != m_encodingMap.end() ? it->second.dotNetName.c_str() : nullptr;
}

StringViewResult UniConv::GetEncodingNameFast(int codepage) noexcept {
    auto it = m_encodingMap.find(static_cast<std::uint16_t>(codepage));
    if (it != m_encodingMap.end()) {
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