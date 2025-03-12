#include "UniConv.h"
#include <codecvt>
#define DEBUG

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


void TestWide2Utf8() {
	
	std::wstring wstr = L"这是一个测试字符串,用来转换成Utf-8编码";
	std::string sstr1 = gUniConv->WstringConvertToString(wstr);
	std::cout << sstr1 << "\n";
	system("chcp 65001");
	std::string sstr2 = gUniConv->WideConvertToUtf8(wstr);
	std::cout << sstr1 << "\n" << sstr2 << "\n";
}

void TestGB2321ToUtf8() {
	// 测试成功
	//设置本地环境编码为UTF-8
	
	std::string src_str = "这是一个测试的字符串，用来转换成Utf-8编码";
	const char* src_cstr = "这是一个测试的C字符串，用来转换为Utf-8编码";
	std::string res_cstrutf8 = gUniConv->LocaleConvertToUtf8(src_cstr);
	std::string res_strutf8  = gUniConv->LocaleConvertToUtf8(src_str);
	// 打开文件并写入字符串
	std::ofstream out_file("outputUTF8.txt");
	if (out_file.is_open()) {
		out_file << res_strutf8 << std::endl; // 将字符串写入文件
		out_file << res_cstrutf8;
		out_file.close();
		std::cout << "String successfully written to file." << std::endl;
	}
	else {
		std::cerr << "Failed to open file for writing." << std::endl;
		
	}
	std::string iutf8_str;
	std::string iutf8_cstr;
	std::ifstream in_file("outputUTF8.txt");
	if (in_file.is_open()) {		
       
		std::getline(in_file, iutf8_str);		
		std::getline(in_file, iutf8_cstr);
		in_file.close();
		std::cout << "Before convert " << iutf8_str <<iutf8_str << std::endl;
		std::string convert_out_utf8_str = gUniConv->Utf8ConvertToLocale(iutf8_str);
		std::string convert_oututf8_cstr = gUniConv->Utf8ConvertToLocale(iutf8_cstr); 
		std::cout << "After convert" << convert_out_utf8_str << convert_oututf8_cstr << std::endl;
	

	}
	else {
		std::cerr << "Failed to open file for reading." << std::endl;
		
	}
}

void TestGB18030ToUTF8() {
	std::string gb18030_str = {};
	std::ifstream in_file("InputGB18030.txt");
	if (in_file.is_open()) {
		std::getline(in_file,gb18030_str);
		in_file.close();
		std::cout << "Before convert " << gb18030_str << std::endl;
		std::string convert_out_utf8_str = gUniConv->Utf8ConvertToLocale(gb18030_str);
		std::cout<< "After convert" << convert_out_utf8_str << std::endl;
	}



}

int main(void) {
	TestGB2321ToUtf8();
}
