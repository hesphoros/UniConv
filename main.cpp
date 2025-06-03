#if _MSC_VER >= 1600 
#pragma execution_character_set("utf-8")
#endif

#include "UniConv.h"
#include "LightLogWriteImpl.hpp"
#include <iostream>
#include <windows.h>

// �����µĲ��Ժ���
void RunAllTests();


constexpr const char* LOG_INFO  = "[  INFO   ]";
constexpr const char* LOG_ERROR = "[  ERROR  ]";
constexpr const char* LOG_DEBUG = "[  DEBUG  ]";
constexpr const char* LOG_WARN  = "[  WARN   ]";
constexpr const char* LOG_FATAL = "[  FATAL  ]";
constexpr const char* LOG_TRACE = "[  TRACE  ]";
constexpr const char* LOG_OK    = "[   OK    ]";

auto g_conv = UniConv::GetInstance();

#define LOGERROR(msg) glogger.WriteLogContent(LOG_ERROR, msg)
#define LOGINFO(msg)  glogger.WriteLogContent(LOG_INFO, msg)
#define LOGDEBUG(msg) glogger.WriteLogContent(LOG_DEBUG, msg)
#define LOGWARN(msg)  glogger.WriteLogContent(LOG_WARN, msg)
#define LOGFATAL(msg) glogger.WriteLogContent(LOG_FATAL, msg)
#define LOGTRACE(msg) glogger.WriteLogContent(LOG_TRACE, msg)
#define LOGOK(msg)    glogger.WriteLogContent(LOG_OK, msg)

int Test() {

    std::cout << "UniConv ����ת������Գ���" << std::endl;
    std::cout << "============================" << std::endl;

    try {
        // �������в���
        RunAllTests();

        std::cout << "���в�����ɣ���鿴��־�ļ�������ļ���" << std::endl;
        std::cout << "��־�ļ���log/TestNewConvert.log" << std::endl;
        std::cout << "����ļ���testdata/output/" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "���Թ����з�������" << e.what() << std::endl;
        return 1;
    }

    return 0;
}

void InitializeLogging() {
    // ��ʼ����־��¼��
    glogger.SetLogsFileName("log/test_log.log");
}



/// <summary>
/// Test GetCurrentSystemEncoding API
/// </summary>
void TestGetCurrentSystemEncoding() {
	std::string sCurSysEnc =  g_conv->GetCurrentSystemEncoding();
    LOGINFO("Current system encoding:\t" + sCurSysEnc);
}
/// <summary>
/// Test GetCurrentSystemEncodingCodePage API
/// </summary>
void TestGetCurrentSystemEncodingCodePage() {
    int nCurSysEncCodePage = g_conv->GetCurrentSystemEncodingCodePage();
    LOGINFO("Current system codepage:\t" + std::to_string(nCurSysEncCodePage));
}

void TestGetEncodingNameByCodePage() {
    int codepage = g_conv->GetCurrentSystemEncodingCodePage();
    LOGINFO("Current system codepage:\t" + std::to_string(codepage));
    std::string encodingName = g_conv->GetEncodingNameByCodePage(codepage);
    LOGINFO("Encoding name for codepage " + std::to_string(codepage) + ":\t" + encodingName);
    std::string convResult = g_conv->GetEncodingNameByCodePage(codepage);
    if (convResult != encodingName) {
        LOGERROR("Encoding name mismatch for codepage " + std::to_string(codepage) + ": expected '" + encodingName + "', got '" + convResult + "'");
    } else {
        LOGOK("Encoding name for codepage " + std::to_string(codepage) + " is correct: " + convResult);
    }

}

int  main() {

    SetConsoleOutputCP(CP_UTF8);
    InitializeLogging();

    TestGetCurrentSystemEncoding();
    TestGetEncodingNameByCodePage();


    return 0;
}