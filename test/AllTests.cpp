#if _MSC_VER >= 1600 
#pragma execution_character_set("utf-8")
#endif

#include "UniConv.h"
#include "common.h"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <windows.h>
#include <assert.h>
#include <io.h>
#include <fcntl.h>
#include <vector>
#include <direct.h>
#include <utility>

auto g_conv = UniConv::GetInstance();

// Conversion task structure
struct ConversionTask {
    std::string inputFile;
    std::string outputFile;
    std::string fromEncoding;
    std::string toEncoding;
    std::string description;
};

// BOM encoding enumeration
enum class BomEncoding {
    None,
    UTF8,
    UTF16_LE,
    UTF16_BE,
    UTF32_LE,
    UTF32_BE
};

// Encoding string constants
std::string UTF8    = UniConv::ToString(UniConv::Encoding::utf_8);
std::string GBK     = UniConv::ToString(UniConv::Encoding::gbk);
std::string UTF16LE = UniConv::ToString(UniConv::Encoding::utf_16le);
std::string UTF16BE = UniConv::ToString(UniConv::Encoding::utf_16be);

// Test task configuration
ConversionTask task_gb2312_to_utf8 = {
    "testdata/input_gb2312.txt",
    "testdata/output/output_gb2312_to_utf-8.txt",
    "GBK",
    "UTF-8",
    "GBK -> UTF-8"
};

std::vector<ConversionTask> tasks = {
    task_gb2312_to_utf8
};

// ========== Utility Functions ==========

// Helper functions for test data handling
std::string ReadFileBytes(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        std::cout << "[DEBUG] Failed to open file: " << filePath << std::endl;
        return "";
    }
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    std::cout << "[DEBUG] Successfully read file: " << filePath << " (" << content.size() << " bytes)" << std::endl;
    return content;
}

bool WriteFileBytes(const std::string& filePath, const std::string& data) {
    std::ofstream file(filePath, std::ios::binary);
    if (!file) {
        return false;
    }
    file.write(data.data(), data.size());
    return file.good();
}

// Convert the byte data to a hexadecimal string
std::string BytesToHex(const std::string& data) {
    std::ostringstream oss;
    for (unsigned char c : data) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c) << " ";
    }
    return oss.str();
}

// Create necessary directories
void CreateDirectories(const std::string& path) {
    _mkdir("testdata");
    _mkdir("testdata\\output");
}

// Check file encoding and remove BOM if exists
std::pair<std::string, std::string> DetectEncodingAndRemoveBOM(const std::string& data) {
    if (data.empty()) {
        return std::make_pair("UTF-8", data);
    }

    // detect bom
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

    return std::make_pair("", data);
}

std::pair<BomEncoding, std::string_view> RemoveBOM(const std::string_view& data) {
    const unsigned char* bytes = reinterpret_cast<const unsigned char*>(data.data());
    size_t len = data.size();

    if (len >= 3 && bytes[0] == 0xEF && bytes[1] == 0xBB && bytes[2] == 0xBF)
        return { BomEncoding::UTF8, data.substr(3) };
    if (len >= 2 && bytes[0] == 0xFF && bytes[1] == 0xFE)
        return { BomEncoding::UTF16_LE, data.substr(2) };
    if (len >= 2 && bytes[0] == 0xFE && bytes[1] == 0xFF)
        return { BomEncoding::UTF16_BE, data.substr(2) };
    if (len >= 4 && bytes[0] == 0xFF && bytes[1] == 0xFE && bytes[2] == 0x00 && bytes[3] == 0x00)
        return { BomEncoding::UTF32_LE, data.substr(4) };
    if (len >= 4 && bytes[0] == 0x00 && bytes[1] == 0x00 && bytes[2] == 0xFE && bytes[3] == 0xFF)
        return { BomEncoding::UTF32_BE, data.substr(4) };

    return { BomEncoding::None, data };
}

void InitializeLogging() {
    glogger.SetLogsFileName("log/test_log.log");
}

// ========== Test Functions from main.cpp ==========

/// <summary>
/// Test GetCurrentSystemEncoding API
/// </summary>
void TestGetCurrentSystemEncoding() {
    std::string sCurSysEnc = g_conv->GetCurrentSystemEncoding();
    LOGINFO("Current system encoding:\t" + sCurSysEnc);
}

