/**
 * @file LightLogWriteImpl.h
 * @brief Lightweight logging implementation with thread-safe asynchronous writing
 * @details This file provides a thread-safe, asynchronous logging system that supports
 *          multiple string encodings (UTF-8, UTF-16, wide strings) and automatic
 *          log file rotation based on time periods. The implementation uses a
 *          producer-consumer pattern with a dedicated writer thread.
 * 
 * @author UniConv Development Team
 * @copyright Copyright (c) 2025. All rights reserved.
 * @license MIT License
 * @version 1.0.0.1
 * 
 * @par Key Features:
 * - Thread-safe asynchronous logging
 * - Multiple string encoding support (UTF-8, UTF-16, wide strings)
 * - Automatic log file rotation (AM/PM based)
 * - Producer-consumer pattern with dedicated writer thread
 * - Directory auto-creation for log files
 * - Cross-platform time handling
 * 
 * @par Dependencies:
 * - convert_tools.h: String encoding conversion utilities
 * - Standard C++ threading and filesystem libraries
 * 
 * @par Thread Safety:
 * All public methods are thread-safe using mutex synchronization.
 * 
 * @since 1.0.0.1
 */

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

//TODO : Integrate with custom UniConv library

/**
 * @brief Log entry information structure
 * @details Contains the tag and content for a single log entry.
 *          Used internally by the logging queue system.
 */
struct LightLogWrite_Info {
	std::wstring                   sLogTagNameVal; ///< Log tag/category name
	std::wstring                   sLogContentVal; ///< Log message content
};

/**
 * @brief Lightweight asynchronous logging implementation
 * @details Thread-safe logging class that provides asynchronous log writing
 *          with support for multiple string encodings and automatic file rotation.
 *          Uses a dedicated writer thread and producer-consumer pattern for
 *          high-performance logging without blocking the calling threads.
 * 
 * @par Usage Pattern:
 * 1. Create instance and optionally set log file or persistent logging
 * 2. Call WriteLogContent() from any thread to add log entries
 * 3. Destructor automatically handles cleanup and thread synchronization
 * 
 * @par Thread Safety:
 * - All public methods are thread-safe
 * - Uses mutex for queue operations and file access
 * - Condition variable for efficient thread communication
 * 
 * @par File Rotation:
 * - Automatic rotation based on AM/PM periods
 * - Files named as: basename_YYYY_MM_DD_AM/PM.log
 * - Directory structure auto-created as needed
 * 
 * @code{.cpp}
 * // Example usage
 * LightLogWrite_Impl logger;
 * logger.SetLastingsLogs("./logs", "myapp");
 * logger.WriteLogContent("INFO", "Application started");
 * logger.WriteLogContent("ERROR", "Something went wrong");
 * // Destructor handles cleanup automatically
 * @endcode
 * 
 * @since 1.0.0.1
 */
class LightLogWrite_Impl {
public:
	/**
	 * @brief Constructor - initializes logging system and starts writer thread
	 * @details Creates the asynchronous logging system with default settings.
	 *          Starts the dedicated writer thread that processes log entries
	 *          from the internal queue.
	 * 
	 * @par Initialization:
	 * - Sets up atomic flags for thread control
	 * - Starts the writer thread (RunWriteThread)
	 * - Initializes synchronization primitives
	 */
	LightLogWrite_Impl() : bIsStopLogging{ false }, bHasLogLasting{ false }

	{
		sWritedThreads = std::thread(&LightLogWrite_Impl::RunWriteThread, this);
	}

	/**
	 * @brief Destructor - cleanly shuts down logging system
	 * @details Ensures all pending log entries are written and properly
	 *          closes the writer thread and file streams.
	 * 
	 * @par Cleanup Process:
	 * - Signals writer thread to stop
	 * - Waits for all pending entries to be written
	 * - Joins writer thread
	 * - Closes file streams
	 */
	~LightLogWrite_Impl() {
		CloseLogStream();
	}

	/**
	 * @brief Set log file name (wide string version)
	 * @param sFilename Wide string path to log file
	 * @details Sets the target log file for writing. If the file doesn't exist,
	 *          it will be created. The directory structure is also created
	 *          automatically if needed.
	 * 
	 * @par Behavior:
	 * - Thread-safe operation using mutex lock
	 * - Closes any existing file stream
	 * - Creates directory structure if needed
	 * - Opens file in append mode
	 * 
	 * @note This overrides any persistent logging configuration
	 * @see ChecksDirectory()
	 */
	void SetLogsFileName(const std::wstring& sFilename) {
		std::lock_guard<std::mutex> sWriteLock(pLogWriteMutex);
		if (pLogFileStream.is_open()) pLogFileStream.close();
		ChecksDirectory(sFilename); // Ensure directory exists
		pLogFileStream.open(sFilename, std::ios::app);	}

