#include "UniConv.h"



int main(void) {
	
	//std::uint16_t  codepage = UniConv::GetCurrentSystemEncodingCodePage();
	//std::cout << codepage << std::endl;
	//std::cout << UniConv::GetEncodingNameByCodePage(codepage) << std::endl;
	
	system("chcp 65001 > null");
	
	extern char const*  gbk_cstr;
	extern std::string  gbk_str;

	std::cout << UniConv::GetInstance()->LocateConvertToUtf8(gbk_cstr) << "\n";
	std::cout << UniConv::GetInstance()->LocateConvertToUtf8(gbk_str)  << "\n";
	return 0;
}
