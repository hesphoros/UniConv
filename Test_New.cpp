// Test_New.cpp - 重构的测试文件
#include "UniConv.h"
#include "LightLogWriteImpl.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <filesystem>

// 全局日志实例
static LightLogWrite_Impl g_logger;

// 简化的日志函数
void Log(const std::string& message) {
    g_logger.WriteLogContent(L"INFO", Utf8ConvertsToUcs4(message));
    std::cout << message << std::endl;
}

// 辅助函数：将字节数据转换为十六进制字符串
std::string BytesToHex(const std::string& data) {
    std::ostringstream oss;
    for (unsigned char c : data) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c) << " ";
    }
    return oss.str();
}

// 辅助函数：读取文件的原始字节数据
std::string ReadFileBytes(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        return "";
    }
    return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

// 辅助函数：写入字节数据到文件
bool WriteFileBytes(const std::string& filePath, const std::string& data) {
    std::ofstream file(filePath, std::ios::binary);
    if (!file) {
        return false;
    }
    file.write(data.data(), data.size());
    return file.good();
}

// 准备测试环境（只创建目录，不修改文件）
void PrepareTestEnvironment() {
    // 创建测试目录
    std::filesystem::create_directories("testdata");
    std::filesystem::create_directories("testdata/output");
    
    // 检查测试文件是否存在
    std::vector<std::string> required_files = {
        "testdata/input_utf8.txt",
        "testdata/input_gbk.txt", 
        "testdata/input_utf16le.txt",
        "testdata/input_utf16be.txt"
    };
    
    bool all_files_exist = true;
    for (const auto& file : required_files) {
        if (!std::filesystem::exists(file)) {
            Log("警告：缺少测试文件 " + file);
            all_files_exist = false;
        }
    }
    
    if (all_files_exist) {
        Log("所有测试文件已存在，准备开始测试");
    } else {
        Log("请确保所有必需的测试文件都存在");
        Log("建议运行 generate_test_files.py 脚本来生成测试文件");
    }
}

// 检测文件编码并去除BOM
std::pair<std::string, std::string> DetectEncodingAndRemoveBOM(const std::string& data) {
    if (data.empty()) {
        return {"UTF-8", data};
    }
    
    // 检测BOM
    if (data.size() >= 3 && 
        static_cast<unsigned char>(data[0]) == 0xEF && 
        static_cast<unsigned char>(data[1]) == 0xBB && 
        static_cast<unsigned char>(data[2]) == 0xBF) {
        return {"UTF-8", data.substr(3)};
    }
    
    if (data.size() >= 2 && 
        static_cast<unsigned char>(data[0]) == 0xFF && 
        static_cast<unsigned char>(data[1]) == 0xFE) {
        return {"UTF-16LE", data.substr(2)};
    }
    
    if (data.size() >= 2 && 
        static_cast<unsigned char>(data[0]) == 0xFE && 
        static_cast<unsigned char>(data[1]) == 0xFF) {
        return {"UTF-16BE", data.substr(2)};
    }
    
    // 没有BOM，返回原数据
    return {"", data};
}

// 批量转换文件的新实现
void BatchConvertFiles() {
    LightLogWriteImpl::GetInstance().Log("=== 开始批量文件转换测试 ===");
    
    auto& conv = UniConv::GetInstance();
    
    // 转换配置：源文件 -> 目标编码
    struct ConversionTask {
        std::string inputFile;
        std::string outputFile;
        std::string fromEncoding;
        std::string toEncoding;
        std::string description;
    };
    
    std::vector<ConversionTask> tasks = {
        {"testdata/input_utf8.txt", "testdata/output/output_utf16le.txt", "UTF-8", "UTF-16LE", "UTF-8 -> UTF-16LE"},
        {"testdata/input_utf8.txt", "testdata/output/output_utf16be.txt", "UTF-8", "UTF-16BE", "UTF-8 -> UTF-16BE"},
        {"testdata/input_utf8.txt", "testdata/output/output_gbk.txt", "UTF-8", "GBK", "UTF-8 -> GBK"},
        {"testdata/input_gbk.txt", "testdata/output/output_utf8_from_gbk.txt", "GBK", "UTF-8", "GBK -> UTF-8"},
        {"testdata/input_utf16le.txt", "testdata/output/output_utf8_from_utf16le.txt", "UTF-16LE", "UTF-8", "UTF-16LE -> UTF-8"},
        {"testdata/input_utf16be.txt", "testdata/output/output_utf8_from_utf16be.txt", "UTF-16BE", "UTF-8", "UTF-16BE -> UTF-8"}
    };
    
    for (const auto& task : tasks) {
        LightLogWriteImpl::GetInstance().Log("--- " + task.description + " ---");
        
        // 读取输入文件
        std::string input_data = ReadFileBytes(task.inputFile);
        if (input_data.empty()) {
            LightLogWriteImpl::GetInstance().Log("错误：无法读取文件 " + task.inputFile);
            continue;
        }
        
        // 检测并去除BOM
        auto [detected_encoding, clean_data] = DetectEncodingAndRemoveBOM(input_data);
        std::string actual_from_encoding = detected_encoding.empty() ? task.fromEncoding : detected_encoding;
        
        LightLogWriteImpl::GetInstance().Log("输入文件：" + task.inputFile);
        LightLogWriteImpl::GetInstance().Log("原始数据大小：" + std::to_string(input_data.size()) + " 字节");
        LightLogWriteImpl::GetInstance().Log("检测到的编码：" + (detected_encoding.empty() ? "无BOM" : detected_encoding));
        LightLogWriteImpl::GetInstance().Log("清理后数据大小：" + std::to_string(clean_data.size()) + " 字节");
        LightLogWriteImpl::GetInstance().Log("输入数据十六进制：" + BytesToHex(clean_data));
        
        // 执行编码转换
        auto result = conv.ConvertEncoding(clean_data, actual_from_encoding.c_str(), task.toEncoding.c_str());
        
        if (result.IsSuccess()) {
            LightLogWriteImpl::GetInstance().Log("转换成功！");
            LightLogWriteImpl::GetInstance().Log("输出数据大小：" + std::to_string(result.conv_result_str.size()) + " 字节");
            LightLogWriteImpl::GetInstance().Log("输出数据十六进制：" + BytesToHex(result.conv_result_str));
            
            // 写入输出文件
            if (WriteFileBytes(task.outputFile, result.conv_result_str)) {
                LightLogWriteImpl::GetInstance().Log("成功写入输出文件：" + task.outputFile);
            } else {
                LightLogWriteImpl::GetInstance().Log("错误：无法写入输出文件：" + task.outputFile);
            }
        } else {
            LightLogWriteImpl::GetInstance().Log("转换失败：" + result.error_msg);
        }
        
        LightLogWriteImpl::GetInstance().Log("");
    }
    
    LightLogWriteImpl::GetInstance().Log("=== 批量文件转换测试完成 ===");
}

