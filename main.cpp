#include "UniConv.h"
#include "LightLogWriteImpl.h"

#if CPP_STANDARD < CPP_STANDARD
#include <codecvt>
#endif


auto gUniConv = UniConv::GetInstance();
LightLogWrite_Impl gLogWrite;

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






void TestWide2Utf8() {
	
	std::wstring wstr = L"这是一个测试字符串,用来转换成Utf-8编码";
	std::string sstr1 = gUniConv->WstringConvertToString(wstr);
	std::cout << sstr1 << "\n";
	system("chcp 65001");
	std::string sstr2 = gUniConv->WideConvertToUtf8(wstr);
	std::cout << sstr1 << "\n" << sstr2 << "\n";
}

// 测试成功
void TestGB2321ToUtf8() {
	
	
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
		std::cout << "Before convert:\n" << "\t" << iutf8_str <<"\n\t"<< iutf8_cstr << std::endl;
		std::string convert_out_utf8_str = gUniConv->Utf8ConvertToLocale(iutf8_str);
		std::string convert_oututf8_cstr = gUniConv->Utf8ConvertToLocale(iutf8_cstr); 
		std::cout << "Before convert:\n" << "\t" << convert_out_utf8_str << "\n\t" << convert_oututf8_cstr << std::endl;
	

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

int main() {
	gLogWrite.SetLastingsLogs("./log","uniconv");
	//gLogWrite.SetLogsFileName(L"uniconv.log");
	gLogWrite.WriteLogContent(L"INFO", L"This is a test info  log message.");
	TestGB2321ToUtf8();
	return 0;
}



// 测试日志文件创建和写入
void TestLogFileCreation() {
	LightLogWrite_Impl logger;

	logger.SetLogsFileName(L"test_log.txt");
	logger.WriteLogContent(L"INFO", L"This is a test info  log message.");
	std::this_thread::sleep_for(std::chrono::seconds(1)); // 等待日志写入完成
	std::cout << "TestLogFileCreation: Log file created and message written.\n";
}

// 测试多线程日志写入
void TestMultiThreadLogging() {
	LightLogWrite_Impl logger;
	logger.SetLogsFileName(L"multi_thread_log.txt");

	auto logTask = [&logger](int threadId) {
		for (int i = 0; i < 5; ++i) {
			std::wstring message = L"Thread " + std::to_wstring(threadId) + L" - Log " + std::to_wstring(i);
			logger.WriteLogContent(L"TestMultiThreadLogging", message);
			std::this_thread::sleep_for(std::chrono::milliseconds(100)); // 模拟延迟
		}
		};

	std::vector<std::thread> threads;
	for (int i = 0; i < 5; ++i) {
		threads.emplace_back(logTask, i + 1);
	}

	for (auto& t : threads) {
		t.join();
	}

	std::cout << "TestMultiThreadLogging: Log messages written from multiple threads.\n";
}