/// <summary>
/// Test GetCurrentSystemEncodingCodePage API
/// </summary>
void TestGetCurrentSystemEncodingCodePage() {
    int nCurSysEncCodePage = g_conv->GetCurrentSystemEncodingCodePage();
    LOGINFO("Current system codepage:\t" + std::to_string(nCurSysEncCodePage));
}

/// <summary>
/// Test API GetEncodingNameByCodePage
/// </summary>
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

void TestTostring() {
    std::string utf8 = "UTF-8";
    std::string gbk = "GBK";
    std::string uni_utf8 = UniConv::ToString(UniConv::Encoding::utf_8);
    std::string uni_gbk = UniConv::ToString(UniConv::Encoding::gbk);

    if (uni_utf8 == utf8 && uni_gbk == gbk) {
        LOGOK("UniConv::ToString() works correctly for UTF-8 and GBK.");
    } else {
        LOGERROR("UniConv::ToString() failed for UTF-8 or GBK.");
    }
}

/// <summary>
/// Test API ToLocaleFromUtf8
/// </summary>
void TestToLocaleFromUtf8() {
    std::string inputFile = "testdata/input_utf8.txt";
    std::string outputFile = "testdata/output/output_utf-8_to_local.txt";
    std::string content = ReadFileBytes(inputFile);

    std::string convert = g_conv->ToLocaleFromUtf8(content);
    if (convert.empty()) {
        LOGERROR("Conversion failed for file: " + inputFile);
        std::cout << "Conversion failed for file: " + inputFile << std::endl;
        return;
    }

    WriteFileBytes(outputFile, convert);
    std::cout << "Converted content written to: " + outputFile << std::endl;
    std::cout << content << std::endl;
    std::cout << convert << std::endl;
}

/// <summary>
/// Test API ToLocaleFromUtf8 with BOM
/// </summary>
void TestToLocaleFromUtf8WithBOM() {
    std::string inputFile = "testdata/input_utf8_bom.txt";
    std::string outputFile = "testdata/output/output_utf-8_bom_to_local.txt";
    std::string content = ReadFileBytes(inputFile);

    auto [encoding, content_view] = RemoveBOM(content);
    if (encoding != BomEncoding::UTF8) {
        LOGERROR("Expected UTF-8 encoding with BOM, but got: " + std::to_string(static_cast<int>(encoding)) + " FILE: " + inputFile);
        std::cout << "Expected UTF-8 encoding with BOM, but got: " + std::to_string(static_cast<int>(encoding)) << std::endl;
        return;
    } else if (encoding == BomEncoding::UTF8) {
        LOGOK("Encoding detected as UTF-8 with BOM. FILE: " + inputFile);
        std::cout << "Encoding detected as UTF-8 with BOM. FILE: " << inputFile << std::endl;
    }

    std::string cleaned_content(content_view.data(), content_view.size());
    auto convert = g_conv->ToLocaleFromUtf8(cleaned_content);
    if (convert.empty()) {
        LOGERROR("Conversion failed for file: " + inputFile);
        std::cout << "Conversion failed for file: " + inputFile << std::endl;
        return;
    }

    WriteFileBytes(outputFile, convert);
    std::cout << "Converted content written to: " + outputFile << std::endl;
}

/// <summary>
/// Test API ToUtf8FromLocale
/// </summary>
void TestToUtf8FromLocale() {
    std::string inputFile = "testdata/input_gb2312.txt";
    std::string outputFile = "testdata/output/output_gb2312_to_utf-8_2.txt";
    std::string content = ReadFileBytes(inputFile);

    std::string convert = g_conv->ToUtf8FromLocale(content);
    if (convert.empty()) {
        LOGERROR("Conversion failed for file: " + inputFile);
        std::cout << "Conversion failed for file: " + inputFile << std::endl;
        return;
    }
    WriteFileBytes(outputFile, convert);

    std::cout << content << std::endl;
    std::cout << convert << std::endl;
    LOGINFO("Converted content: " + convert);
}

/// <summary>
/// Test API ToUtf16LEFromLocale
/// </summary>
void TestToUtf16LEFromLocale() {
    std::string inputFile = "testdata/input_gb2312.txt";
    std::string outputFile = "testdata/output/output_gb2312_to_utf-16le.txt";
    std::string content = ReadFileBytes(inputFile);

    std::u16string convert = g_conv->ToUtf16LEFromLocale(content);
    if (convert.empty()) {
        LOGERROR("Conversion failed for file: " + inputFile);
        std::cout << "Conversion failed for file: " + inputFile << std::endl;
        return;
    }

    WriteFileBytes(outputFile, std::string(reinterpret_cast<const char*>(convert.data()), convert.size() * sizeof(char16_t)));
    std::cout << "Converted content written to: " + outputFile << std::endl;
    std::cout << content << std::endl;
}

