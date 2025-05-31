#include <iostream>
#include <string>
#include <chrono>
#include <mutex>
#include <fstream>
#include <queue>
#include <thread>
#include <condition_variable>
#include <atomic>
#include <locale>
#include <codecvt>
#include <ostream>
#include <sstream>
#include <iomanip>
#include <filesystem>
#include <vector>
#include <stdexcept>
#include "convert_tools.h"



//TODO : 接入自己编写的UniConv库

// 日志写入接口
struct LightLogWrite_Info {
	std::wstring                   sLogTagNameVal;//日志标签
	std::wstring                   sLogContentVal;//日志的内容
};

class LightLogWrite_Impl {
public:
	LightLogWrite_Impl() : bIsStopLogging{ false }, bHasLogLasting{ false }

	{
		sWritedThreads = std::thread(&LightLogWrite_Impl::RunWriteThread, this);
	}

	~LightLogWrite_Impl() {
		CloseLogStream();
	}

	/// <summary>
	/// 设置日志文件名  如果不存在将会创建
	/// </summary>
	/// <param name="sFilename"></param>
	void SetLogsFileName(const std::wstring& sFilename) {
		std::lock_guard<std::mutex> sWriteLock(pLogWriteMutex);
		if (pLogFileStream.is_open()) pLogFileStream.close();
		ChecksDirectory(sFilename);//确保目录存在
		pLogFileStream.open(sFilename, std::ios::app);
	}

	void SetLogsFileName(const std::string& sFilename) {
		SetLogsFileName(Utf8ConvertsToUcs4(sFilename));
	}

	void SetLogsFileName(const std::u16string& sFilename) {
		SetLogsFileName(U16StringToWString(sFilename));
	}

	/// <summary>
	/// 设置日志持久化输出
	/// </summary>
	/// <param name="sFilePath"></param>
	/// <param name="sBaseName"></param>
	void SetLastingsLogs(const std::wstring& sFilePath, const std::wstring& sBaseName) {
		sLogLastingDir = sFilePath;
		sLogsBasedName = sBaseName;
		bHasLogLasting = true;
		CreateLogsFile();
	}

	void SetLastingsLogs(const std::u16string& sFilePath, const std::u16string& sBaseName) {
		SetLastingsLogs(U16StringToWString(sFilePath), U16StringToWString(sBaseName));
	}
	void SetLastingsLogs(const std::string& sFilePath, const std::string& sBaseName) {
		SetLastingsLogs(Utf8ConvertsToUcs4(sFilePath), Utf8ConvertsToUcs4(sBaseName));
	}

	void WriteLogContent(const std::wstring& sTypeVal, const std::wstring& sMessage) {
		{
			std::lock_guard<std::mutex> sWriteLock(pLogWriteMutex);
			pLogWriteQueue.push({ sTypeVal, sMessage });
		}
		pWritedCondVar.notify_one();//通知线程
	}

	void WriteLogContent(const std::string& sTypeVal, const std::string& sMessage)
	{
		WriteLogContent(Utf8ConvertsToUcs4(sTypeVal), Utf8ConvertsToUcs4(sMessage));
	}

	void WriteLogContent(const std::u16string& sTypeVal, const std::u16string& sMessage) {
		WriteLogContent(U16StringToWString(sTypeVal), U16StringToWString(sMessage));
	}

private:

	std::wstring BuildLogFileOut() {
		std::tm                 sTmPartsInfo = GetCurrsTimerTm();
		std::wostringstream     sWosStrStream;

		sWosStrStream << std::put_time(&sTmPartsInfo, L"%Y_%m_%d") << (sTmPartsInfo.tm_hour > 12 ? L"_AM" : L"_PM") << L".log";

		bLastingTmTags = (sTmPartsInfo.tm_hour > 12);

		std::filesystem::path   sLotOutPaths = sLogLastingDir;
		std::filesystem::path   sLogOutFiles = sLotOutPaths / (sLogsBasedName + sWosStrStream.str());

		return sLogOutFiles.wstring();
	}

