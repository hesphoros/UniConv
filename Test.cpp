// Test.cpp - �ع��Ĳ����ļ�
#include "UniConv.h"
#include "LightLogWriteImpl.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <io.h>
#include <fcntl.h>
#include <vector>
#include <direct.h>
#include <utility>

// ȫ����־ʵ��
static LightLogWrite_Impl g_logger;

// ��ʼ����־ϵͳ
void InitLogger() {
    g_logger.SetLogsFileName("log/test_log.txt");
}

// �򻯵���־����
void Log(const std::string& message) {
    g_logger.WriteLogContent("INFO", message);
    std::cout << message << std::endl;
}

// �������������ֽ�����ת��Ϊʮ�������ַ���
std::string BytesToHex(const std::string& data) {
    std::ostringstream oss;
    for (unsigned char c : data) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c) << " ";
    }
    return oss.str();
}

// ������������ȡ�ļ���ԭʼ�ֽ�����
std::string ReadFileBytes(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        return "";
    }
    return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

// ����������д���ֽ����ݵ��ļ�
bool WriteFileBytes(const std::string& filePath, const std::string& data) {
    std::ofstream file(filePath, std::ios::binary);
    if (!file) {
        return false;
    }
    file.write(data.data(), data.size());
    return file.good();
}

// ����Ŀ¼�ĸ�������
void CreateDirectories(const std::string& path) {
    _mkdir("testdata");
    _mkdir("testdata\\output");
}

// ���ɲ��������ļ�
// ׼�����Ի�����ֻ����Ŀ¼�����޸��ļ���
void PrepareTestEnvironment() {
    // ��������Ŀ¼
    CreateDirectories("testdata");
    
    // �������ļ��Ƿ����
    std::vector<std::string> required_files = {
        "testdata/input_utf8.txt",
        "testdata/input_gbk.txt", 
        "testdata/input_utf16le.txt",
        "testdata/input_utf16be.txt"
    };
    
    bool all_files_exist = true;
    for (const auto& file : required_files) {
        std::ifstream test_file(file);
        if (!test_file.good()) {
            Log("���棺ȱ�ٲ����ļ� " + file);
            all_files_exist = false;
        }
    }
    
    if (all_files_exist) {
        Log("���в����ļ��Ѵ��ڣ�׼����ʼ����");
    } else {
        Log("��ȷ�����б���Ĳ����ļ�������");
        Log("�������� generate_test_files.py �ű������ɲ����ļ�");
    }
}

// ����ļ����벢ȥ��BOM
std::pair<std::string, std::string> DetectEncodingAndRemoveBOM(const std::string& data) {
    if (data.empty()) {
        return std::make_pair("UTF-8", data);
    }
    
    // ���BOM
    if (data.size() >= 3 && 
        static_cast<unsigned char>(data[0]) == 0xEF && 
        static_cast<unsigned char>(data[1]) == 0xBB && 
        static_cast<unsigned char>(data[2]) == 0xBF) {
        return std::make_pair("UTF-8", data.substr(3));
    }
    
    if (data.size() >= 2 && 
        static_cast<unsigned char>(data[0]) == 0xFF && 
        static_cast<unsigned char>(data[1]) == 0xFE) {
        return std::make_pair("UTF-16LE", data.substr(2));
    }
    
    if (data.size() >= 2 && 
        static_cast<unsigned char>(data[0]) == 0xFE && 
        static_cast<unsigned char>(data[1]) == 0xFF) {
        return std::make_pair("UTF-16BE", data.substr(2));
    }
    
    // û��BOM������ԭ����
    return std::make_pair("", data);
}

// ����ת���ļ�����ʵ��
void BatchConvertFiles() {
    Log("=== ��ʼ�����ļ�ת������ ===");
    
    auto conv = UniConv::GetInstance();
    
    // ת�����ã�Դ�ļ� -> Ŀ�����
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
        Log("--- " + task.description + " ---");
        
        // ��ȡ�����ļ�
        std::string input_data = ReadFileBytes(task.inputFile);
        if (input_data.empty()) {
            Log("�����޷���ȡ�ļ� " + task.inputFile);
            continue;
        }
        
        // ��Ⲣȥ��BOM
        auto result_pair = DetectEncodingAndRemoveBOM(input_data);
        std::string detected_encoding = result_pair.first;
        std::string clean_data = result_pair.second;
        std::string actual_from_encoding = detected_encoding.empty() ? task.fromEncoding : detected_encoding;
        
        Log("�����ļ���" + task.inputFile);
        Log("ԭʼ���ݴ�С��" + std::to_string(input_data.size()) + " �ֽ�");
        Log("��⵽�ı��룺" + (detected_encoding.empty() ? "��BOM" : detected_encoding));
        Log("��������ݴ�С��" + std::to_string(clean_data.size()) + " �ֽ�");
        Log("��������ʮ�����ƣ�" + BytesToHex(clean_data));
        
        // ִ�б���ת��
        auto result = conv->ConvertEncoding(clean_data, actual_from_encoding.c_str(), task.toEncoding.c_str());
        
        if (result.IsSuccess()) {
            Log("ת���ɹ���");
            Log("������ݴ�С��" + std::to_string(result.conv_result_str.size()) + " �ֽ�");
            Log("�������ʮ�����ƣ�" + BytesToHex(result.conv_result_str));
              // д������ļ�
            if (WriteFileBytes(task.outputFile, result.conv_result_str)) {
                Log("�ɹ�д������ļ���" + task.outputFile);
            } else {
                Log("�����޷�д������ļ���" + task.outputFile);
            }
        } else {
            Log("ת��ʧ�ܣ�" + result.error_msg);
        }
        
        Log("");
    }
    
    Log("=== �����ļ�ת��������� ===");
}

