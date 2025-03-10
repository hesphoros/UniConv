#include "UniConv.h"
#include <codecvt>


std::u16string to_utf16(std::string str) // utf-8 to utf16
{
	return std::wstring_convert< std::codecvt_utf8_utf16<char16_t>, char16_t >{}.from_bytes(str);
}

std::string to_utf8(std::u16string str16)
{
	return std::wstring_convert< std::codecvt_utf8_utf16<char16_t>, char16_t >{}.to_bytes(str16);
}

std::u32string to_utf32(std::string str)
{
	return std::wstring_convert< std::codecvt_utf8<char32_t>, char32_t >{}.from_bytes(str);
}

std::string to_utf8(std::u32string str32)
{
	return std::wstring_convert< std::codecvt_utf8<char32_t>, char32_t >{}.to_bytes(str32);
}

std::wstring to_wchar_t(std::string str)
{
	return std::wstring_convert< std::codecvt_utf8<wchar_t>, wchar_t >{}.from_bytes(str);
}

std::string to_utf8(std::wstring wstr)
{
	return std::wstring_convert< std::codecvt_utf8<wchar_t>, wchar_t >{}.to_bytes(wstr);
}


auto gUniConv = UniConv::GetInstance();

void TestUtf82Locale() {
	extern char const* utf8_cstr;
	extern std::string utf8_str;
    std::cout << gUniConv->Utf8ConvertToLocale(utf8_cstr) << "\n";
    std::cout << gUniConv->Utf8ConvertToLocale(utf8_str)  << "\n";
}


void TestLocale2Utf8() {
	system("chcp 65001 > null");
	extern char const* gbk_cstr;
	extern std::string gbk_str;
	std::cout << gUniConv->LocaleConvertToUtf8(gbk_cstr) << "\n";
	std::cout << gUniConv->LocaleConvertToUtf8(gbk_str)  << "\n";

}


void TestLocale2Utf16LE() {
	const char* test_cstr = "这是一个c测试字符串,用来转换成Utf-16LE编码";
	std::string test_str = "这是一个cpp测试字符串,用来转换成Utf-16LE编码";
	
	std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter;
	std::cout << converter.to_bytes(gUniConv->LocaleConvertToUtf16LE(test_cstr)) << "\n";
	std::cout << converter.to_bytes(gUniConv->LocaleConvertToUtf16LE(test_str)) << "\n";
	
}

void TestLocale2Utf16BE() {

	const char* test_cstr = "这是一个c测试字符串,用来转换成Utf-16BE编码";
	std::string test_str = "这是一个cpp测试字符串,用来转换成Utf-16BE编码";
	std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter;
	//std::cout << gUniConv->Utf16BEConvertToLocale((gUniConv->LocaleConvertToUtf16BE(test_cstr))) << "\n";
    std::cout << converter.to_bytes(gUniConv->LocaleConvertToUtf16BE(test_cstr)) << "\n";
    //std::cout << converter.to_bytes(gUniConv->LocaleConvertToUtf16BE(test_str)) << "\n";
}


int main(void) {
	//1200
	//TestLocale2Utf16BE();
	system("chcp 65001");
	TestLocale2Utf16LE();
	return 0;
}