	/**
	 * @brief Set log file name (UTF-8 string version)
	 * @param sFilename UTF-8 encoded path to log file
	 * @details Convenience overload that converts UTF-8 string to wide string
	 *          and calls the main SetLogsFileName method.
	 * 
	 * @see SetLogsFileName(const std::wstring&)
	 * @see Utf8ConvertsToUcs4()
	 */
	void SetLogsFileName(const std::string& sFilename) {
		SetLogsFileName(Utf8ConvertsToUcs4(sFilename));
	}

	/**
	 * @brief Set log file name (UTF-16 string version)
	 * @param sFilename UTF-16 encoded path to log file
	 * @details Convenience overload that converts UTF-16 string to wide string
	 *          and calls the main SetLogsFileName method.
	 * 
	 * @see SetLogsFileName(const std::wstring&)
	 * @see U16StringToWString()
	 */
	void SetLogsFileName(const std::u16string& sFilename) {
		SetLogsFileName(U16StringToWString(sFilename));
	}

	/**
	 * @brief Enable persistent logging with automatic file rotation
	 * @param sFilePath Wide string directory path for log files
	 * @param sBaseName Wide string base name for log files
	 * @details Enables persistent logging mode where log files are automatically
	 *          rotated based on time periods (AM/PM). Files are named using the
	 *          pattern: basename_YYYY_MM_DD_AM/PM.log
	 * 
	 * @par File Naming Pattern:
	 * - Format: {sBaseName}_{YYYY_MM_DD}_{AM|PM}.log
	 * - Example: myapp_2025_03_14_AM.log
	 * - Rotation occurs when crossing AM/PM boundary
	 * 
	 * @par Directory Structure:
	 * All log files are created in the specified directory path.
	 * The directory structure is created automatically if it doesn't exist.
	 * 
	 * @see BuildLogFileOut()
	 * @see CreateLogsFile()
	 */
	void SetLastingsLogs(const std::wstring& sFilePath, const std::wstring& sBaseName) {
		sLogLastingDir = sFilePath;
		sLogsBasedName = sBaseName;
		bHasLogLasting = true;
		CreateLogsFile();
	}

	/**
	 * @brief Enable persistent logging (UTF-16 version)
	 * @param sFilePath UTF-16 directory path for log files
	 * @param sBaseName UTF-16 base name for log files
	 * @details Convenience overload that converts UTF-16 strings to wide strings.
	 * 
	 * @see SetLastingsLogs(const std::wstring&, const std::wstring&)
	 */
	void SetLastingsLogs(const std::u16string& sFilePath, const std::u16string& sBaseName) {
		SetLastingsLogs(U16StringToWString(sFilePath), U16StringToWString(sBaseName));
	}

	/**
	 * @brief Enable persistent logging (UTF-8 version)
	 * @param sFilePath UTF-8 directory path for log files
	 * @param sBaseName UTF-8 base name for log files
	 * @details Convenience overload that converts UTF-8 strings to wide strings.
	 * 
	 * @see SetLastingsLogs(const std::wstring&, const std::wstring&)
	 */
	void SetLastingsLogs(const std::string& sFilePath, const std::string& sBaseName) {
		SetLastingsLogs(Utf8ConvertsToUcs4(sFilePath), Utf8ConvertsToUcs4(sBaseName));
	}

	/**
	 * @brief Write log entry to queue (wide string version)
	 * @param sTypeVal Wide string log type/tag (e.g., "INFO", "ERROR", "DEBUG")
	 * @param sMessage Wide string log message content
	 * @details Thread-safe method to add a log entry to the processing queue.
	 *          The entry is processed asynchronously by the writer thread.
	 * 
	 * @par Thread Safety:
	 * - Uses mutex lock for queue operations
	 * - Notifies writer thread via condition variable
	 * - Non-blocking for calling thread
	 * 
	 * @par Log Format:
	 * The final log entry format is: {sTypeVal}-//>>>{timestamp} : {sMessage}
	 * 
	 * @see RunWriteThread()
	 */
	void WriteLogContent(const std::wstring& sTypeVal, const std::wstring& sMessage) {
		{
			std::lock_guard<std::mutex> sWriteLock(pLogWriteMutex);
			pLogWriteQueue.push({ sTypeVal, sMessage });
		}
		pWritedCondVar.notify_one(); // Notify writer thread
	}