/// <summary>
/// Test API ToUtf16BEFromLocale
/// </summary>
void TestToUtf16BEFromLocale() {
    std::string inputFile = "testdata/input_gb2312.txt";
    std::string outputFile = "testdata/output/output_gb2312_to_utf-16be.txt";
    std::string content = ReadFileBytes(inputFile);
    std::u16string convert = g_conv->ToUtf16BEFromLocale(content);
    if (convert.empty()) {
        LOGERROR("Conversion failed for file: " + inputFile);
        std::cout << "Conversion failed for file: " + inputFile << std::endl;
        return;
    }
    WriteFileBytes(outputFile, std::string(reinterpret_cast<const char*>(convert.data()), convert.size() * sizeof(char16_t)));
    std::cout << "Converted content written to: " + outputFile << std::endl;
    std::cout << content << std::endl;
}

/// <summary>
/// Test API ToLocaleFromUtf16BE
/// </summary>
void TestToLocaleFromUtf16BE() {
    std::string inputFile = "testdata/input_utf16be_nobom.txt";
    std::string outputFile = "testdata/output/output_utf-16be_to_local.txt";
    std::string content = ReadFileBytes(inputFile);
    std::u16string utf16be_content(reinterpret_cast<const char16_t*>(content.data()), content.size() / sizeof(char16_t));

    auto convert = g_conv->ToLocaleFromUtf16BE(utf16be_content);
    if (convert.empty()) {
        LOGERROR("Conversion failed for file: " + inputFile);
        std::cout << "Conversion failed for file: " + inputFile << std::endl;
        return;
    }
    WriteFileBytes(outputFile, convert);
    std::cout << "Converted content written to: " + outputFile << std::endl;
}

/// <summary>
/// Test API ToLocaleFromUtf16BE with BOM
/// </summary>
void TestToLocaleFromUtf16BE_WithBOM() {
    std::string inputFile = "testdata/input_utf16be.txt";
    std::string outputFile = "testdata/output/output_utf-16be_to_local_with_bom.txt";
    std::string content = ReadFileBytes(inputFile);
    auto [encoding, content_view] = RemoveBOM(content);
    if (encoding != BomEncoding::UTF16_BE) {
        LOGERROR("Expected UTF-16BE encoding with BOM, but got: " + std::to_string(static_cast<int>(encoding)) + "FILE : " + inputFile);
        std::cout << "Expected UTF-16BE encoding with BOM, but got: " + std::to_string(static_cast<int>(encoding)) << std::endl;
        return;
    } else if (encoding == BomEncoding::UTF16_BE) {
        LOGOK("Encoding detected as UTF-16BE with BOM. FILE: " + inputFile);
        std::cout << "Encoding detected as UTF-16BE with BOM. FILE: " << inputFile << std::endl;
    }

    std::string cleaned_content(content_view.data(), content_view.size());
    std::u16string u16_cleaned_content(reinterpret_cast<const char16_t*>(cleaned_content.data()), cleaned_content.size() / sizeof(char16_t));

    auto convert = g_conv->ToLocaleFromUtf16BE(u16_cleaned_content);
    if (convert.empty()) {
        LOGERROR("Conversion failed for file: " + inputFile);
        std::cout << "Conversion failed for file: " + inputFile << std::endl;
        return;
    }
    WriteFileBytes(outputFile, convert);
    std::cout << "Converted content written to: " + outputFile << std::endl;
}

/// <summary>
/// Test API ToLocalFromUtf16LE
/// </summary>
void TestToLocaleFromUtf16LE() {
    std::string inputFile = "testdata/input_utf16le_nobom.txt";
    std::string outputFile = "testdata/output/output_utf-16le_to_local.txt";
    std::string content = ReadFileBytes(inputFile);
    std::u16string utf16le_content(reinterpret_cast<const char16_t*>(content.data()), content.size() / sizeof(char16_t));

    auto convert = g_conv->ToLocaleFromUtf16LE(utf16le_content);
    if (convert.empty()) {
        LOGERROR("Conversion failed for file: " + inputFile);
        std::cout << "Conversion failed for file: " + inputFile << std::endl;
        return;
    }
    WriteFileBytes(outputFile, convert);
    std::cout << "Converted content written to: " + outputFile << std::endl;
}

