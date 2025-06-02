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
*****************************************************************************/
#include "UniConv.h"
#include "LightLogWriteImpl.hpp"


namespace {
	constexpr std::string_view encoding_names[] = {
	#define X(name, str) str,
	#include "encodings.inc"
	#undef X
	};
}

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

std::unordered_map<std::string, UniConv::IconvSharedPtr> UniConv::m_iconvDesscriptorCacheMapS = {};


// ===================== General convert functions =====================
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
        return result;    }

    // Prepare input and output buffers
    size_t inbytesleft = input.size();
    // Enough space for output, assuming worst case of 4 bytes per input byte
    // (e.g., UTF-8 to UTF-32 conversion)
    size_t outbytesleft = inbytesleft * 4; 
    std::string output(outbytesleft, '\0');
      char* inbuf = const_cast<char*>(input.data());
    char* outbuf = &output[0];
    char* outbuf_start = outbuf;
    
   
    size_t ret = iconv(cd, const_cast<const char**>(&inbuf), &inbytesleft, &outbuf, &outbytesleft);
    
    if (ret == (size_t)-1) {
        result.error_code = errno;
        result.error_msg = "iconv conversion failed: " + std::string(strerror(errno));
        iconv_close(cd);
        return result;
    }
    
    // Calculate the size of the converted output
    size_t converted_size = outbuf - outbuf_start;
    output.resize(converted_size);
    
    result.conv_result_str = std::move(output);
    result.error_code = 0;
    
    iconv_close(cd);
    return result;
}

