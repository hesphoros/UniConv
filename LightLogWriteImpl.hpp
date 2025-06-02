#ifndef INCLUDE_LIGHTLOGWRITEIMPL_HPP_
#define INCLUDE_LIGHTLOGWRITEIMPL_HPP_
/*****************************************************************************
 *  LightLogWriteImpl
 *  Copyright (C) 2025 hesphoros <hesphoros@gmail.com>
 *
 *  This file is part of LightLogWriteImpl.
 *
 *  This program is free software; you can redistribute it d:\codespace\LightLogWriteImpl\include\LightLogWriteImpl.hppand/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 *  @file     LightLogWriteImpl.hpp
 *  @brief    有锁实现的轻量级的日志库
 *  @details  详细描述
 *
 *  @author   hesphoros
 *  @email    hesphoros@gmail.com
 *  @version  1.0.0.1
 *  @date     2025/05/27
 *  @license  GNU General Public License (GPL)
 *---------------------------------------------------------------------------*
 *  Remark         : None
 *---------------------------------------------------------------------------*
 *  Change History :
 *  <Date>     | <Version> | <Author>       | <Description>
 *  2025/03/27 | 1.0.0.1   | hesphoros      | Create file
 *****************************************************************************/

#ifdef _MSC_VER
#pragma execution_character_set("utf-8")
#endif


#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>
#include <queue>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <atomic>
#include <chrono>
#include <filesystem>
#include <locale>
#include <codecvt>
#include <stdexcept>
#include <memory>


 /**
  * @file LightLogWriteCommon.h
  * @brief Common definitions and structures for LightLogWrite.
  * This file contains the definitions of structures and enums used in the LightLogWrite library.
  * It includes the LightLogWriteInfo structure for log messages and the LogQueueOverflowStrategy enum for handling full log queues.
  * @note Need -std-cpp17
  */

  // TODO: use iconv for character encoding conversion #include <iconv.h> // Uncomment if you want
  // to use iconv for character encoding conversion #pragma comment(lib, "libiconv.lib")

  /**
   * @brief Structure for log message information.
   * @param sLogTagNameVal The tag name of the log.
   * * It can be used to categorize or identify the log message.
   * * such as INFO , WARNING, ERROR, etc.
   * @param sLogContentVal The content of the log message.
   * * It contains the actual log message that will be written to the log file.
   * * This can include any relevant information that needs to be logged, such as error messages, status updates, etc.
  */
struct LightLogWriteInfo {
	std::wstring                   sLogTagNameVal;  /*!< Log tag name */
	std::wstring                   sLogContentVal;  /*!< Log content */
};

/**
	* @brief Enum for strategies to handle full log queues.
	* @details
	* * This enum defines the strategies that can be used when the log queue is full.
	* * It provides options for blocking until space is available or dropping the oldest log entry.
	* @param Block Blocked waiting for space in the queue.
	* * When the queue is full, the logging operation will block until space becomes available.
	* @param DropOldest Drop the oldest log entry when the queue is full.
	* * When the queue is full, the oldest log entry will be removed to make space for the new log entry.
	* * This strategy allows for continuous logging without blocking, but may result in loss of older log entries.
	*/
enum class LogQueueOverflowStrategy {
	Block,      /*!< Blocked waiting           */
	DropOldest  /*!< Drop the oldest log entry */
};

/**
	* @brief Converts a UTF-8 encoded string to UCS-4 (UTF-32) encoded wide string
	* @param utf8str The UTF-8 encoded string to be converted
	* @return A wide string (std::wstring) representing the UCS-4 encoded string
	* @details This function uses std::wstring_convert with std::codecvt_utf8<wchar_t> to perform the conversion.
	*/
static inline std::wstring Utf8ConvertsToUcs4(const std::string& utf8str) {
	try {
		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
		return converter.from_bytes(utf8str);
	}
	catch (const std::range_error& e) {
		throw std::runtime_error("Failed to convert UTF-8 to UCS-4: " + std::string(e.what()));
	}
}