/// <summary>
/// Test API ToLocaleFromUtf16LE with BOM
/// </summary>
void TestToLocaleFromUtf16LEWithBOM() {
    std::string inputFile = "testdata/input_utf16le.txt";
    std::string outputFile = "testdata/output/output_utf-16le_to_local_with_bom.txt";
    std::string content = ReadFileBytes(inputFile);
    auto [encoding, content_view] = RemoveBOM(content);
    if (encoding != BomEncoding::UTF16_LE) {
        LOGERROR("Expected UTF-16LE encoding with BOM, but got: " + std::to_string(static_cast<int>(encoding)) + " FILE: " + inputFile);
        std::cout << "Expected UTF-16LE encoding with BOM, but got: " + std::to_string(static_cast<int>(encoding)) << std::endl;
        return;
    } else if (encoding == BomEncoding::UTF16_LE) {
        LOGOK("Encoding detected as UTF-16LE with BOM. FILE: " + inputFile);
        std::cout << "Encoding detected as UTF-16LE with BOM. FILE: " << inputFile << std::endl;
    }

    std::string cleaned_content(content_view.data(), content_view.size());
    std::u16string u16_cleaned_content(reinterpret_cast<const char16_t*>(cleaned_content.data()), cleaned_content.size() / sizeof(char16_t));

    auto convert = g_conv->ToLocaleFromUtf16LE(u16_cleaned_content);
    if (convert.empty()) {
        LOGERROR("Conversion failed for file: " + inputFile);
        std::cout << "Conversion failed for file: " + inputFile << std::endl;
        return;
    }

    WriteFileBytes(outputFile, convert);
    std::cout << "Converted content written to: " + outputFile << std::endl;
}

/// <summary>
/// Test API ToUtf8FromUtf16LE
/// </summary>
void TestToUtf8FromUtf16LE() {
    std::string inputFile = "testdata/input_utf16le_nobom.txt";
    std::string outputFile = "testdata/output/output_utf-16le_to_utf-8.txt";
    std::string content = ReadFileBytes(inputFile);
    std::u16string utf16le_content(reinterpret_cast<const char16_t*>(content.data()), content.size() / sizeof(char16_t));
    auto convert = g_conv->ToUtf8FromUtf16LE(utf16le_content);
    if (convert.empty()) {
        LOGERROR("Conversion failed for file: " + inputFile);
        std::cout << "Conversion failed for file: " + inputFile << std::endl;
        return;
    }
    WriteFileBytes(outputFile, convert);
    std::cout << "Converted content written to: " + outputFile << std::endl;
}

/// <summary>
/// Test API ToUtf8FromUtf16LE with BOM
/// </summary>
void TestToUtf8FromUtf16LEWithBOM() {
    std::string inputFile = "testdata/input_utf16le.txt";
    std::string outputFile = "testdata/output/output_utf-16le_to_utf-8_with_bom.txt";
    std::string content = ReadFileBytes(inputFile);
    auto [encoding, content_view] = RemoveBOM(content);
    if (encoding != BomEncoding::UTF16_LE) {
        LOGERROR("Expected UTF-16LE encoding with BOM, but got: " + std::to_string(static_cast<int>(encoding)) + " FILE: " + inputFile);
        std::cout << "Expected UTF-16LE encoding with BOM, but got: " + std::to_string(static_cast<int>(encoding)) << std::endl;
        return;
    } else if (encoding == BomEncoding::UTF16_LE) {
        LOGOK("Encoding detected as UTF-16LE with BOM. FILE: " + inputFile);
        std::cout << "Encoding detected as UTF-16LE with BOM. FILE: " << inputFile << std::endl;
    }
    std::string cleaned_content(content_view.data(), content_view.size());
    std::u16string u16_cleaned_content(reinterpret_cast<const char16_t*>(cleaned_content.data()), cleaned_content.size() / sizeof(char16_t));

    auto convert = g_conv->ToUtf8FromUtf16LE(u16_cleaned_content);
    if (convert.empty()) {
        LOGERROR("Conversion failed for file: " + inputFile);
        std::cout << "Conversion failed for file: " + inputFile << std::endl;
        return;
    }
    WriteFileBytes(outputFile, convert);
    std::cout << "Converted content written to: " + outputFile << std::endl;
}