	/**
	 * @brief Write log entry to queue (UTF-8 version)
	 * @param sTypeVal UTF-8 log type/tag
	 * @param sMessage UTF-8 log message content
	 * @details Convenience overload that converts UTF-8 strings to wide strings.
	 * 
	 * @see WriteLogContent(const std::wstring&, const std::wstring&)
	 */
	void WriteLogContent(const std::string& sTypeVal, const std::string& sMessage)
	{
		WriteLogContent(Utf8ConvertsToUcs4(sTypeVal), Utf8ConvertsToUcs4(sMessage));
	}

	/**
	 * @brief Write log entry to queue (UTF-16 version)
	 * @param sTypeVal UTF-16 log type/tag
	 * @param sMessage UTF-16 log message content
	 * @details Convenience overload that converts UTF-16 strings to wide strings.
	 * 
	 * @see WriteLogContent(const std::wstring&, const std::wstring&)
	 */
	void WriteLogContent(const std::u16string& sTypeVal, const std::u16string& sMessage) {
		WriteLogContent(U16StringToWString(sTypeVal), U16StringToWString(sMessage));
	}
private:
	/**
	 * @defgroup LoggingInternals Internal Logging Implementation
	 * @brief Private methods and utilities for the logging system
	 * @{
	 */

	/**
	 * @brief Build log file name with timestamp
	 * @return Wide string path to the log file
	 * @details Constructs the full path for the current log file based on
	 *          the current time and persistent logging settings. File names
	 *          follow the pattern: basename_YYYY_MM_DD_AM/PM.log
	 * 
	 * @par Naming Logic:
	 * - Uses current date for YYYY_MM_DD portion
	 * - Determines AM/PM based on current hour (>12 = AM, <=12 = PM)
	 * - Updates internal bLastingTmTags for rotation detection
	 * 
	 * @par Path Construction:
	 * Combines sLogLastingDir with the generated filename
	 * 
	 * @see GetCurrsTimerTm()
	 * @see CreateLogsFile()
	 */
	std::wstring BuildLogFileOut() {
		std::tm                 sTmPartsInfo = GetCurrsTimerTm();
		std::wostringstream     sWosStrStream;

		sWosStrStream << std::put_time(&sTmPartsInfo, L"%Y_%m_%d") << (sTmPartsInfo.tm_hour > 12 ? L"_AM" : L"_PM") << L".log";

		bLastingTmTags = (sTmPartsInfo.tm_hour > 12);

		std::filesystem::path   sLotOutPaths = sLogLastingDir;
		std::filesystem::path   sLogOutFiles = sLotOutPaths / (sLogsBasedName + sWosStrStream.str());

		return sLogOutFiles.wstring();
	}

	/**
	 * @brief Clean shutdown of logging system
	 * @details Initiates the shutdown sequence for the logging system:
	 *          - Sets stop flag to signal writer thread
	 *          - Notifies all waiting threads
	 *          - Adds final log entry
	 *          - Waits for writer thread to complete
	 * 
	 * @par Shutdown Sequence:
	 * 1. Set bIsStopLogging flag to true
	 * 2. Notify condition variable to wake writer thread
	 * 3. Add shutdown log entry for debugging
	 * 4. Join writer thread (wait for completion)
	 * 
	 * @note Called automatically by destructor
	 */
	void CloseLogStream()
	{
		bIsStopLogging = true;
		pWritedCondVar.notify_all();
		WriteLogContent(L"Stop log write thread", L"===============================>");
		if (sWritedThreads.joinable()) sWritedThreads.join(); // Wait for thread to complete
	}

	/**
	 * @brief Create new log file for persistent logging
	 * @details Creates a new log file based on current timestamp and closes
	 *          any existing file stream. Used for initial setup and rotation.
	 * 
	 * @par Operations:
	 * 1. Generate new filename using BuildLogFileOut()
	 * 2. Lock mutex for thread safety
	 * 3. Ensure directory structure exists
	 * 4. Close existing file stream
	 * 5. Open new file in append mode
	 * 
	 * @see BuildLogFileOut()
	 * @see ChecksDirectory()
	 */
	void CreateLogsFile()
	{
		std::wstring  sOutFileName = BuildLogFileOut();
		std::lock_guard<std::mutex> sLock(pLogWriteMutex);
		ChecksDirectory(sOutFileName);
		pLogFileStream.close();	// Close previous file stream
		pLogFileStream.open(sOutFileName, std::ios::app);
	}

