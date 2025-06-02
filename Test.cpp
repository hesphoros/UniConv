#if _MSC_VER >= 1600 
#pragma execution_character_set("utf-8")
#endif

#include "UniConv.h"

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


// �򻯵���־����
void Log(const std::string& message) {
    glogger.WriteLogContent("INFO", message);
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
void GenerateTestFiles() {
    // ��������Ŀ¼
    CreateDirectories("testdata");
    
    // �����ı������ģ�
    std::string test_text = "�������Hello World����123";
    
    // 1. ����UTF-8�ļ�����BOM��
    WriteFileBytes("testdata/input_utf8.txt", test_text);    // 2. ����GBK/GB2312�ļ�
    auto conv = UniConv::GetInstance();
    auto gbk_result = conv->ConvertEncoding(test_text, "UTF-8", "GBK");
    if (gbk_result.IsSuccess()) {
        WriteFileBytes("testdata/input_gbk.txt", gbk_result.conv_result_str);
    }
      // 3. ����UTF-16LE�ļ�����BOM��
    auto utf16le_result = conv->ConvertEncoding(test_text, "UTF-8", "UTF-16LE");
    if (utf16le_result.IsSuccess()) {
        std::string utf16le_with_bom = "\xFF\xFE" + utf16le_result.conv_result_str;
        WriteFileBytes("testdata/input_utf16le.txt", utf16le_with_bom);
    }
      // 4. ����UTF-16BE�ļ�����BOM��
    auto utf16be_result = conv->ConvertEncoding(test_text, "UTF-8", "UTF-16BE");
    if (utf16be_result.IsSuccess()) {
        std::string utf16be_with_bom = "\xFE\xFF" + utf16be_result.conv_result_str;
        WriteFileBytes("testdata/input_utf16be.txt", utf16be_with_bom);
    }
      // 5. ���ɱ��ر����ļ���GB2312��
    auto local_result = conv->ConvertEncoding(test_text, "UTF-8", "GB2312");
    if (local_result.IsSuccess()) {
        WriteFileBytes("testdata/input_local.txt", local_result.conv_result_str);
    }
    
    Log("�����ļ��������");
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
	system("chcp 65001"); // ���ÿ���̨����ΪUTF-8
	system("cls"); // ����

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
    std::string test_text = "�����ı�Hello World 123";
    
    Log("ԭʼ�����ı���" + test_text);
    Log("ϵͳ���룺" + conv->GetCurrentSystemEncoding());
    
    // ���� UTF-8 <-> ���ر���
    {
        auto local_result = conv->FromUtf8ToLocal(test_text);
        Log("UTF-8 -> Local: " + BytesToHex(local_result));
        
        auto utf8_result = conv->ToUtf8FromLocal(local_result);
        Log("Local -> UTF-8: " + utf8_result);
        Log("����ת���ɹ�: " + std::string(utf8_result == test_text ? "��" : "��"));
    }
    
    // ���� UTF-8 <-> UTF-16LE
    {
        auto utf16le_result = conv->FromUtf8ToUtf16LE(test_text);
        Log("UTF-8 -> UTF-16LE: " + BytesToHex(std::string(reinterpret_cast<const char*>(utf16le_result.data()), utf16le_result.size() * 2)));
        
        auto utf8_result = conv->FromUtf16LEToUtf8(utf16le_result);
        Log("UTF-16LE -> UTF-8: " + utf8_result);
        Log("����ת���ɹ�: " + std::string(utf8_result == test_text ? "��" : "��"));
    }
    
    // ���� UTF-8 <-> UTF-16BE
    {
        auto utf16be_result = conv->FromUtf8ToUtf16BE(test_text);
        Log("UTF-8 -> UTF-16BE: " + BytesToHex(std::string(reinterpret_cast<const char*>(utf16be_result.data()), utf16be_result.size() * 2)));
        
        auto utf8_result = conv->FromUtf16BEToUtf8(utf16be_result);
        Log("UTF-16BE -> UTF-8: " + utf8_result);
        Log("����ת���ɹ�: " + std::string(utf8_result == test_text ? "��" : "��"));
    }
    
    Log("=== ���б���ת������������� ===");
}

// �����Ժ���
void RunAllTests() {
    
    // ���ɲ����ļ�
    GenerateTestFiles();
    
    // ��������ת������
    TestAllConversions();
    
    // ����ת���ļ�
    BatchConvertFiles();
}