/// <summary>
/// Test API ToUtf8FromUtf16BE
/// </summary>
void TestToUtf8FromUtf16BE() {
    std::string inputFile = "testdata/input_utf16be_nobom.txt";
    std::string outputFile = "testdata/output/output_utf-16be_to_utf-8.txt";
    std::string content = ReadFileBytes(inputFile);
    std::u16string utf16be_content(reinterpret_cast<const char16_t*>(content.data()), content.size() / sizeof(char16_t));
    auto convert = g_conv->ToUtf8FromUtf16BE(utf16be_content);
    if (convert.empty()) {
        LOGERROR("Conversion failed for file: " + inputFile);
        std::cout << "Conversion failed for file: " + inputFile << std::endl;
        return;
    }
    WriteFileBytes(outputFile, convert);
    std::cout << "Converted content written to: " + outputFile << std::endl;
}

/// <summary>
/// Test API ToUtf8FromUtf16BE with BOM
/// </summary>
void TestToUtf8FromUtf16BEWithBOM() {
    std::string inputFile = "testdata/input_utf16be.txt";
    std::string outputFile = "testdata/output/output_utf-16be_to_utf-8_with_bom.txt";
    std::string content = ReadFileBytes(inputFile);
    auto [encoding, content_view] = RemoveBOM(content);
    if (encoding != BomEncoding::UTF16_BE) {
        LOGERROR("Expected UTF-16BE encoding with BOM, but got: " + std::to_string(static_cast<int>(encoding)) + " FILE: " + inputFile);
        std::cout << "Expected UTF-16BE encoding with BOM, but got: " + std::to_string(static_cast<int>(encoding)) << std::endl;
        return;
    } else if (encoding == BomEncoding::UTF16_BE) {
        LOGOK("Encoding detected as UTF-16BE with BOM. FILE: " + inputFile);
        std::cout << "Encoding detected as UTF-16BE with BOM. FILE: " << inputFile << std::endl;
    }
    std::string cleaned_content(content_view.data(), content_view.size());
    std::u16string u16_cleaned_content(reinterpret_cast<const char16_t*>(cleaned_content.data()), cleaned_content.size() / sizeof(char16_t));
    auto convert = g_conv->ToUtf8FromUtf16BE(u16_cleaned_content);
    if (convert.empty()) {
        LOGERROR("Conversion failed for file: " + inputFile);
        std::cout << "Conversion failed for file: " + inputFile << std::endl;
        return;
    }
    WriteFileBytes(outputFile, convert);
    std::cout << "Converted content written to: " + outputFile << std::endl;
}

/// <summary>
/// Test API ToUtf16LEFromUtf8
/// </summary>
void TestToUtf16LEFromUtf8() {
    std::string inputFile = "testdata/input_utf8.txt";
    std::string outputFile = "testdata/output/output_utf-8_to_utf-16le.txt";
    std::string content = ReadFileBytes(inputFile);
    std::u16string convert = g_conv->ToUtf16LEFromUtf8(content);
    if (convert.empty()) {
        LOGERROR("Conversion failed for file: " + inputFile);
        std::cout << "Conversion failed for file: " + inputFile << std::endl;
        return;
    }
    WriteFileBytes(outputFile, std::string(reinterpret_cast<const char*>(convert.data()), convert.size() * sizeof(char16_t)));
    std::cout << "Converted content written to: " + outputFile << std::endl;
}

/// <summary>
/// Test API ToUtf16BEFromUtf8
/// </summary>
void TestToUtf16BEFromUtf8() {
    std::string inputFile = "testdata/input_utf8.txt";
    std::string outputFile = "testdata/output/output_utf-8_to_utf-16be.txt";
    std::string content = ReadFileBytes(inputFile);
    std::u16string convert = g_conv->ToUtf16BEFromUtf8(content);
    if (convert.empty()) {
        LOGERROR("Conversion failed for file: " + inputFile);
        std::cout << "Conversion failed for file: " + inputFile << std::endl;
        return;
    }
    WriteFileBytes(outputFile, std::string(reinterpret_cast<const char*>(convert.data()), convert.size() * sizeof(char16_t)));
    std::cout << "Converted content written to: " + outputFile << std::endl;
}