/**
	* @brief Converts a UCS-4 (UTF-32) encoded wide string to UTF-8 encoded string
	* @param wstr The UCS-4 encoded wide string to be converted
	* @return A UTF-8 encoded string (std::string) representing the converted wide string
	* @details This function uses std::wstring_convert with std::codecvt_utf8<wchar_t> to perform the conversion.
	*/
static inline std::string Ucs4ConvertToUtf8(const std::wstring& wstr) {
	try {
		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
		return converter.to_bytes(wstr);
	}
	catch (const std::range_error& e) {
		throw std::runtime_error("Failed to convert UCS-4 to UTF-8: " + std::string(e.what()));
	}
}

/**
	* @brief Converts a UTF-16 encoded string to a wide string (UCS-4)
	* @param u16str The UTF-16 encoded string to be converted
	* @return A wide string (std::wstring) representing the UCS-4 encoded string
	* @details This function uses std::wstring_convert with std::codecvt_utf16<wchar_t, 0x10ffff, std::little_endian> to perform the conversion.
	*/
static inline std::wstring U16StringToWString(const std::u16string& u16str) {
	std::wstring wstr;
#ifdef _WIN32
	wstr.assign(u16str.begin(), u16str.end());
#else
	std::wstring_convert<std::codecvt_utf16<wchar_t, 0x10ffff, std::little_endian>>converter;
	wstr = converter.from_bytes(
		reinterpret_cast<const char*>(u16str.data()),
		reinterpret_cast<const char*>(u16str.data() + u16str.size()));
#endif
	return wstr;
}

/**
 * @brief Implementation of the LightLogWrite class
 * @details This class provides a thread-safe implementation for writing logs to a file.
*/
class LightLogWrite_Impl {
public:
	/**
		* @brief Constructor for LightLogWrite_Impl
		* @param maxQueueSize The max log written queue size
		* @param strategy The strategy for handling full log queue
		* @param reportInterval The interval for reporting log overflow
		* @details This constructor initializes the log writer with a specified maximum queue size,
		*          a strategy for handling full queues, and an interval for reporting log overflow.
		* @note The default maximum queue size is 500000, the default strategy is to block when the queue is full,
		*       and the default report interval is 100 discarded logs.
		* @version 1.0.0
		*/
	LightLogWrite_Impl(size_t maxQueueSize = 500000, LogQueueOverflowStrategy strategy = LogQueueOverflowStrategy::Block, size_t reportInterval = 100)
		: kMaxQueueSize(maxQueueSize),
		discardCount(0),
		lastReportedDiscardCount(0),
		bIsStopLogging{ false },
		queueFullStrategy(strategy),
		reportInterval(reportInterval),
		bHasLogLasting{ false } {
		sWrittenThreads = std::thread(&LightLogWrite_Impl::RunWriteThread, this);
	}

	/**
		* @brief Destructor for LightLogWrite_Impl
		* @details This destructor closes the log file stream and stops the logging thread.
		* * It will write a finial log entry indicating that the logging has stopped.
		* @note It is important to call this destructor to ensure that all logs are flushed and the thread is properly terminated.
		*/
	~LightLogWrite_Impl() {
		CloseLogStream();
	}

	/**
		* @brief Sets the log file name for logging
		* @param sFilename The name of the log file to be used for logging
		* @details This function sets the log file name and ensures that the directory exists.
		* * If the directory does not exist, it will be created.
		*/
	void SetLogsFileName(const std::wstring& sFilename) {
		std::lock_guard<std::mutex> sWriteLock(pLogWriteMutex);
		if (pLogFileStream.is_open())
			pLogFileStream.close();
		ChecksDirectory(sFilename); //  确保目录存在
		pLogFileStream.open(sFilename, std::ios::app);
	}

	/**
		* @brief Sets the log file name for logging
		* @param sFilename The name of the log file to be used for logging
		* @details This function sets the log file name and ensures that the directory exists.
		* * If the directory does not exist, it will be created.
		*/
	void SetLogsFileName(const std::string& sFilename) {
		SetLogsFileName(Utf8ConvertsToUcs4(sFilename));
	}

