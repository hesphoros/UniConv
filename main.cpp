#include "UniConv.h"
#include "LightLogWriteImpl.h"

#if CPP_STANDARD < CPP_STANDARD
#include <codecvt>
#endif


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
	
	
	std::string chinese_str =  "这是一个测试的string字符串，用来转换成Utf-8编码";
	const char* chinese_cstr = "这是一个测试的const char 字符串，用来转换为Utf-8编码";
	std::string chinese_conv_strutf8 = gUniConv->LocaleConvertToUtf8(chinese_str);
	std::string chinese_conv_cstrutf8  = gUniConv->LocaleConvertToUtf8(chinese_cstr);

	std::string english_str = "This is a test string to convert to UTF-8 encoding";
	const char* english_cstr = "This is a test  const char* str  to convert to UTF-8 encoding";
	std::string english_conv_str = gUniConv->LocaleConvertToUtf8(english_cstr);
	std::string english_conv_cstr = gUniConv->LocaleConvertToUtf8(english_cstr);

	std::string japanese_str = "これはUTF-8エンコードに変換するためのテスト文字列です";
	const char* japanese_cstr = "これはUTF-8エンコードに変換するためのテストC文字列です";
	std::string japanese_conv_str = gUniConv->LocaleConvertToUtf8(japanese_cstr);
    std::string japanese_conv_cstr = gUniConv->LocaleConvertToUtf8(japanese_cstr);

	//韩文
	std::string korean_str = "이것은 UTF-8 인코딩으로 변환하기 위한 테스트 문자열입니다";
	const char* korean_cstr = "이것은 UTF-8 인코딩으로 변환하기 위한 테스트 C 문자열입니다";
	std::string	korean_conv_str = gUniConv->LocaleConvertToUtf8(korean_cstr);
    std::string korean_conv_cstr = gUniConv->LocaleConvertToUtf8(korean_cstr);

	std::string french_str = "Ceci est une chaîne de test pour convertir en encodage Utf-8";
	const char* french_cstr = "Ceci est une chaîne C de test pour convertir en encodage Utf-8";
	std::string french_conv_str = gUniConv->LocaleConvertToUtf8(french_cstr);
    std::string french_conv_cstr = gUniConv->LocaleConvertToUtf8(french_cstr);

	std::string spanish_str = "Esta es una cadena de prueba para convertir a codificación Utf-8";
	const char* spanish_cstr = "Esta es una cadena C de prueba para convertir a codificación Utf-8";
	std::string spanish_conv_str = gUniConv->LocaleConvertToUtf8(spanish_cstr);
    std::string spanish_conv_cstr = gUniConv->LocaleConvertToUtf8(spanish_cstr);

	std::string russian_str = "Это тестовая строка для преобразования в кодировку Utf-8";
	const char* russian_cstr = "Это тестовая C-строка для преобразования в кодировку Utf-8";
    std::string russian_conv_str = gUniConv->LocaleConvertToUtf8(russian_cstr);
    std::string russian_conv_cstr = gUniConv->LocaleConvertToUtf8(russian_cstr);

	//葡萄牙文
	std::string portuguese_str = "Esta é uma string de teste para converter para codificação Utf-8";
	const char* portuguese_cstr = "Esta é uma string C de teste para converter para codificação Utf-8";
    std::string portuguese_conv_str = gUniConv->LocaleConvertToUtf8(portuguese_cstr);
    std::string portuguese_conv_cstr = gUniConv->LocaleConvertToUtf8(portuguese_cstr);


	std::string italian_str = "Questa è una stringa di test per la conversione in codifica Utf-8";
	const char* italian_cstr = "Questa è una stringa C di test per la conversione in codifica Utf-8";
	std::string italian_conv_str = gUniConv->LocaleConvertToUtf8(italian_cstr);
    std::string italian_conv_cstr = gUniConv->LocaleConvertToUtf8(italian_cstr);

	// 打开文件并写入字符串
	std::ofstream out_file("outputUTF8.txt");
	if (out_file.is_open()) {
		//TODO 
		out_file << "Chinese:" << std::endl;
		out_file << "\t" << chinese_conv_strutf8 << "\n" << "\t" << chinese_conv_cstrutf8 << "\n";

		out_file << "English:" << std::endl;
        out_file << "\t" << english_conv_str << "\n" << "\t" << english_conv_cstr << "\n";
		out_file.close();
		std::cout << "String successfully written to file." << std::endl;
	}
	else {
		std::cerr << "Failed to open file for writing." << std::endl;
		
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
	//gLogWrite.SetLastingsLogs("./log","TestGB2312TOUTF8");
	//gLogWrite.SetLogsFileName(L"uniconv.log");
	
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

void Test(){

}