/// <summary>
/// Test API ToUtf16LEFromUtf16BE
/// </summary>
void TestToUtf16BEFromUtf16LE() {
    std::string inputFile = "testdata/input_utf16le_nobom.txt";
    std::string outputFile = "testdata/output/output_utf-16le_to_utf-16be.txt";
    std::string content = ReadFileBytes(inputFile);
    std::u16string utf16le_content(reinterpret_cast<const char16_t*>(content.data()), content.size() / sizeof(char16_t));
    std::u16string convert = g_conv->ToUtf16BEFromUtf16LE(utf16le_content);
    if (convert.empty()) {
        LOGERROR("Conversion failed for file: " + inputFile);
        std::cout << "Conversion failed for file: " + inputFile << std::endl;
        return;
    }
    WriteFileBytes(outputFile, std::string(reinterpret_cast<const char*>(convert.data()), convert.size() * sizeof(char16_t)));
    std::cout << "Converted content written to: " + outputFile << std::endl;
}

/// <summary>
/// Test API ToUtf16LEFromUtf16BE
/// </summary>
void TestToUtf16LEFromUtf16BE() {
    std::string inputFile = "testdata/input_utf16be_nobom.txt";
    std::string outputFile = "testdata/output/output_utf-16be_to_utf-16le.txt";
    std::string content = ReadFileBytes(inputFile);
    std::u16string utf16be_content(reinterpret_cast<const char16_t*>(content.data()), content.size() / sizeof(char16_t));
    std::u16string convert = g_conv->ToUtf16LEFromUtf16BE(utf16be_content);
    if (convert.empty()) {
        LOGERROR("Conversion failed for file: " + inputFile);
        std::cout << "Conversion failed for file: " + inputFile << std::endl;
        return;
    }
    WriteFileBytes(outputFile, std::string(reinterpret_cast<const char*>(convert.data()), convert.size() * sizeof(char16_t)));
    std::cout << "Converted content written to: " + outputFile << std::endl;
}

/// <summary>
/// Test API LocaleToWideString
/// </summary>
void TestLocaleToWideString() {
    std::string inputFile = "testdata/input_gb2312.txt";

    std::string content = ReadFileBytes(inputFile);
    std::cout << "Content size: " << content.size() << " bytes" << std::endl;
    std::cout << "Content : " << content << std::endl;
    std::wstring wideResult = g_conv->LocaleToWideString(content);
    assert(!wideResult.empty());

    std::string outputFile = "testdata/output/output_gbk_to_wide.txt";
    WriteFileBytes(outputFile, std::string(reinterpret_cast<const char*>(wideResult.data()), wideResult.size() * sizeof(wchar_t)));
    std::cout << "[Test_LocaleToWideString] wideResult size: " << wideResult.size() << " characters" << std::endl;
    std::wcout.imbue(std::locale(""));
    std::wcout << L"[Test_LocaleToWideString] wideResult: " << wideResult << std::endl;
    LOGINFO("Converted content written to: " + outputFile);
}

// ========== Test Functions from Test.cpp ==========

// Generate test files
void GenerateTestFiles() {
    // Create output directory
    CreateDirectories("testdata");
    
    // Test text (Chinese)
    std::string test_text = "Test Chinese Hello World！！123";
    
    LOGINFO("Starting to generate test files...");
    
    // 1. Generate UTF-8 file (no BOM)
    WriteFileBytes("testdata/input_utf8.txt", test_text);
    LOGINFO("Generated UTF-8 test file: testdata/input_utf8.txt");
    
    // 2. Generate GBK/GB2312 file
    auto conv = UniConv::GetInstance();
    auto gbk_result = conv->ConvertEncoding(test_text, "UTF-8", "GBK");
    if (gbk_result.IsSuccess()) {
        WriteFileBytes("testdata/input_gbk.txt", gbk_result.conv_result_str);
        LOGOK("Generated GBK test file successfully: testdata/input_gbk.txt");
    } else {
        LOGERROR("Failed to generate GBK test file: " + gbk_result.error_msg);
    }
    
    // 3. Generate UTF-16LE file (with BOM)
    auto utf16le_result = conv->ConvertEncoding(test_text, "UTF-8", "UTF-16LE");
    if (utf16le_result.IsSuccess()) {
        std::string utf16le_with_bom = "\xFF\xFE" + utf16le_result.conv_result_str;
        WriteFileBytes("testdata/input_utf16le.txt", utf16le_with_bom);
        LOGOK("Generated UTF-16LE test file successfully: testdata/input_utf16le.txt");
    } else {
        LOGERROR("Failed to generate UTF-16LE test file: " + utf16le_result.error_msg);
    }
    
    // 4. Generate UTF-16BE file (with BOM)
    auto utf16be_result = conv->ConvertEncoding(test_text, "UTF-8", "UTF-16BE");
    if (utf16be_result.IsSuccess()) {
        std::string utf16be_with_bom = "\xFE\xFF" + utf16be_result.conv_result_str;
        WriteFileBytes("testdata/input_utf16be.txt", utf16be_with_bom);
        LOGOK("Generated UTF-16BE test file successfully: testdata/input_utf16be.txt");
    } else {
        LOGERROR("Failed to generate UTF-16BE test file: " + utf16be_result.error_msg);
    }
    
    // 5. Generate local encoding file (GB2312)
    auto local_result = conv->ConvertEncoding(test_text, "UTF-8", "GB2312");
    if (local_result.IsSuccess()) {
        WriteFileBytes("testdata/input_local.txt", local_result.conv_result_str);
        LOGOK("Generated GB2312 test file successfully: testdata/input_local.txt");
    } else {
        LOGERROR("Failed to generate GB2312 test file: " + local_result.error_msg);
    }
    
    LOGINFO("Test file generation completed");
}

