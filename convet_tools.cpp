#include "convert_tools.h"
#include "UniConv.h"
#ifdef _WIN32
#include <windows.h>
#endif

// 简化的UTF-8转换为std::wstring（使用UniConv）
std::wstring Utf8ConvertsToUcs4(const std::string& utf8str) {
    try {
        auto conv = UniConv::GetInstance();
        
        // 使用 UniConv 将 UTF-8 转换为系统本地编码，然后转换为宽字符
        std::string local_str = conv->FromUtf8ToLocal(utf8str);
        
        // 使用 MultiByteToWideChar 转换为宽字符
#ifdef _WIN32
        if (local_str.empty()) {
            return std::wstring();
        }
        
        int wlen = MultiByteToWideChar(CP_ACP, 0, local_str.c_str(), -1, nullptr, 0);
        if (wlen <= 0) {
            throw std::runtime_error("Failed to convert to wide string");
        }
        
        std::wstring result(wlen - 1, L'\0'); // -1 因为 wlen 包含了 null terminator
        MultiByteToWideChar(CP_ACP, 0, local_str.c_str(), -1, &result[0], wlen);
        return result;
#else
        // Linux implementation
        return std::wstring(utf8str.begin(), utf8str.end());
#endif
    }
    catch (const std::exception& e) {
        throw std::runtime_error("Failed to convert UTF-8 to UCS-4: " + std::string(e.what()));
    }
}




std::string Ucs4ConvertToUtf8(const std::wstring& wstr) {
    try {
        auto conv = UniConv::GetInstance();
        
        // 使用 WideCharToMultiByte 转换为多字节字符
#ifdef _WIN32
        if (wstr.empty()) {
            return std::string();
        }
        
        int len = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
        if (len <= 0) {
            throw std::runtime_error("Failed to convert from wide string");
        }
        
        std::string local_str(len - 1, '\0'); // -1 因为 len 包含了 null terminator
        WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, &local_str[0], len, nullptr, nullptr);
        
        // 使用 UniConv 将本地编码转换为 UTF-8
        return conv->ToUtf8FromLocal(local_str);
#else
        // Linux implementation
        std::string result(wstr.begin(), wstr.end());
        return conv->ToUtf8FromLocal(result);
#endif
    }
    catch (const std::exception& e) {
        throw std::runtime_error("Failed to convert UCS-4 to UTF-8: " + std::string(e.what()));
    }
}



std::wstring U16StringToWString(const std::u16string& u16str)
{
	std::wstring wstr;

#ifdef _WIN32
	// Windows ƽ̨��wchar_t �� 2 �ֽڣ�UTF-16����ֱ�ӿ���
	wstr.assign(u16str.begin(), u16str.end());
#else
	// Linux ƽ̨��wchar_t �� 4 �ֽڣ�UTF-32������Ҫת��
	std::wstring_convert<std::codecvt_utf16<wchar_t, 0x10ffff, std::little_endian>> converter;
	wstr = converter.from_bytes(
		reinterpret_cast<const char*>(u16str.data()),
		reinterpret_cast<const char*>(u16str.data() + u16str.size())
	);
#endif

	return wstr;
}