	void SetLogsFileName(const std::u16string& sFilename) {
		SetLogsFileName(U16StringToWString(sFilename));
	}

	/**
		* @brief Sets the directory and base name for lasting logs
		* @param sFilePath The directory path where the logs will be stored
		* @param sBaseName The base name for the log files
		* @details This function sets the directory and base name for lasting logs.
		* * It will create a new log file based on the current date and time.
		* * The log files will be named in the format "BaseName_YYYY_MM_DD_AM/PM.log". such as "LightLogWriteImpl_2025_05_27_AM.log".
		* * If the directory does not exist, it will be created.
		* @note This function should be called before writing any logs to ensure that the logs are stored correctly.
		*/
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

	/**
		* @brief Writes a log message with a specific type
		* @param sTypeVal The type of the log message (e.g., "INFO", "ERROR")
		* @param sMessage The content of the log message
		* @details This function writes a log message with a specific type to the log file.
		* It will also handle log overflow according to the specified strategy.
		*/
	void WriteLogContent(const std::wstring& sTypeVal, const std::wstring& sMessage) {
		bool bNeedReport = false;
		size_t currentDiscard = 0;
		static thread_local bool inErrorReport = false;

		if (queueFullStrategy == LogQueueOverflowStrategy::Block) {
			std::unique_lock<std::mutex> sWriteLock(pLogWriteMutex);
			// std::wcerr << L"[WriteLogContent] Try push (Block), queue size: " <<
			// pLogWriteQueue.size() << std::endl;
			pWrittenCondVar.wait(sWriteLock, [this] { return pLogWriteQueue.size() < kMaxQueueSize; });
			pLogWriteQueue.push({ sTypeVal, sMessage });
			// std::wcerr << L"[WriteLogContent] Pushed (Block), queue size: " <<
			// pLogWriteQueue.size() << std::endl;
		}
		else if (queueFullStrategy == LogQueueOverflowStrategy::DropOldest) {
			std::lock_guard<std::mutex> sWriteLock(pLogWriteMutex);
			// std::wcerr << L"[WriteLogContent] Try push (DropOldest), queue size: " <<
			// pLogWriteQueue.size() << std::endl;
			if (pLogWriteQueue.size() >= kMaxQueueSize) {
				// std::wcerr << L"[WriteLogContent] Drop oldest, queue full: " <<
				// pLogWriteQueue.size() << std::endl;
				pLogWriteQueue.pop();
				++discardCount;
				if (discardCount - lastReportedDiscardCount >= reportInterval) {
					bNeedReport = true;
					currentDiscard = discardCount;
					lastReportedDiscardCount.store(discardCount.load());
				}
			}
			pLogWriteQueue.push({ sTypeVal, sMessage });
			// std::wcout << L"[WriteLogContent] Pushed (DropOldest), queue size: " <<
			// pLogWriteQueue.size() << std::endl;
		}
		pWrittenCondVar.notify_one();
		if (bNeedReport && !inErrorReport) {
			inErrorReport = true;
			std::wstring overflowMsg = L"The log queue overflows and has been discarded " + std::to_wstring(currentDiscard) + L" logs";
			// std::wcerr << L"[WriteLogContent] Report overflow: " << overflowMsg << std::endl;
			WriteLogContent(L"LOG_OVERFLOW", overflowMsg);
			inErrorReport = false;
		}
	}

	void WriteLogContent(const std::string& sTypeVal, const std::string& sMessage) {
		WriteLogContent(Utf8ConvertsToUcs4(sTypeVal), Utf8ConvertsToUcs4(sMessage));
	}

	void WriteLogContent(const std::u16string& sTypeVal, const std::u16string& sMessage) {
		WriteLogContent(U16StringToWString(sTypeVal), U16StringToWString(sMessage));
	}

	/**
		* @brief Gets the current discard count
		* @return The number of discarded log messages
		* @retval size_t The number of log messages that have been discarded due to overflow
		* @details This function returns the current count of discarded log messages.
		*/
	size_t GetDiscardCount() const {
		return discardCount;
	}