// ===================== System encoding related functions =====================
std::string UniConv::GetCurrentSystemEncoding()
{
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

std::uint16_t UniConv::GetCurrentSystemEncodingCodePage() {
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

std::string  UniConv::GetEncodingNameByCodePage(std::uint16_t codePage)
{
	auto it = m_encodingMap.find(codePage);
	if (it != m_encodingMap.end())
		return it->second.dotNetName;
	else
		return "Encoding not found.";
}


// ===================== UTF-16LE with length parameter overloads =====================
std::string UniConv::FromUtf16LEToUtf8(const char16_t* input, size_t len) {
    if (!input || len == 0) return "";

    std::string input_bytes(reinterpret_cast<const char*>(input), len * sizeof(char16_t));
    auto result = ConvertEncoding(input_bytes, "UTF-16LE", "UTF-8");
    return result.IsSuccess() ? result.conv_result_str : "";
}

std::string UniConv::FromUtf16BEToUtf8(const char16_t* input, size_t len) {
    if (!input || len == 0) return "";

    std::string input_bytes(reinterpret_cast<const char*>(input), len * sizeof(char16_t));
    auto result = ConvertEncoding(input_bytes, "UTF-16BE", "UTF-8");
    return result.IsSuccess() ? result.conv_result_str : "";
}

// ===================== Refactored encoding conversion methods =====================

// System local encoding -> UTF-8
std::string UniConv::ToUtf8FromLocal(const std::string& input) {
    std::string currentEncoding = GetCurrentSystemEncoding();
    auto result = ConvertEncoding(input, currentEncoding.c_str(), "UTF-8");
    return result.IsSuccess() ? result.conv_result_str : "";
}

std::string UniConv::ToUtf8FromLocal(const char* input) {
    if (!input) return "";
    return ToUtf8FromLocal(std::string(input));
}

// UTF-8 -> System local encoding
std::string UniConv::FromUtf8ToLocal(const std::string& input) {
    std::string currentEncoding = GetCurrentSystemEncoding();
    auto result = ConvertEncoding(input, "UTF-8", currentEncoding.c_str());
    return result.IsSuccess() ? result.conv_result_str : "";
}

std::string UniConv::FromUtf8ToLocal(const char* input) {
    if (!input) return "";
    return FromUtf8ToLocal(std::string(input));
}

// UTF-16LE -> UTF-8
std::string UniConv::FromUtf16LEToUtf8(const std::u16string& input) {
    if (input.empty()) return "";

    std::string input_bytes(reinterpret_cast<const char*>(input.data()), input.size() * sizeof(char16_t));
    auto result = ConvertEncoding(input_bytes, "UTF-16LE", "UTF-8");
    return result.IsSuccess() ? result.conv_result_str : "";
}

std::string UniConv::FromUtf16LEToUtf8(const char16_t* input) {
    if (!input) return "";
    return FromUtf16LEToUtf8(std::u16string(input));
}

// UTF-16BE -> UTF-8
std::string UniConv::FromUtf16BEToUtf8(const std::u16string& input) {
    if (input.empty()) return "";
    
    std::string input_bytes(reinterpret_cast<const char*>(input.data()), input.size() * sizeof(char16_t));
    auto result = ConvertEncoding(input_bytes, "UTF-16BE", "UTF-8");
    return result.IsSuccess() ? result.conv_result_str : "";
}

std::string UniConv::FromUtf16BEToUtf8(const char16_t* input) {
    if (!input) return "";
    return FromUtf16BEToUtf8(std::u16string(input));
}

// UTF-8 -> UTF-16LE
std::u16string UniConv::FromUtf8ToUtf16LE(const std::string& input) {
    auto result = ConvertEncoding(input, "UTF-8", "UTF-16LE");
    if (result.IsSuccess() && result.conv_result_str.size() % 2 == 0) {
        return std::u16string(reinterpret_cast<const char16_t*>(result.conv_result_str.data()), 
                             result.conv_result_str.size() / 2);
    }
    return std::u16string();
}

std::u16string UniConv::FromUtf8ToUtf16LE(const char* input) {
    if (!input) return std::u16string();
    return FromUtf8ToUtf16LE(std::string(input));
}

// UTF-8 -> UTF-16BE
std::u16string UniConv::FromUtf8ToUtf16BE(const std::string& input) {
    auto result = ConvertEncoding(input, "UTF-8", "UTF-16BE");
    if (result.IsSuccess() && result.conv_result_str.size() % 2 == 0) {
        return std::u16string(reinterpret_cast<const char16_t*>(result.conv_result_str.data()), 
                             result.conv_result_str.size() / 2);
    }
    return std::u16string();
}

std::u16string UniConv::FromUtf8ToUtf16BE(const char* input) {
    if (!input) return std::u16string();
    return FromUtf8ToUtf16BE(std::string(input));
}

// System local encoding -> UTF-16LE
std::u16string UniConv::ToUtf16LEFromLocal(const std::string& input) {
    std::string currentEncoding = GetCurrentSystemEncoding();
    auto result = ConvertEncoding(input, currentEncoding.c_str(), "UTF-16LE");
    if (result.IsSuccess() && result.conv_result_str.size() % 2 == 0) {
        return std::u16string(reinterpret_cast<const char16_t*>(result.conv_result_str.data()), 
                             result.conv_result_str.size() / 2);
    }
    return std::u16string();
}

std::u16string UniConv::ToUtf16LEFromLocal(const char* input) {
    if (!input) return std::u16string();
    return ToUtf16LEFromLocal(std::string(input));
}

// System local encoding -> UTF-16BE
std::u16string UniConv::ToUtf16BEFromLocal(const std::string& input) {
    std::string currentEncoding = GetCurrentSystemEncoding();
    auto result = ConvertEncoding(input, currentEncoding.c_str(), "UTF-16BE");
    if (result.IsSuccess() && result.conv_result_str.size() % 2 == 0) {
        return std::u16string(reinterpret_cast<const char16_t*>(result.conv_result_str.data()), 
                             result.conv_result_str.size() / 2);
    }
    return std::u16string();
}

std::u16string UniConv::ToUtf16BEFromLocal(const char* input) {
    if (!input) return std::u16string();
    return ToUtf16BEFromLocal(std::string(input));
}

// UTF-16LE -> ϵͳ���ر���
std::string UniConv::FromUtf16LEToLocal(const std::u16string& input) {
    if (input.empty()) return "";
    
    std::string currentEncoding = this->GetCurrentSystemEncoding();
    std::string input_bytes(reinterpret_cast<const char*>(input.data()), input.size() * sizeof(char16_t));
    auto result = this->ConvertEncoding(input_bytes, "UTF-16LE", currentEncoding.c_str());
    return result.IsSuccess() ? result.conv_result_str : "";
}

std::string UniConv::FromUtf16LEToLocal(const char16_t* input) {
    if (!input) return "";
    return this->FromUtf16LEToLocal(std::u16string(input));
}

// UTF-16BE -> System local encoding
std::string UniConv::FromUtf16BEToLocal(const std::u16string& input) {
    if (input.empty()) return "";
    
    std::string currentEncoding = GetCurrentSystemEncoding();
    std::string input_bytes(reinterpret_cast<const char*>(input.data()), input.size() * sizeof(char16_t));
    auto result = ConvertEncoding(input_bytes, "UTF-16BE", currentEncoding.c_str());
    return result.IsSuccess() ? result.conv_result_str : "";
}

std::string UniConv::FromUtf16BEToLocal(const char16_t* input) {
    if (!input) return "";
    return FromUtf16BEToLocal(std::u16string(input));
}

// UTF-16LE -> UTF-16BE
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

std::u16string UniConv::FromUtf16LEToUtf16BE(const char16_t* input) {
    if (!input) return std::u16string();
    return FromUtf16LEToUtf16BE(std::u16string(input));
}

// UTF-16BE -> UTF-16LE
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

std::u16string UniConv::FromUtf16BEToUtf16LE(const char16_t* input) {
    if (!input) return std::u16string();
    size_t len = 0;
    while (input[len] != 0) ++len;
    return FromUtf16BEToUtf16LE(std::u16string(input, len));
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

std::wstring UniConv::LocaleToWideString(const char* sInput) {
#ifdef _WIN32
    if (!sInput) return L"";
    
    int wchars_needed = MultiByteToWideChar(CP_ACP, 0, sInput, -1, nullptr, 0);
    if (wchars_needed <= 0) return L"";
    
    std::wstring result(wchars_needed - 1, L'\0');
    MultiByteToWideChar(CP_ACP, 0, sInput, -1, &result[0], wchars_needed);    return result;
#else
    // Linux implementation
    std::wstring_convert<std::codecvt_byname<wchar_t, char, std::mbstate_t>> converter(new std::codecvt_byname<wchar_t, char, std::mbstate_t>(""));
    return converter.from_bytes(sInput);
#endif
}

std::wstring UniConv::LocaleToWideString(const std::string& sInput) {
    return LocaleToWideString(sInput.c_str());
}

// ===================== Error Handling Related =====================
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
    }
    return "Unknown iconv error: " + std::to_string(err_code);
}