// Batch convert files test implementation
void BatchConvertFiles() {
    system("chcp 65001"); // Set console encoding to UTF-8
    system("cls"); // Clear screen

    LOGINFO("=== Starting batch file conversion test ===");
    
    auto conv = UniConv::GetInstance();
    
    // Conversion configuration: source file -> target encoding
    struct ConversionTask {
        std::string inputFile;
        std::string outputFile;
        std::string fromEncoding;
        std::string toEncoding;
        std::string description;
    };
    
    std::vector<ConversionTask> conversion_tasks = {
        {"testdata/input_utf8.txt", "testdata/output/output_utf16le.txt", "UTF-8", "UTF-16LE", "UTF-8 -> UTF-16LE"},
        {"testdata/input_utf8.txt", "testdata/output/output_utf16be.txt", "UTF-8", "UTF-16BE", "UTF-8 -> UTF-16BE"},
        {"testdata/input_utf8.txt", "testdata/output/output_gbk.txt", "UTF-8", "GBK", "UTF-8 -> GBK"},
        {"testdata/input_gbk.txt", "testdata/output/output_utf8_from_gbk.txt", "GBK", "UTF-8", "GBK -> UTF-8"},
        {"testdata/input_utf16le.txt", "testdata/output/output_utf8_from_utf16le.txt", "UTF-16LE", "UTF-8", "UTF-16LE -> UTF-8"},
        {"testdata/input_utf16be.txt", "testdata/output/output_utf8_from_utf16be.txt", "UTF-16BE", "UTF-8", "UTF-16BE -> UTF-8"}
    };
    
    for (const auto& task : conversion_tasks) {
        LOGINFO("--- " + task.description + " ---");
        
        // Read input file
        std::string input_data = ReadFileBytes(task.inputFile);
        if (input_data.empty()) {
            LOGERROR("Error: Cannot read file " + task.inputFile);
            continue;
        }
        
        // Detect and remove BOM
        auto result_pair = DetectEncodingAndRemoveBOM(input_data);
        std::string detected_encoding = result_pair.first;
        std::string clean_data = result_pair.second;
        std::string actual_from_encoding = detected_encoding.empty() ? task.fromEncoding : detected_encoding;
        
        LOGINFO("Input file: " + task.inputFile);
        LOGINFO("Original data size: " + std::to_string(input_data.size()) + " bytes");
        LOGINFO("Detected encoding: " + (detected_encoding.empty() ? "No BOM" : detected_encoding));
        LOGINFO("Cleaned data size: " + std::to_string(clean_data.size()) + " bytes");
        LOGDEBUG("Input data hex: " + BytesToHex(clean_data));
        
        // Execute encoding conversion
        auto result = conv->ConvertEncoding(clean_data, actual_from_encoding.c_str(), task.toEncoding.c_str());
        
        if (result.IsSuccess()) {
            LOGOK("Conversion successful");
            LOGINFO("Output data size: " + std::to_string(result.conv_result_str.size()) + " bytes");
            LOGDEBUG("Output data hex: " + BytesToHex(result.conv_result_str));
            
            // Write to output file
            if (WriteFileBytes(task.outputFile, result.conv_result_str)) {
                LOGOK("Successfully wrote output file: " + task.outputFile);
            } else {
                LOGERROR("Error: Cannot write output file: " + task.outputFile);
            }
        } else {
            LOGERROR("Conversion failed: " + result.error_msg);
        }
        
        LOGINFO("");
    }
    
    LOGINFO("=== Batch file conversion test completed ===");
}