	/**
		* @brief Resets the discard count to zero
		* @details This function resets the count of discarded log messages to zero.
		* * It can be useful for clearing the count after handling or reporting the discarded logs.
		* @note This function does not affect the log messages themselves, only the count of discarded messages.
		*/
	void ResetDiscardCount() {
		discardCount = 0;
	}

private:
	/**
		* @brief Builds the output log file name based on the current date and time
		* @return A wide string representing the log file name
		* @details This function constructs the log file name using the current date and time.
		* * The format is "BaseName_YYYY_MM_DD_AM/PM.log", where BaseName is the base name set by SetLastingsLogs.
		*/
	std::wstring BuildLogFileOut() {
		std::tm sTmPartsInfo = GetCurrsTimerTm();
		std::wostringstream sWosStrStream;

		sWosStrStream << std::put_time(&sTmPartsInfo, L"%Y_%m_%d") << (sTmPartsInfo.tm_hour >= 12 ? L"_AM" : L"_PM") << L".log";

		bLastingTmTags = (sTmPartsInfo.tm_hour > 12);

		std::filesystem::path sLotOutPaths = sLogLastingDir;
		std::filesystem::path sLogOutFiles = sLotOutPaths / (sLogsBasedName + sWosStrStream.str());

		return sLogOutFiles.wstring();
	}

	/**
	* @brief Closes the log stream and stops the logging thread
	* @details This function sets the stop flag for the logging thread, notifies it to wake up,
	* * and waits for the thread to finish.
	* * It also writes a final log entry indicating that the logging has stopped.
	* @note It is important to call this function to ensure that all logs are flushed and the thread is properly terminated.
	*/
	void CloseLogStream() {
		bIsStopLogging = true;
		pWrittenCondVar.notify_all();
		WriteLogContent(L"<================================              Stop log write thread    ", L"================================>");
		if (sWrittenThreads.joinable())
			sWrittenThreads.join(); // 等待线程结束
	}

	/**
		* @brief Creates a new log file based on the current date and time
		* @details This function constructs a new log file name using the current date and time,
		* * checks if the directory exists, and opens the log file stream for appending.
		* * If the directory does not exist, it will be created.
		*/
	void CreateLogsFile() {
		std::wstring sOutFileName = BuildLogFileOut();
		std::lock_guard<std::mutex> sLock(pLogWriteMutex);
		ChecksDirectory(sOutFileName);
		pLogFileStream.close(); // 关闭之前提交的文件流
		pLogFileStream.open(sOutFileName, std::ios::app);
	}

	/**
		* @brief Runs the log writing thread
		* @details This function runs in a separate thread and continuously checks for new log messages to write.
		* * It waits for new log messages to be added to the queue and writes them to the log file.
		* * If the log file needs to be created or rotated, it will handle that as well.
		* * The thread will exit when the stop flag is set and the queue is empty.
		* @note This function should be called in a separate thread to avoid blocking the main application.
		*/
	void RunWriteThread() {
		while (true) {
			if (bHasLogLasting)
				if (bLastingTmTags != (GetCurrsTimerTm().tm_hour > 12))
					CreateLogsFile();
			LightLogWriteInfo sLogMessageInf;

			{
				auto sLock = std::unique_lock<std::mutex>(pLogWriteMutex);
				pWrittenCondVar.wait(sLock, [this]
					{ return !pLogWriteQueue.empty() || bIsStopLogging; });

				if (bIsStopLogging && pLogWriteQueue.empty())
					break; // 如果停止标志为真且队列为空，则退出线程
				if (!pLogWriteQueue.empty()) {
					sLogMessageInf = pLogWriteQueue.front();
					pLogWriteQueue.pop();
					pWrittenCondVar.notify_one();
				}
			}
			if (!sLogMessageInf.sLogContentVal.empty() && pLogFileStream.is_open()) {
				pLogFileStream << sLogMessageInf.sLogTagNameVal << L"-//>>>" << GetCurrentTimer() << " : " << sLogMessageInf.sLogContentVal << "\n";
			}
		}
		pLogFileStream.close();
		std::cerr << "Log write thread Exit\n";
	}