	/**
	 * @brief Main writer thread function
	 * @details Continuously processes log entries from the queue until
	 *          shutdown is requested. Handles file rotation and writing.
	 * 
	 * @par Main Loop:
	 * 1. Check for file rotation needs (AM/PM boundary)
	 * 2. Wait for log entries or shutdown signal
	 * 3. Process available log entries
	 * 4. Write formatted entries to file
	 * 5. Repeat until shutdown
	 * 
	 * @par File Rotation:
	 * Automatically detects AM/PM boundary changes and creates new log files
	 * 
	 * @par Log Format:
	 * Each entry written as: {tag}-//>>>{timestamp} : {content}
	 * 
	 * @see GetCurrentTimer()
	 * @see CreateLogsFile()
	 */
	void RunWriteThread() {
		while (true) {
			if (bHasLogLasting)
				if (bLastingTmTags != (GetCurrsTimerTm().tm_hour > 12))
					CreateLogsFile();
			LightLogWrite_Info sLogMessageInf;
			{
				auto sLock = std::unique_lock<std::mutex>(pLogWriteMutex);
				pWritedCondVar.wait(sLock, [this] {return !pLogWriteQueue.empty() || bIsStopLogging; });
				if (bIsStopLogging && pLogWriteQueue.empty()) break; // Exit if stop flag is true and queue is empty
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

	/**
	 * @brief Ensure directory structure exists for log file
	 * @param sFilename Wide string path to check/create
	 * @details Extracts the parent directory from the given file path and
	 *          creates the directory structure if it doesn't exist.
	 * 
	 * @par Directory Creation:
	 * - Uses std::filesystem::create_directories for recursive creation
	 * - Only creates directories if they don't already exist
	 * - Handles nested directory structures automatically
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
	 * @brief Get current time as formatted wide string
	 * @return Wide string formatted as "YYYY-MM-DD HH:MM:SS"
	 * @details Creates a formatted timestamp string for log entries.
	 *          Uses local time zone for timestamp generation.
	 * 
	 * @par Format:
	 * Standard format: "YYYY-MM-DD HH:MM:SS" (24-hour format)
	 * 
	 * @see GetCurrsTimerTm()
	 */
	std::wstring GetCurrentTimer() const {
		std::tm              sTmPartsInfo = GetCurrsTimerTm();
		std::wostringstream  sWosStrStream;
		sWosStrStream << std::put_time(&sTmPartsInfo, L"%Y-%m-%d %H:%M:%S");
		return	sWosStrStream.str();
	}

	/**
	 * @brief Get current time as std::tm structure
	 * @return std::tm structure with current local time
	 * @details Cross-platform function to get current local time as std::tm.
	 *          Uses platform-specific safe time conversion functions.
	 * 
	 * @par Platform Differences:
	 * - Windows: Uses localtime_s for thread safety
	 * - Unix/Linux: Uses localtime_r for thread safety
	 * 
	 * @par Usage:
	 * Used by both timestamp generation and file rotation logic
	 */
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

	/** @} */ // end of LoggingInternals group

private:
	/**
	 * @defgroup LoggingMembers Member Variables
	 * @brief Internal state and synchronization primitives
	 * @{
	 */

	std::wofstream                                 pLogFileStream;	///< Log file output stream
	std::mutex                                     pLogWriteMutex;	///< Mutex for thread-safe operations
	std::queue<LightLogWrite_Info>                 pLogWriteQueue;	///< Queue for pending log entries
	std::condition_variable	                       pWritedCondVar;	///< Condition variable for thread communication

	std::thread                                    sWritedThreads;	///< Background writer thread
	std::atomic<bool>                              bIsStopLogging;	///< Atomic stop flag (default: false)
	std::wstring                                   sLogLastingDir;	///< Directory path for persistent logs
	std::wstring                                   sLogsBasedName; ///< Base name for persistent log files
	std::atomic<bool>                              bHasLogLasting;	///< Enable persistent logging flag (default: false)
	std::atomic<bool>                              bLastingTmTags;	///< Current time period flag (AM/PM tracking)

	/** @} */ // end of LoggingMembers group
};
