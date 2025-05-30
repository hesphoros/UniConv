#pragma once
#include <iostream>
#include <string>
#include <fstream>
#include <mutex>
#include <chrono>
#include <iomanip>

class Logger {
private:
    static std::mutex log_mutex;
    static std::ofstream log_file;
    static bool file_initialized;

public:
    static void Log(const std::string& message) {
        std::lock_guard<std::mutex> lock(log_mutex);
        
        if (!file_initialized) {
            log_file.open("log/TestLog.log", std::ios::app);
            file_initialized = true;
        }
        
        // 获取当前时间
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        
        // 输出到控制台
        std::cout << "[" << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") << "] " << message << std::endl;
        
        // 输出到文件
        if (log_file.is_open()) {
            log_file << "[" << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") << "] " << message << std::endl;
            log_file.flush();
        }
    }
    
    static void Close() {
        std::lock_guard<std::mutex> lock(log_mutex);
        if (log_file.is_open()) {
            log_file.close();
        }
    }
};

// 静态成员定义
std::mutex Logger::log_mutex;
std::ofstream Logger::log_file;
bool Logger::file_initialized = false;