	/**
		* @brief Checks if the directory for the log file exists, and creates it if it does not
		* @param sFilename The full path of the log file
		* @details This function checks if the directory of the specified log file exists.
		* * If the directory does not exist, it will create all necessary directories.
		*/
	void ChecksDirectory(const std::wstring& sFilename) {
		std::filesystem::path sFullFileName(sFilename);
		std::filesystem::path sOutFilesPath = sFullFileName.parent_path();
		if (!sOutFilesPath.empty() && !std::filesystem::exists(sOutFilesPath))
		{
			std::filesystem::create_directories(sOutFilesPath);
		}
	}
	/**
		* @brief Gets the current time as a formatted string
		* @return A wide string representing the current time in the format "YYYY-MM-DD HH:MM:SS"
		* @details This function retrieves the current system time and formats it into a wstring.
		* * The format used is "YYYY-MM-DD HH:MM:SS", which is suitable for logging purposes.
		*/
	std::wstring GetCurrentTimer() const {
		std::tm sTmPartsInfo = GetCurrsTimerTm();
		std::wostringstream sWosStrStream;
		sWosStrStream << std::put_time(&sTmPartsInfo, L"%Y-%m-%d %H:%M:%S");
		return sWosStrStream.str();
	}

	/**
		* @brief Gets the current time as a tm structure
		* @return A tm structure representing the current time
		* @details This function retrieves the current system time and converts it into a tm structure.
		* * The tm structure contains various components of the time, such as year, month, day, hour, minute, and second.
		* * This is useful for formatting or manipulating time data in a more structured way.
		* @note The tm structure is in local time, so it will reflect the local timezone settings of the system.
		* @retval std::tm The current time as a tm structure
		* @version 1.0.0
		*/
	std::tm GetCurrsTimerTm() const {
		auto sCurrentTime = std::chrono::system_clock::now();
		std::time_t sCurrTimerTm = std::chrono::system_clock::to_time_t(sCurrentTime);
		std::tm sCurrTmDatas;
#ifdef _WIN32
		localtime_s(&sCurrTmDatas, &sCurrTimerTm);
#else
		localtime_r(&sCurrTmDatas, &sCurrTimerTm);
#endif
		return sCurrTmDatas;
	}

private:
	//------------------------------------------------------------------------------------------------
	// Section Name: Private Members @{                                                              +
	//------------------------------------------------------------------------------------------------
	std::wofstream                  pLogFileStream;            /*!< Log file stream                  */
	std::mutex                      pLogWriteMutex;            /*!< Log write mutex                  */
	std::queue<LightLogWriteInfo>  pLogWriteQueue;             /*!< Log write queue FIFO             */
	std::condition_variable         pWrittenCondVar;           /*!< Cond for waking log write thread */
	std::thread                     sWrittenThreads;           /*!< Log write thread                 */
	std::atomic<bool>               bIsStopLogging;            /*!< Stop flag                        */
	std::wstring                    sLogLastingDir;            /*!< Directory for lasting logs       */
	std::wstring                    sLogsBasedName;            /*!< Base name for log files          */
	std::atomic<bool>               bHasLogLasting;            /*!< Whether to persist logs          */
	std::atomic<bool>               bLastingTmTags;            /*!< Current log file AM/PM tag       */
	const size_t                    kMaxQueueSize;             /*!< Max queue size                   */
	LogQueueOverflowStrategy        queueFullStrategy;         /*!< Queue full strategy              */
	std::atomic<size_t>             discardCount;              /*!< Discard count                    */
	std::atomic<size_t>             lastReportedDiscardCount;  /*!< Last reported discard count      */
	std::atomic<size_t>             reportInterval;            /*!< Report interval                  */
	//------------------------------------------------------------------------------------------------
	// @} End of Private Members                                                                     +
	//------------------------------------------------------------------------------------------------
};


#endif // !INCLUDE_LIGHTLOGWRITEIMPL_HPP_