// Test all encoding conversion functions
void TestAllConversions() {
    LOGINFO("=== Starting test of all encoding conversion functions ===");
    
    auto conv = UniConv::GetInstance();
    std::string test_text = "Test text Hello World 123";
    
    LOGINFO("Original test text: " + test_text);
    LOGINFO("System encoding: " + conv->GetCurrentSystemEncoding());
    
    // Test UTF-8 <-> Local encoding
    {
        LOGINFO("--- Testing UTF-8 <-> Local encoding ---");
        auto local_result = conv->ToLocaleFromUtf8(test_text);
        LOGDEBUG("UTF-8 -> Local: " + BytesToHex(local_result));
        
        auto utf8_result = conv->ToUtf8FromLocale(local_result);
        LOGINFO("Local -> UTF-8: " + utf8_result);
        
        if (utf8_result == test_text) {
            LOGOK("Round-trip conversion successful");
        } else {
            LOGERROR("Round-trip conversion failed");
        }
    }
    
    // Test UTF-8 <-> UTF-16LE
    {
        LOGINFO("--- Testing UTF-8 <-> UTF-16LE ---");
        auto utf16le_result = conv->ToUtf16LEFromUtf8(test_text);
        LOGDEBUG("UTF-8 -> UTF-16LE: " + BytesToHex(std::string(reinterpret_cast<const char*>(utf16le_result.data()), utf16le_result.size() * 2)));
        
        auto utf8_result = conv->ToUtf8FromUtf16LE(utf16le_result);
        LOGINFO("UTF-16LE -> UTF-8: " + utf8_result);
        
        if (utf8_result == test_text) {
            LOGOK("Round-trip conversion successful");
        } else {
            LOGERROR("Round-trip conversion failed");
        }
    }
    
    // Test UTF-8 <-> UTF-16BE
    {
        LOGINFO("--- Testing UTF-8 <-> UTF-16BE ---");
        auto utf16be_result = conv->ToUtf16BEFromUtf8(test_text);
        LOGDEBUG("UTF-8 -> UTF-16BE: " + BytesToHex(std::string(reinterpret_cast<const char*>(utf16be_result.data()), utf16be_result.size() * 2)));
        
        auto utf8_result = conv->ToUtf8FromUtf16BE(utf16be_result);
        LOGINFO("UTF-16BE -> UTF-8: " + utf8_result);
        
        if (utf8_result == test_text) {
            LOGOK("Round-trip conversion successful");
        } else {
            LOGERROR("Round-trip conversion failed");
        }
    }
    
    LOGINFO("=== All encoding conversion tests completed ===");
}

// Run all test functions
void RunAllTests() {
    LOGINFO("=== Starting to run all tests ===");
    
    // Generate test files
    GenerateTestFiles();
    
    // Test all conversion functions
    TestAllConversions();
    
    // Batch convert files
    BatchConvertFiles();
    
    LOGINFO("=== All tests completed ===");
}

// ========== Main Function ==========

int main() {
    SetConsoleOutputCP(CP_UTF8);
    InitializeLogging();

    // Run individual API tests from main.cpp
    LOGINFO("=== Running Individual API Tests ===");
    
    TestToUtf8FromLocale();            // OK
    TestToLocaleFromUtf8();            // OK
    TestGetCurrentSystemEncoding();    // OK
    TestGetEncodingNameByCodePage();   // OK
    TestToUtf16LEFromLocale();         // OK
    TestToUtf16BEFromLocale();         // OK
    TestToLocaleFromUtf16BE();         // OK
    TestToLocaleFromUtf16LE();         // OK
    TestToLocaleFromUtf8WithBOM();     // OK
    TestToLocaleFromUtf16BE_WithBOM(); // OK
    TestToLocaleFromUtf16LEWithBOM();  // OK
    TestToUtf8FromUtf16LE();
    TestToUtf8FromUtf16LEWithBOM();
    TestToUtf8FromUtf16BE();          // OK
    TestToUtf8FromUtf16BEWithBOM();   // OK
    TestToUtf16LEFromUtf8();
    TestToUtf16BEFromUtf8();
    TestToUtf16BEFromUtf16LE();  // OK
    TestToUtf16LEFromUtf16BE();  // OK
    TestLocaleToWideString();
    TestTostring();

    // Run comprehensive tests from Test.cpp
    LOGINFO("=== Running Comprehensive Test Suite ===");
    RunAllTests();

    LOGINFO("=== All tests completed successfully ===");
    system("pause");
    return 0;
}