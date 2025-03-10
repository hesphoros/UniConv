#include "UniConv.h"



int main(void) {
	
	//std::uint16_t  codepage = UniConv::GetCurrentSystemEncodingCodePage();
	//std::cout << codepage << std::endl;
	//std::cout << UniConv::GetEncodingNameByCodePage(codepage) << std::endl;
	
	system("chcp 65001 > null");
	std::cout << UniConv::GetCurrentSystemEncoding() << std::endl;
	extern char const* gbk_str;
	extern std::string gbk_cppstr;
	//auto res = UniConv::GetInstance()->Convert(gbk_str, UniConv::UTF8,UniConv::GBK);
	/*if (res) {
		std::cout << res.conv_result_str << std::endl;
	}
	else {
		std::cout << res.error_msg << std::endl;
	}*/
	return 0;
}