// �������б���ת������
void TestAllConversions() {
    Log("=== ��ʼ�������б���ת������ ===");
    
    auto conv = UniConv::GetInstance();
    
    // ��UTF-8�����ļ���ȡʵ�ʵ�UTF-8��������
    std::string utf8_file_data = ReadFileBytes("testdata/input_utf8.txt");
    if (utf8_file_data.empty()) {
        Log("�����޷���ȡUTF-8�����ļ�����������ת������");
        return;
    }
    
    // ȥ��BOM������еĻ���
    auto result_pair = DetectEncodingAndRemoveBOM(utf8_file_data);
    std::string test_text = result_pair.second;
    
    Log("��UTF-8�ļ���ȡ�����ı���С��" + std::to_string(test_text.size()) + " �ֽ�");
    Log("UTF-8����ʮ�����ƣ�" + BytesToHex(test_text));
    Log("ϵͳ���룺" + conv->GetCurrentSystemEncoding());
    
    // ���� UTF-8 <-> ���ر���
    {
        Log("--- ���� UTF-8 <-> ���ر��� ---");
        auto local_result = conv->FromUtf8ToLocal(test_text);
        Log("UTF-8 -> Local: " + BytesToHex(local_result));
        
        auto utf8_result = conv->ToUtf8FromLocal(local_result);
        Log("Local -> UTF-8: " + BytesToHex(utf8_result));
        
        bool success = (utf8_result == test_text);
        Log("����ת���ɹ�: " + std::string(success ? "��" : "��"));
        if (!success) {
            Log("ԭʼ��С: " + std::to_string(test_text.size()) + ", �����С: " + std::to_string(utf8_result.size()));
        }
    }
    
    // ���� UTF-8 <-> UTF-16LE
    {
        Log("--- ���� UTF-8 <-> UTF-16LE ---");
        auto utf16le_result = conv->FromUtf8ToUtf16LE(test_text);
        Log("UTF-8 -> UTF-16LE: " + BytesToHex(std::string(reinterpret_cast<const char*>(utf16le_result.data()), utf16le_result.size() * 2)));
        
        auto utf8_result = conv->FromUtf16LEToUtf8(utf16le_result);
        Log("UTF-16LE -> UTF-8: " + BytesToHex(utf8_result));
        
        bool success = (utf8_result == test_text);
        Log("����ת���ɹ�: " + std::string(success ? "��" : "��"));
        if (!success) {
            Log("ԭʼ��С: " + std::to_string(test_text.size()) + ", �����С: " + std::to_string(utf8_result.size()));
        }
    }
    
    // ���� UTF-8 <-> UTF-16BE
    {
        Log("--- ���� UTF-8 <-> UTF-16BE ---");
        auto utf16be_result = conv->FromUtf8ToUtf16BE(test_text);
        Log("UTF-8 -> UTF-16BE: " + BytesToHex(std::string(reinterpret_cast<const char*>(utf16be_result.data()), utf16be_result.size() * 2)));
        
        auto utf8_result = conv->FromUtf16BEToUtf8(utf16be_result);
        Log("UTF-16BE -> UTF-8: " + BytesToHex(utf8_result));
        
        bool success = (utf8_result == test_text);
        Log("����ת���ɹ�: " + std::string(success ? "��" : "��"));
        if (!success) {
            Log("ԭʼ��С: " + std::to_string(test_text.size()) + ", �����С: " + std::to_string(utf8_result.size()));
        }
    }
    
    Log("=== ���б���ת������������� ===");
}

// �����Ժ���
void RunAllTests() {
    // ��ʼ����־ϵͳ
    InitLogger();
    
    // ���ɲ����ļ�
    PrepareTestEnvironment();
    
    // ��������ת������
    TestAllConversions();
    
    // ����ת���ļ�
    BatchConvertFiles();
}
