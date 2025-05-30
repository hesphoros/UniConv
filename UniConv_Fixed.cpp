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
#include "LightLogWriteImpl.h"

// ===================== 通用编码转换函数 =====================
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
    
    // 准备输入输出缓冲区
    size_t inbytesleft = input.size();
    size_t outbytesleft = inbytesleft * 4; // 足够大的输出缓冲区
    std::string output(outbytesleft, '\0');
    
    char* inbuf = const_cast<char*>(input.data());
    char* outbuf = &output[0];
    char* outbuf_start = outbuf;
    
    // 执行转换
    size_t ret = iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
    
    if (ret == (size_t)-1) {
        result.error_code = errno;
        result.error_msg = "iconv conversion failed: " + std::string(strerror(errno));
        iconv_close(cd);
        return result;
    }
    
    // 计算实际输出大小
    size_t converted_size = outbuf - outbuf_start;
    output.resize(converted_size);
    
    result.conv_result_str = std::move(output);
    result.error_code = 0;
    
    iconv_close(cd);
    return result;
}

// ===================== 系统编码相关函数 =====================
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

std::uint16_t UniConv::GetCurrentSystemEncodingCodePage() {
#ifdef _WIN32
    return GetACP();
#else
    return 65001; // UTF-8
#endif
}

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

// ===================== UTF-16LE 带长度参数重载 =====================
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

// ===================== 重构所有编码转换方法 =====================

// 系统本地编码 -> UTF-8
std::string UniConv::ToUtf8FromLocal(const std::string& input) {
    std::string currentEncoding = GetCurrentSystemEncoding();
    auto result = ConvertEncoding(input, currentEncoding.c_str(), "UTF-8");
    return result.IsSuccess() ? result.conv_result_str : "";
}

std::string UniConv::ToUtf8FromLocal(const char* input) {
    if (!input) return "";
    return ToUtf8FromLocal(std::string(input));
}

// UTF-8 -> 系统本地编码
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

// 系统本地编码 -> UTF-16LE
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

// 系统本地编码 -> UTF-16BE
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

// UTF-16LE -> 系统本地编码
std::string UniConv::FromUtf16LEToLocal(const std::u16string& input) {
    if (input.empty()) return "";
    
    std::string currentEncoding = GetCurrentSystemEncoding();
    std::string input_bytes(reinterpret_cast<const char*>(input.data()), input.size() * sizeof(char16_t));
    auto result = ConvertEncoding(input_bytes, "UTF-16LE", currentEncoding.c_str());
    return result.IsSuccess() ? result.conv_result_str : "";
}

std::string UniConv::FromUtf16LEToLocal(const char16_t* input) {
    if (!input) return "";
    return FromUtf16LEToLocal(std::u16string(input));
}

// UTF-16BE -> 系统本地编码
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

// ===================== 辅助函数 =====================
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
    MultiByteToWideChar(CP_ACP, 0, sInput, -1, &result[0], wchars_needed);
    return result;
#else
    // Linux实现
    std::wstring_convert<std::codecvt_byname<wchar_t, char, std::mbstate_t>> converter(new std::codecvt_byname<wchar_t, char, std::mbstate_t>(""));
    return converter.from_bytes(sInput);
#endif
}

std::wstring UniConv::LocaleToWideString(const std::string& sInput) {
    return LocaleToWideString(sInput.c_str());
}

// ===================== 错误处理相关 =====================
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