	void CloseLogStream()
	{
		bIsStopLogging = true;
		pWritedCondVar.notify_all();
		WriteLogContent(L"Stop log write thread", L"================================>");
		if (sWritedThreads.joinable()) sWritedThreads.join();//等待线程结束
	}

	void CreateLogsFile()
	{
		std::wstring  sOutFileName = BuildLogFileOut();
		std::lock_guard<std::mutex> sLock(pLogWriteMutex);
		ChecksDirectory(sOutFileName);
		pLogFileStream.close();	//关闭之前提交的文件流
		pLogFileStream.open(sOutFileName, std::ios::app);
	}

	void RunWriteThread() {
		while (true) {
			if (bHasLogLasting)
				if (bLastingTmTags != (GetCurrsTimerTm().tm_hour > 12))
					CreateLogsFile();
			LightLogWrite_Info sLogMessageInf;
			{
				auto sLock = std::unique_lock<std::mutex>(pLogWriteMutex);
				pWritedCondVar.wait(sLock, [this] {return !pLogWriteQueue.empty() || bIsStopLogging; });
				if (bIsStopLogging && pLogWriteQueue.empty()) break;//如果停止标志为真且队列为空，则退出线程
				if (!pLogWriteQueue.empty()) {
					sLogMessageInf = pLogWriteQueue.front();
					pLogWriteQueue.pop();
					std::cerr << "pop:" << Ucs4ConvertToUtf8(sLogMessageInf.sLogContentVal) << "\n";
				}
			}
			if (!sLogMessageInf.sLogContentVal.empty() && pLogFileStream.is_open())
			{
				pLogFileStream << sLogMessageInf.sLogTagNameVal << L"-//>>>" << GetCurrentTimer() << " : " << sLogMessageInf.sLogContentVal << "\n";
			}
		}
		pLogFileStream.close();
		std::cerr << "Log write thread Exit\n";
	}

	void ChecksDirectory(const std::wstring& sFilename) {
		std::filesystem::path sFullFileName(sFilename);
		std::filesystem::path sOutFilesPath = sFullFileName.parent_path();
		if (!sOutFilesPath.empty() && !std::filesystem::exists(sOutFilesPath))
		{
			std::filesystem::create_directories(sOutFilesPath);
		}
	}

	std::wstring GetCurrentTimer() const {
		std::tm              sTmPartsInfo = GetCurrsTimerTm();
		std::wostringstream  sWosStrStream;
		sWosStrStream << std::put_time(&sTmPartsInfo, L"%Y-%m-%d %H:%M:%S");
		return	sWosStrStream.str();
	}

	std::tm GetCurrsTimerTm() const {
		auto        sCurrentTime = std::chrono::system_clock::now();
		std::time_t sCurrTimerTm = std::chrono::system_clock::to_time_t(sCurrentTime);
		std::tm     sCurrTmDatas;
#ifdef _WIN32
		localtime_s(&sCurrTmDatas, &sCurrTimerTm);
#else
		localtime_r(&sCurrTmDatas, &sCurrTimerTm);
#endif
		return sCurrTmDatas;
	}

private:
	std::wofstream                                 pLogFileStream;	// 日志文件流
	std::mutex                                     pLogWriteMutex;	// 日志写入锁
	std::queue<LightLogWrite_Info>                 pLogWriteQueue;	// 日志消息队列
	std::condition_variable	                       pWritedCondVar;	// 条件变量

	std::thread                                    sWritedThreads;	// 日志处理线程
	std::atomic<bool>                              bIsStopLogging;	// 停止标志  default false
	std::wstring                                   sLogLastingDir;	// 持久化日志路径
	std::wstring                                   sLogsBasedName; // 持久化日志选项
	std::atomic<bool>                              bHasLogLasting;	// 是否日志持久化输出 default false
	std::atomic<bool>                              bLastingTmTags;	// 判断时间是上午还是下午
};
