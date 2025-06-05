#if _MSC_VER >= 1600 
#pragma execution_character_set("utf-8")
#endif

#include "UniConv.h"
#include "LightLogWriteImpl.hpp"
#include <iostream>
#include <windows.h>
#include <assert.h>



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



struct ConversionTask {
    std::string inputFile;
    std::string outputFile;
    std::string fromEncoding;
    std::string toEncoding;
    std::string description;
};



std::string UTF8    = UniConv::ToString(UniConv::Encoding::utf_8);
std::string GBK     = UniConv::ToString(UniConv::Encoding::gbk);
std::string UTF16LE = UniConv::ToString(UniConv::Encoding::utf_16le);
std::string UTF16BE = UniConv::ToString(UniConv::Encoding::utf_16be);


ConversionTask task_gb2312_to_utf8 = {
    "testdata/input_gb2312.txt",
    "testdata/output/output_gb2312_to_utf-8.txt",
	"GBK",
	"UTF-8",
	"GBK -> UTF-8"
};



enum class BomEncoding {
    None,
    UTF8,
    UTF16_LE,
    UTF16_BE,
    UTF32_LE,
    UTF32_BE
};

std::vector<ConversionTask> tasks = {
    task_gb2312_to_utf8
};

// Convert the byte data to a hexadecimal string
std::string BytesToHex(const std::string& data) {
    std::ostringstream oss;
    for (unsigned char c : data) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c) << " ";
    }
    return oss.str();
}


std::string ReadFileBytes(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        return "";
    }
    return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

bool WriteFileBytes(const std::string& filePath, const std::string& data) {
    std::ofstream file(filePath, std::ios::binary);
    if (!file) {
        return false;
    }
    file.write(data.data(), data.size());
    return file.good();
}

// check file encoding and remove BOM if exists
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

//
//void BatchConvertFiles() {
//    std::cout << "=== Start the batch file conversion test === " << std::endl;
//    LOGINFO("=== Start the batch file conversion test === ");
//
//    for (const auto& task : tasks) {
//        LOGINFO("--- " + task.description + " ---");
//        std::cout << "--- " + task.description + " ---" << std::endl;
//        std::string input_data = ReadFileBytes(task.inputFile);
//        if ( input_data.empty() ) {
//            LOGERROR("Error: cannot read file\t" + task.inputFile);
//			std::cout << "Error: cannot read file\t" + task.inputFile << std::endl;
//        }
//
//        // Check and remove BOM 
//		auto result_pair = DetectEncodingAndRemoveBOM(input_data);
//		std::string detectedEncoding = result_pair.first;
//		std::string cleanedData = result_pair.second;
//		std::string actualFromEncoding = detectedEncoding.empty() ? task.fromEncoding : detectedEncoding;
//
//        // Log info
//		LOGINFO("Input file: " + task.inputFile);
//        LOGINFO("Input file size: " + std::to_string(input_data.size()) + " bytes");
//		LOGINFO("Detected encoding: " + (detectedEncoding.empty()) ? "NO BOM" : detectedEncoding);
//		LOGINFO("Clean data size: " + std::to_string(cleanedData.size()) + " bytes");
//		LOGINFO("Input data hex: " + BytesToHex(cleanedData));
//
//        std::cout << "Input file: " + task.inputFile << std::endl;
//		std::cout << "Input file size: " + std::to_string(input_data.size()) + " bytes" << std::endl;
//		std::cout << "Detected encoding: " + (detectedEncoding.empty() ? "NO BOM" : detectedEncoding) << std::endl;
//		std::cout << "Clean data size: " + std::to_string(cleanedData.size()) + " bytes" << std::endl;
//        std::cout << "Input data hex: " + BytesToHex(cleanedData) << std::endl;
//
//		auto result = g_conv->ConvertEncoding(cleanedData, actualFromEncoding.c_str(), task.toEncoding.c_str());
//
//        if (result.IsSuccess()) {
//            LOGOK("Conversion successful: " + task.fromEncoding + " -> " + task.toEncoding);
//			std::cout << "Conversion successful: " + task.fromEncoding + " -> " + task.toEncoding << std::endl;
//
//            LOGINFO("Output file: " + task.outputFile);
//            std::cout << "Output file: " + task.outputFile << std::endl;
//
//            // Write the converted data to the output file
//            if (WriteFileBytes(task.outputFile, result.conv_result_str)) {
//                LOGOK("Output file written successfully: " + task.outputFile);
//                std::cout << "Output file written successfully: " + task.outputFile << std::endl;
//            } else {
//                LOGERROR("Error writing output file: " + task.outputFile);
//                std::cout << "Error writing output file: " + task.outputFile << std::endl;
//            }
//        }
//        else {
//
//        }
//    }
//}


void TestTostring()
{
    std::string utf8 = "UTF-8";
	std::string gbk = "GBK";
    std::string uni_utf8 = UniConv::ToString(UniConv::Encoding::utf_8);
	std::string uni_gbk = UniConv::ToString(UniConv::Encoding::gbk);

    if( uni_utf8 == utf8 && uni_gbk == gbk ) {
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
    }
    else if (encoding == BomEncoding::UTF8) {
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
    auto [encoding, content_view]  = RemoveBOM(content);
    if ( encoding !=  BomEncoding::UTF16_BE ) {
        LOGERROR("Expected UTF-16BE encoding with BOM, but got: " + std::to_string(static_cast<int>(encoding)) + "FILE : " + inputFile);
        std::cout << "Expected UTF-16BE encoding with BOM, but got: " + std::to_string(static_cast<int>(encoding)) << std::endl;
        return;
    }
    else if ( encoding == BomEncoding::UTF16_BE ) {
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

    auto convert = g_conv->ToLocalFromUtf16LE(utf16le_content);
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
    }
    else if (encoding == BomEncoding::UTF16_LE) {
        LOGOK("Encoding detected as UTF-16LE with BOM. FILE: " + inputFile);
        std::cout << "Encoding detected as UTF-16LE with BOM. FILE: " << inputFile << std::endl;
	}

    std::string cleaned_content(content_view.data(), content_view.size());
    std::u16string u16_cleaned_content(reinterpret_cast<const char16_t*>(cleaned_content.data()), cleaned_content.size() / sizeof(char16_t));

	auto convert = g_conv->ToLocalFromUtf16LE(u16_cleaned_content);
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
    }
    else if (encoding == BomEncoding::UTF16_LE) {
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
    }
    else if (encoding == BomEncoding::UTF16_BE) {
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



int  main() {

    SetConsoleOutputCP(CP_UTF8);
    InitializeLogging();

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
   
    

    return 0;
}