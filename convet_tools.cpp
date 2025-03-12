#include "convert_tools.h"

// 函数：将 UTF-8 转换为 std::wstring（假设为 UCS-4）
std::wstring Utf8ConvertsToUcs4(const std::string& utf8str) {

	try {
		// 创建 std::wstring_convert 对象，使用 std::codecvt_utf8<wchar_t> 进行转换
		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
		// 将 UTF-8 字符串转换为 std::wstring
		return converter.from_bytes(utf8str);
	}
	catch (const std::range_error& e) {
		// 如果转换失败（例如输入不是有效的 UTF-8），抛出异常
		throw std::runtime_error("Failed to convert UTF-8 to UCS-4: " + std::string(e.what()));
	}
}




std::string Ucs4ConvertToUtf8(const std::wstring& wstr) {
	try {
		// 创建 std::wstring_convert 对象，使用 std::codecvt_utf8<wchar_t> 进行转换
		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
		// 将 std::wstring 转换为 UTF-8 编码的 std::string
		return converter.to_bytes(wstr);
	}
	catch (const std::range_error& e) {
		// 如果转换失败（例如输入包含无效的宽字符），抛出异常
		throw std::runtime_error("Failed to convert UCS-4 to UTF-8: " + std::string(e.what()));
	}
}



std::wstring U16StringToWString(const std::u16string& u16str)
{
	std::wstring wstr;

#ifdef _WIN32
	// Windows 平台：wchar_t 是 2 字节（UTF-16），直接拷贝
	wstr.assign(u16str.begin(), u16str.end());
#else
	// Linux 平台：wchar_t 是 4 字节（UTF-32），需要转换
	std::wstring_convert<std::codecvt_utf16<wchar_t, 0x10ffff, std::little_endian>> converter;
	wstr = converter.from_bytes(
		reinterpret_cast<const char*>(u16str.data()),
		reinterpret_cast<const char*>(u16str.data() + u16str.size())
	);
#endif

	return wstr;
}