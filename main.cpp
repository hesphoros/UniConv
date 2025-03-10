#include "UniConv.h"





int main(void) {
	
	//std::uint16_t  codepage = UniConv::GetCurrentSystemEncodingCodePage();
	//std::cout << codepage << std::endl;
	//std::cout << UniConv::GetEncodingNameByCodePage(codepage) << std::endl;		
	
	extern char const* utf8_cstr;
	extern std::string utf8_str;

	std::cout << UniConv::GetInstance()->Utf8ConvertToLocate(utf8_cstr) << "\n";
	std::cout << UniConv::GetInstance()->Utf8ConvertToLocate(utf8_str)  << "\n";

	//等待三秒
    Sleep(3000);

	system("chcp 65001 > null");

	extern char const* gbk_cstr;
	extern std::string gbk_str;

	std::cout << UniConv::GetInstance()->LocateConvertToUtf8(gbk_cstr) << "\n";
    std::cout << UniConv::GetInstance()->LocateConvertToUtf8(gbk_str)  << "\n";

	//1200
	
	return 0;
}