// 测试所有编码转换方法
void TestAllConversions() {
    LightLogWriteImpl::GetInstance().Log("=== 开始测试所有编码转换方法 ===");
    
    auto& conv = UniConv::GetInstance();
    
    // 从实际的 UTF-8 文件读取测试文本，确保是真正的 UTF-8 编码
    std::string test_text_utf8;
    if (std::filesystem::exists("testdata/input_utf8.txt")) {
        test_text_utf8 = ReadFileBytes("testdata/input_utf8.txt");
    } else {
        // 如果文件不存在，使用简单的ASCII文本
        test_text_utf8 = "Hello World 123";
    }
    
    LightLogWriteImpl::GetInstance().Log("原始测试文本（UTF-8）：" + test_text_utf8);
    LightLogWriteImpl::GetInstance().Log("系统编码：" + conv.GetCurrentSystemEncoding());
    
    // 测试 UTF-8 <-> 本地编码
    {
        auto local_result = conv.FromUtf8ToLocal(test_text);
        LightLogWriteImpl::GetInstance().Log("UTF-8 -> Local: " + BytesToHex(local_result));
        
        auto utf8_result = conv.ToUtf8FromLocal(local_result);
        LightLogWriteImpl::GetInstance().Log("Local -> UTF-8: " + utf8_result);
        LightLogWriteImpl::GetInstance().Log("往返转换成功: " + std::string(utf8_result == test_text ? "是" : "否"));
    }
    
    // 测试 UTF-8 <-> UTF-16LE
    {
        auto utf16le_result = conv.FromUtf8ToUtf16LE(test_text);
        LightLogWriteImpl::GetInstance().Log("UTF-8 -> UTF-16LE: " + BytesToHex(std::string(reinterpret_cast<const char*>(utf16le_result.data()), utf16le_result.size() * 2)));
        
        auto utf8_result = conv.FromUtf16LEToUtf8(utf16le_result);
        LightLogWriteImpl::GetInstance().Log("UTF-16LE -> UTF-8: " + utf8_result);
        LightLogWriteImpl::GetInstance().Log("往返转换成功: " + std::string(utf8_result == test_text ? "是" : "否"));
    }
    
    // 测试 UTF-8 <-> UTF-16BE
    {
        auto utf16be_result = conv.FromUtf8ToUtf16BE(test_text);
        LightLogWriteImpl::GetInstance().Log("UTF-8 -> UTF-16BE: " + BytesToHex(std::string(reinterpret_cast<const char*>(utf16be_result.data()), utf16be_result.size() * 2)));
        
        auto utf8_result = conv.FromUtf16BEToUtf8(utf16be_result);
        LightLogWriteImpl::GetInstance().Log("UTF-16BE -> UTF-8: " + utf8_result);
        LightLogWriteImpl::GetInstance().Log("往返转换成功: " + std::string(utf8_result == test_text ? "是" : "否"));
    }
    
    LightLogWriteImpl::GetInstance().Log("=== 所有编码转换方法测试完成 ===");
}

// 主测试函数
void RunAllTests() {    // 设置日志文件
    LightLogWriteImpl::GetInstance().SetLogFileName("TestNewConvert.log");
    
    // 准备测试环境（只创建目录，不修改文件）
    PrepareTestEnvironment();
    
    // 测试所有转换方法
    TestAllConversions();
    
    // 批量转换文件
    BatchConvertFiles();
}
